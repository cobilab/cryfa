/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Encryption / Decryption
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include "EnDecrypto.h"
#include "pack.h"
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"
#include "cryptopp/eax.h"
using std::vector;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::thread;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::StreamTransformationFilter;

std::mutex mutx;

htable_t HDR_MAP, QS_MAP;          // hash tables for header and quality score
//string   HEADERS, QUALITY_SCORES;   // max: 39 values
string totHdrRange, totQsRange;     // if hash tables are built in pack function
byte nEmptyIn;
//bool finished;

/*******************************************************************************
    constructor
*******************************************************************************/
EnDecrypto::EnDecrypto () {}

/*******************************************************************************
    compress FASTA.
    * reserved symbols:
          (char) 255:  penalty if sequence length isn't multiple of 3
          (char) 254:  end of each sequence line
          (char) 253:  instead of '>' in header
          (char) 252:  instead of empty line
*******************************************************************************/
void EnDecrypto::compressFA ()
{
    ifstream in(inFileName);
    string line, seq, context;  // FASTA: context = header + seq (+ empty lines)
    
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    // watermark for encrypted file
    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                       + std::to_string(RELEASE_CRYFA) + "\n";

    // to tell decryptor this isn't FASTQ
    context += (char) 127;      // context += "\n";
    while (getline(in, line).good())
    {
        // header
        if (line[0] == '>')
        {
            if (!seq.empty())   context += packSeq_3to1(seq);   // previous seq
            seq.clear();
            
            // header line. (char) 253 instead of '>'
            context += (char) 253 + line.substr(1) + "\n";
        }
            
            // empty line. (char) 252 instead of line feed
        else if (line.empty())    seq += (char) 252;
            
            // sequence
        else
        {
            if (line.find(' ') != string::npos)
            { cerr << "Invalid sequence -- spaces not allowed.\n";    exit(1); }
            // (char) 254 instead of '\n' at the end of each seq line
            seq += line + (char) 254;
        }
    }
    if (!seq.empty())   context += packSeq_3to1(seq);           // the last seq
    
    in.close();
    
    // encryption
    cout << encrypt(context);
    cout << '\n';
}

/*******************************************************************************
    compress FASTQ.
    * reserved symbols:
          (char) 255:  penalty if sequence length isn't multiple of 3
          (char) 254:  end of each line
          (char) 253:  if third line contains only +
          (char) 252:  end of file
    * categories for headers and quality scores:
                cat 1  =  2
                cat 2  =  3
           4 <= cat 3 <=  6
           7 <= cat 4 <= 15
          16 <= cat 5 <= 39
*******************************************************************************/
void EnDecrypto::compressFQ ()
{
    ifstream in(inFileName);   // main input file
    string line;                    // each file line
    string inTh;    // input string which will be sent to threads
    thread arrThread[n_threads];
    byte t;
    
//    htable_t HDR_MAP, QS_MAP;          // hash tables for header and quality score
//    string   HEADERS, QUALITY_SCORES;   // max: 39 values
//    string hdrRange, qsRange;
////    string   line, seq, context;   // FASTQ: context = header + seq + plus + qs

    // check if the third line contains only +
    bool justPlus = true;
    in.ignore(LARGE_NUMBER, '\n');                  // ignore header
    in.ignore(LARGE_NUMBER, '\n');                  // ignore seq
    if(getline(in, line).good() && line.length() > 1)   justPlus = false;
    in.clear();  in.seekg(0, in.beg);       // beginning of file
//    in.close();

    
    /*
    // check if the third line contains only +
    bool justPlus = true;
    string::const_iterator lFFirst = std::find(in.begin(), in.end(), '\n');
    string::const_iterator lFSecond = std::find(lFFirst+1, in.end(), '\n');
    if (*(lFSecond+2) != '\n')  justPlus = false;   // check symbol after +
    */


    // gather all headers and quality scores
    while (!in.eof())
    {
        if (getline(in, line).good())                       // header
        {
            for (const char &c : line)
                if (totHdrRange.find_first_of(c) == string::npos)
                    totHdrRange += c;
        }
        in.ignore(LARGE_NUMBER, '\n');                      // ignore sequence
        in.ignore(LARGE_NUMBER, '\n');                      // ignore +
        if (getline(in, line).good())                       // quality score
        {
            for (const char &c : line)
                if (totQsRange.find_first_of(c) == string::npos)
                    totQsRange += c;
        }
    }
    in.clear();     in.seekg(0, std::ios::beg);             // beginning of file
    in.close();
    
    totHdrRange.erase(totHdrRange.begin());                       //ignore '@'
    std::sort(totHdrRange.begin(), totHdrRange.end());            // sort values
    std::sort(totQsRange.begin(),  totQsRange.end());             // sort ASCII values

//    using packHdrPointer = string (*)(string, string, htable_t);
    using packHdrPointer = string (*)(const string&, const htable_t&);
//    using packHdrPointer = string (*)(const string&, const string&, const htable_t&);
    packHdrPointer packHdr;                                 // function pointer
//    using packQSPointer  = string (*)(string, string, htable_t);
    using packQSPointer  = string (*)(const string&, const htable_t&);
//    using packQSPointer  = string (*)(const string&, const string&, const htable_t&);
    packQSPointer packQS;                                   // function pointer

    string HEADERS_X;                                 // extended HEADERS
    string QUALITY_SCORES_X;                          // extended QUALITY_SCORES
    const size_t hdrRangeLen = totHdrRange.length();
    const size_t qsRangeLen  = totQsRange.length();

    // header
    if (hdrRangeLen > MAX_CAT_5)          // if len > 39 filter the last 39 ones
    {
        HEADERS   = totHdrRange.substr(hdrRangeLen - MAX_CAT_5);
        HEADERS_X = HEADERS;
        // ASCII char after last char in HEADERS
        HEADERS_X += (char) (HEADERS[HEADERS.size()-1] + 1);
    
//        HDR_MAP = buildHashTable(HDR_MAP, HEADERS_X, KEYLEN_CAT_5);
        HDR_MAP = buildHashTable(HEADERS_X, KEYLEN_CAT_5);
        packHdr = &packLargeHdr_3to2;
//        packHdr = &packLarge_3to2;
    }
    else
    {
        HEADERS = totHdrRange;

        if (hdrRangeLen > MAX_CAT_4)            // cat 5
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_5);
            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_5);
            packHdr = &pack_3to2;
        }
        else if (hdrRangeLen > MAX_CAT_3)       // cat 4
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_4);
            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_4);
            packHdr = &pack_2to1;
        }
        else if (hdrRangeLen == MAX_CAT_3 || hdrRangeLen == MID_CAT_3
                 || hdrRangeLen == MIN_CAT_3)  // cat 3
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_3);
            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_3);
            packHdr = &pack_3to1;
        }
        else if (hdrRangeLen == CAT_2)          // cat 2
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_2);
            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_2);
            packHdr = &pack_5to1;
        }
        else if (hdrRangeLen == CAT_1)          // cat 1
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_1);
            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_1);
            packHdr = &pack_7to1;
        }
        else    // hdrRangeLen = 1
        {
//            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, 1);
            HDR_MAP = buildHashTable(HEADERS, 1);
            packHdr = &pack_1to1;
        }
    }
    
    // quality score
    if (qsRangeLen > MAX_CAT_5)           // if len > 39 filter the last 39 ones
    {
        QUALITY_SCORES   = totQsRange.substr(qsRangeLen - MAX_CAT_5);
        QUALITY_SCORES_X = QUALITY_SCORES;
        // ASCII char after last char in QUALITY_SCORES
        QUALITY_SCORES_X +=(char) (QUALITY_SCORES[QUALITY_SCORES.size()-1] + 1);
    
//        QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES_X, KEYLEN_CAT_5);
        QS_MAP = buildHashTable(QUALITY_SCORES_X, KEYLEN_CAT_5);
        packQS = &packLargeQs_3to2;
//        packQS = &packLarge_3to2;
    }
    else
    {
        QUALITY_SCORES = totQsRange;

        if (qsRangeLen > MAX_CAT_4)             // cat 5
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_5);
            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_5);
            packQS = &pack_3to2;
        }
        else if (qsRangeLen > MAX_CAT_3)        // cat 4
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_4);
            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_4);
            packQS = &pack_2to1;
        }
        else if (qsRangeLen == MAX_CAT_3 || qsRangeLen == MID_CAT_3
                 || qsRangeLen == MIN_CAT_3)   // cat 3
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_3);
            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_3);
            packQS = &pack_3to1;
        }
        else if (qsRangeLen == CAT_2)           // cat 2
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_2);
            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_2);
            packQS = &pack_5to1;
        }
        else if (qsRangeLen == CAT_1)           // cat 1
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_1);
            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_1);
            packQS = &pack_7to1;
        }
        else    // qsRangeLen = 1
        {
//            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, 1);
            QS_MAP = buildHashTable(QUALITY_SCORES, 1);
            packQS = &pack_1to1;
        }
    }
    
    
//    for (htable_t::iterator i = HDR_MAP.begin(); i != HDR_MAP.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';




////    bool single = true;     // single-thread
//    bool single = false;
//
//    if(single)
//    {
//        // watermark for encrypted file
//        cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
//                + std::to_string(RELEASE_CRYFA) + "\n";
//
//        string cipherText;
//        string context;
//        context += hdrRange;                       // send hdrRange to decryptor
//        context += (char) 254;                     // to detect hdrRange in dec.
//        context += qsRange;                        // send qsRange to decryptor
//        context += (justPlus ? (char) 253 : '\n'); //'+ or not just +' condition
//        while (!in.eof())    // process 4 lines by 4 lines
//        {
//            if (getline(in, line).good())          // header
//                context += packHdr(line.substr(1), HEADERS, HDR_MAP)
//                           + (char) 254;    // ignore '@'
//
//            if (getline(in, line).good())          // sequence
//                context += packSeq_3to1(line) + (char) 254;
//
//            in.ignore(LARGE_NUMBER, '\n');         // +. ignore
//
//            if (getline(in, line).good())          // quality score
//                context += packQS(line, QUALITY_SCORES, QS_MAP) + (char) 254;
//        }
//        context += (char) 252;  // end of file
//        cout << context;
//
////    // encryption
////    cout << encrypt(context);
//////    // dump cyphertext for read
//////    for (const char &c : cipherText)
//////        cout << (char) (c & 0xFF);
////    cout << '\n';
//    }
//    else
    {

//    byte nEmptyIn = 0;
    ULL startLine;
    ULL i=0;
//    finished = false;
//    while (!in.eof())
    while (!nEmptyIn)
//    while (i<1)
//    while (!finished)
    {
        nEmptyIn = 0;

//        finished = true;

        // save LINE_BUFFER lines to a string & pass to "pack"
        for (t = 0; t != n_threads; ++t)
        {
            startLine = (i*n_threads + t) * LINE_BUFFER;

//            inTh.clear();
//            for (UI i = 0; i != LINE_BUFFER && !in.eof()
//                           && getline(in, line).good(); ++i)
//                inTh += line + "\n";
//
//            if (inTh.empty())   ++nEmptyIn; // number of empty input strings
//            else
            arrThread[t] = thread(&EnDecrypto::pack, this,
                                  startLine, packHdr, packQS, t);
        }   // end for t


        for (t = 0; t != n_threads; ++t)
                arrThread[t].join();

        ++i;
    }   // end while




    // join encrypted files
    ifstream encFile[n_threads];
//    std::vector<pos_t> chunkEndPos;
    string context;

    // watermark for encrypted file
    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                       + std::to_string(RELEASE_CRYFA) + "\n";

//    context += hdrRange;                       // send hdrRange to decryptor
//    context += (char) 254;                     // to detect hdrRange in dec.
//    context += qsRange;                        // send qsRange to decryptor
    context += totHdrRange;                       // send hdrRange to decryptor
    context += (char) 254;                     // to detect hdrRange in dec.
    context += totQsRange;                        // send qsRange to decryptor
    context += (justPlus ? (char) 253 : '\n'); //'+ or not just +' condition
//    out << context ;//<< '\n';    // too aes cbc mode nemishe

    // open input files
    for (t = 0; t != n_threads; ++t)
        encFile[t].open(ENC_FILENAME + std::to_string(t));

    bool prevLineNotThrID;    // if previous line was "THR=" or not
    while (!encFile[0].eof())
    {
        for (t = 0; t != n_threads; ++t)
        {
            prevLineNotThrID = false;

            while (getline(encFile[t], line).good() &&
                    line.compare(THR_ID_HDR+std::to_string(t)))
            {
                if (prevLineNotThrID)   context += '\n';
                context += line;
                prevLineNotThrID = true;
            }

//            chunkEndPos.push_back(out.tellp());     // chunks end position
        }
    }
    context += (char) 252;

    // close input and output files
    for (t = 0; t != n_threads; ++t)   encFile[t].close();

//    cout << context;



//    // remove the first zeros corresponding to the first line of all files
//    chunkEndPos.erase(chunkEndPos.begin(), chunkEndPos.begin() + n_threads);
//
    cout << encrypt(context);
    cout << '\n';


    } // end not single thread

    
    
    
    

//    // join encrypted files
//    ofstream out(ENC_FILENAME);
//    ifstream encFile[n_threads];
//    std::vector<pos_t> chunkEndPos;
//
//    // watermark for encrypted file
//    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
//                       + std::to_string(RELEASE_CRYFA) + "\n";
//
//    string context;
//    context += hdrRange;                       // send hdrRange to decryptor
//    context += (char) 254;                     // to detect hdrRange in dec.
//    context += qsRange;                        // send qsRange to decryptor
//    context += (justPlus ? (char) 253 : '\n'); //'+ or not just +' condition
//
//    cipherText = encrypt(context);
//    for (const char &c : cipherText)    cout << (char) (c & 0xFF);
//    cout << '\n';
//
//    // open input files
//    for (t = 0; t != n_threads; ++t)
//        encFile[t].open(ENC_FILENAME + std::to_string(t));
//
//    while (!encFile[0].eof())
//        for (t = 0; t != n_threads; ++t)
//        {
//            while (getline(encFile[t], line).good() &&
//                   line.compare(THR_ID_HDR + std::to_string(t)))
////                out << line << '\n';        // line isn't THR=...
//            cout << line << '\n';        // line isn't THR=...
//
//            chunkEndPos.push_back(out.tellp());     // chunks end position
//        }
//
//    cipherText = encrypt(std::to_string((char) 252));
//    for (const char &c : cipherText)    cout << (char) (c & 0xFF);
//    cout << '\n';
//
//    // close input and output files
//    for (t = 0; t != n_threads; ++t)   encFile[t].close();
//    out.close();
//
//    // remove the first zeros corresponding to the first line of all files
//    chunkEndPos.erase(chunkEndPos.begin(), chunkEndPos.begin() + n_threads);

//    in.close();

/*
//    // get size of file
//    infile.seekg(0, infile.end);
//    long size = 1000000;//infile.tellg();
//    infile.seekg(0);
//
//    // allocate memory for file content
//    char *buffer = new char[size];
//
//    // read content of infile
//    infile.read(buffer, size);
//
//    // write to outfile
//    outfile.write(buffer, size);
//
//    // release dynamically-allocated memory
//    delete[] buffer;

//    outfile.close();
//    infile.close();
*/

}

/*******************************************************************************
    pack -- '@' at the beginning of headers is not packed
*******************************************************************************/
inline void EnDecrypto::pack (const ULL startLine,
                              string (*packHdr)(const string&, const htable_t&),
                              string (*packQS)(const string&, const htable_t&),
                              const byte threadID)
{
//    string hdrRange, qsRange;
//    htable_t HDR_MAP, QS_MAP;           // hash tables for header and quality score
//    string HEADERS, QUALITY_SCORES;   // max: 39 values
    string context; // output string
    string inTempStr;
//    string::const_iterator i = in.begin();
    
    mutx.lock();
    ifstream in(inFileName);
    mutx.unlock();
    
    string line;
    
    for (ULL l = 0; l != startLine; ++l)    in.ignore(LARGE_NUMBER, '\n');
    
    // beginning of the part of file for this thread
//    std::ios_base::seekdir pos_beg = std::ios_base::cur;
    pos_t pos_beg = in.tellg();
    
//    mutx.lock();
//    cerr << (int) threadID << ';' << startLine << ' '
////         << pos_beg
//         << (char) in.peek() << '\n';
//    mutx.unlock();
    
    //todo. mohem
    if (in.peek()==EOF) { mutx.lock();  ++nEmptyIn;  mutx.unlock();    return; }
    
//    if (!getline(in, line).good())
//    {
//        mutx.lock();
//        ++nEmptyIn;
////        cerr << "ID=" << (int) threadID << ' ' << '\n';
//        mutx.unlock();
//        return;
//    }
//
//    in.clear();    in.seekg(pos_beg);  // return to beginning of line
////    in.seekg(-line.size() - 1, std::ios_base::cur);// return to beginning of line



    
//    // gather all headers and quality scores
//    for (ULL l = 0; l != LINE_BUFFER && !in.eof(); l+=4)
//    {
//        if (getline(in, line).good())                       // header
//            for (const char &c : line)
//                if (hdrRange.find_first_of(c) == string::npos)
//                    hdrRange += c;
//
//        in.ignore(LARGE_NUMBER, '\n');                      // ignore sequence
//        in.ignore(LARGE_NUMBER, '\n');                      // ignore +
//
//        if (getline(in, line).good())                       // quality score
//            for (const char &c : line)
//                if (qsRange.find_first_of(c) == string::npos)
//                    qsRange += c;
//    }
//    in.clear();     in.seekg(pos_beg);             // beginning of this part of file
////    in.clear();     in.seekg(0, std::ios::beg);             // beginning of file
//    hdrRange.erase(hdrRange.begin());                       //ignore '@'
//
////    mutx.lock();
////    cerr << (int) threadID << '=' << hdrRange << '\n';
//////    cerr << (int) threadID << '=' << qsRange << '\n';
////    mutx.unlock();
    
    
    /*
    // gather all headers and quality scores
    while (i != in.end())
    {
        for (i += 1; *i != '\n'; ++i)   // header -- ignore '@'
            if (hdrRange.find_first_of(*i) == string::npos)
                hdrRange += *i;

        for (i += 1; *i != '\n'; ++i);  // ignore sequence
        for (i += 1; *i != '\n'; ++i);  // ignore +

        for (i += 1; *i != '\n'; ++i)   // quality score
            if (qsRange.find_first_of(*i) == string::npos)
                qsRange += *i;

        i += 1;
    }
     */
    
////    mutx.lock();
//    for (const char &c : hdrRange)
//        if (totHdrRange.find_first_of(c) == string::npos)
//            totHdrRange += c;
//
//    for (const char &c : qsRange)
//        if (totQsRange.find_first_of(c) == string::npos)
//            totQsRange += c;
////    mutx.unlock();
    
    
    // '@' must be included in 'totHdrRange'
//    totHdrRange.erase(std::remove(totHdrRange.begin(), totHdrRange.end(), '@'),
//                      totHdrRange.end());
//    cerr << totHdrRange << '\n';

////    hdrRange.erase(hdrRange.begin());                       //ignore '@'
//    std::sort(hdrRange.begin(), hdrRange.end());            // sort values
//    std::sort(qsRange.begin(),  qsRange.end());             // sort ASCII values
//
//    std::sort(totHdrRange.begin(), totHdrRange.end());  // sort values
//    std::sort(totQsRange.begin(),  totQsRange.end());   // sort ASCII values
//
//
////    using packHdrPointer = string (*)(string, string, htable_t);
//    using packHdrPointer = string (*)(string, string, htable_t&);
//    packHdrPointer packHdr;                                 // function pointer
////    using packQSPointer  = string (*)(string, string, htable_t);
//    using packQSPointer  = string (*)(string, string, htable_t&);
//    packQSPointer packQS;                                   // function pointer
//
//    string HEADERS_X;                                 // extended HEADERS
//    string QUALITY_SCORES_X;                          // extended QUALITY_SCORES
//    const size_t hdrRangeLen = hdrRange.length();
//    const size_t qsRangeLen  = qsRange.length();
//
//    // header
//    if (hdrRangeLen > MAX_CAT_5)          // if len > 39 filter the last 39 ones
//    {
//        HEADERS   = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//        HEADERS_X = HEADERS;
//        // ASCII char after last char in HEADERS
//        HEADERS_X += (char) (HEADERS[HEADERS.size()-1] + 1);
//
////        HDR_MAP = buildHashTable(HDR_MAP, HEADERS_X, KEYLEN_CAT_5);
//        HDR_MAP = buildHashTable(HEADERS_X, KEYLEN_CAT_5);
//        packHdr = &packLarge_3to2;
//    }
//    else
//    {
//        HEADERS = hdrRange;
//
//        if (hdrRangeLen > MAX_CAT_4)            // cat 5
//        {
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_5);
//            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_5);
//            packHdr = &pack_3to2;
//        }
//        else if (hdrRangeLen > MAX_CAT_3)       // cat 4
//        {
//            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_4);
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_4);
//            packHdr = &pack_2to1;
//        }
//        else if (hdrRangeLen == MAX_CAT_3 || hdrRangeLen == MID_CAT_3
//                 || hdrRangeLen == MIN_CAT_3)  // cat 3
//        {
//            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_3);
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_3);
//            packHdr = &pack_3to1;
//        }
//        else if (hdrRangeLen == CAT_2)          // cat 2
//        {
//            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_2);
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_2);
//            packHdr = &pack_5to1;
//        }
//        else if (hdrRangeLen == CAT_1)          // cat 1
//        {
//            HDR_MAP = buildHashTable(HEADERS, KEYLEN_CAT_1);
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_1);
//            packHdr = &pack_7to1;
//        }
//        else    // hdrRangeLen = 1
//        {
//            HDR_MAP = buildHashTable(HEADERS, 1);
////            HDR_MAP = buildHashTable(HDR_MAP, HEADERS, 1);
//            packHdr = &pack_1to1;
//        }
//    }
//
//    // quality score
//    if (qsRangeLen > MAX_CAT_5)           // if len > 39 filter the last 39 ones
//    {
//        QUALITY_SCORES   = qsRange.substr(qsRangeLen - MAX_CAT_5);
//        QUALITY_SCORES_X = QUALITY_SCORES;
//        // ASCII char after last char in QUALITY_SCORES
//        QUALITY_SCORES_X +=(char) (QUALITY_SCORES[QUALITY_SCORES.size()-1] + 1);
//
//        QS_MAP = buildHashTable(QUALITY_SCORES_X, KEYLEN_CAT_5);
////        QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES_X, KEYLEN_CAT_5);
//        packQS = &packLarge_3to2;
//    }
//    else
//    {
//        QUALITY_SCORES = qsRange;
//
//        if (qsRangeLen > MAX_CAT_4)             // cat 5
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_5);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_5);
//            packQS = &pack_3to2;
//        }
//        else if (qsRangeLen > MAX_CAT_3)        // cat 4
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_4);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_4);
//            packQS = &pack_2to1;
//        }
//        else if (qsRangeLen == MAX_CAT_3 || qsRangeLen == MID_CAT_3
//                 || qsRangeLen == MIN_CAT_3)   // cat 3
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_3);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_3);
//            packQS = &pack_3to1;
//        }
//        else if (qsRangeLen == CAT_2)           // cat 2
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_2);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_2);
//            packQS = &pack_5to1;
//        }
//        else if (qsRangeLen == CAT_1)           // cat 1
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, KEYLEN_CAT_1);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_1);
//            packQS = &pack_7to1;
//        }
//        else    // qsRangeLen = 1
//        {
//            QS_MAP = buildHashTable(QUALITY_SCORES, 1);
////            QS_MAP = buildHashTable(QS_MAP, QUALITY_SCORES, 1);
//            packQS = &pack_1to1;
//        }
//    }


    
    for (ULL l = 0; l != LINE_BUFFER; l+=4)     // process 4 lines by 4 lines
    {
        if (getline(in, line).good())           // header -- ignore '@'
            context += packHdr(line.substr(1), HDR_MAP) + (char) 254;
//        context += packHdr(line.substr(1), HEADERS, HDR_MAP) + (char) 254;

        if (getline(in, line).good())           // sequence
            context += packSeq_3to1(line) + (char) 254;

        in.ignore(LARGE_NUMBER, '\n');          // +. ignore

        if (getline(in, line).good())           // quality score
            context += packQS(line, QS_MAP) + (char) 254;
//        context += packQS(line, QUALITY_SCORES, QS_MAP) + (char) 254;
    }
    
//    i = in.begin();
//    while (i != in.end())
//    {
//        // header -- ignore '@'
//        inTempStr.clear();
//        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
//        context += packHdr(inTempStr, HEADERS, HDR_MAP) + (char) 254;
//
//        // sequence
//        inTempStr.clear();
//        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
//        context += packSeq_3to1(inTempStr) + (char) 254;
//
//        // +. ignore
//        for (i += 1; *i != '\n'; ++i);
//
//        // quality score
//        inTempStr.clear();
//        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
//        context += packQS(inTempStr, QUALITY_SCORES, QS_MAP) + (char) 254;
//
//        i += 1;
//    }

    ofstream encfile;
    encfile.open(ENC_FILENAME+std::to_string(threadID), std::ios_base::app);

    // write header containing threadID for each
    encfile << THR_ID_HDR + std::to_string(threadID) << '\n';
    encfile << context << '\n';

    encfile.close();

    in.close();
}

/*******************************************************************************
    encrypt.
    AES encryption uses a secret key of a variable length (128, 196 or 256 bit).
    This key is secretly exchanged between two parties before communication
    begins. DEFAULT_KEYLENGTH = 16 bytes.
*******************************************************************************/
inline string EnDecrypto::encrypt (const string &context)
{
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv, 0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = getPassFromFile();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug

//     // do random shuffle
//     srand(0);
//     std::random_shuffle(context.begin(),context.end());
//     * need to know the reverse of shuffle, for decryption!
    
    string cipherText;
    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
    StreamTransformationFilter stfEncryptor(cbcEncryption,
                                          new CryptoPP::StringSink(cipherText));
    stfEncryptor.Put(reinterpret_cast<const byte*>
                     (context.c_str()), context.length() + 1);
    stfEncryptor.MessageEnd();
    
//    if (verbose)
//    {
//        cerr << "   sym size: " << context.size()    << '\n';
//        cerr << "cipher size: " << cipherText.size() << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE    << '\n';
//    }
    
    string encryptedText;
//    for (char c : cipherText)
    for (const char &c : cipherText)
        encryptedText += (char) (0xFF & static_cast<byte> (c));
//        encryptedText += (char) (c & 0xFF);
//    encryptedText+='\n';
    return encryptedText;
}

/*******************************************************************************
    decompress
*******************************************************************************/
void EnDecrypto::decompress ()
{
    string decText = decrypt();   //decryption
    
    (decText[0] == (char) 127) ? decompFA(decText)
                               : decompFQ(decText); //decompression
}

/*******************************************************************************
    decrypt.
    AES encryption uses a secret key of a variable length (128, 196 or 256 bit).
    This key is secretly exchanged between two parties before communication
    begins. DEFAULT_KEYLENGTH = 16 bytes.
*******************************************************************************/
inline string EnDecrypto::decrypt ()
{
    string decText;
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = getPassFromFile();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    string cipherText( (std::istreambuf_iterator<char> (in)),
                       std::istreambuf_iterator<char> () );
    
    // watermark
    string watermark = "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                                  + std::to_string(RELEASE_CRYFA) + "\n";
    
    string::size_type watermarkIdx = cipherText.find(watermark);
    if (watermarkIdx == string::npos)
    { cerr << "Error: invalid encrypted file!\n";    exit(1); }
    else  cipherText.erase(watermarkIdx, watermark.length());
    
    if (verbose)
    {
        cerr << "cipher size: " << cipherText.size()-1 << '\n';
        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
    }
    
    AES::Decryption aesDecryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    StreamTransformationFilter stfDecryptor(cbcDecryption,
                                            new CryptoPP::StringSink(decText));
    stfDecryptor.Put(reinterpret_cast<const byte*>
                     (cipherText.c_str()), cipherText.size()-1);
    stfDecryptor.MessageEnd();
    
    return decText;
}

/*******************************************************************************
    decompress FASTA.
    * reserved symbols:
          (char) 255:  penalty if sequence length isn't multiple of 3
          (char) 254:  end of each sequence line
          (char) 253:  instead of '>' in header
          (char) 252:  instead of empty line
*******************************************************************************/
inline void EnDecrypto::decompFA (string decText)
{
    string line;
    string tpl;     // tuplet
    string::iterator i = decText.begin();
    
    bool isHeader = true;
    byte s;
    
    ++i;    // jump over decText[0]
    for (; i != decText.end()-1; ++i)   // exclude last symbol of decText
    {
        s = (byte) *i;
        //empty line OR end of each seq line
        if (s == 252 || (s == 254 && !isHeader)) { cout << '\n'; }
            //seq len not multiple of 3
        else if (s == 255) { cout << penaltySym(*(++i)); }
            // header
        else if (s == 253) { cout << '>';  isHeader = true; }
        else if (isHeader) { cout << s; if (s == '\n') isHeader = false; }
            // sequence
        else //if (!isHeader)
        {
            tpl = DNA_UNPACK[s];
            
            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')            // ...
            { cout<<tpl; }
                // using just one 'cout' makes trouble
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')       // X..
            { cout<<penaltySym(*(++i));    cout<<tpl[1];    cout<<tpl[2]; }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')       // .X.
            { cout<<tpl[0];    cout<<penaltySym(*(++i));    cout<<tpl[2]; }
            
            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')       // XX.
            { cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i));
                cout<<tpl[2]; }
            
            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')       // ..X
            { cout<<tpl[0];    cout<<tpl[1];    cout<<penaltySym(*(++i)); }
            
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')       // X.X
            { cout<<penaltySym(*(++i));    cout<<tpl[1];
                cout<<penaltySym(*(++i)); }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')       // .XX
            { cout<<tpl[0];    cout<<penaltySym(*(++i));
                cout<<penaltySym(*(++i)); }
            
            else { cout<<penaltySym(*(++i));                          // XXX
                cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i)); }
        }
    }
}

/*******************************************************************************
    decompress FASTQ.
    * reserved symbols:
          (char) 255:  penalty if sequence length isn't multiple of 3
          (char) 254:  end of each line
          (char) 253:  if third line contains only +
          (char) 252:  end of file
    * categories for headers and quality scores:
                cat 1  = 2
                cat 2  = 3
           4 <= cat 3 <=  6
           7 <= cat 4 <= 15
          16 <= cat 5 <= 39
*******************************************************************************/
inline void EnDecrypto::decompFQ (string decText)
{
//    string*  HDR_UNPACK;    // for unpacking header
//    string*  QS_UNPACK;     // for unpacking quality score
    vector<string> HDR_UNPACK; // for unpacking header
    vector<string> QS_UNPACK;  // for unpacking quality score
    string line;
    string::iterator i = decText.begin();
    
    string qsRange;
    string hdrRange;
    bool justPlus = true;
    
    for (; *i != (char) 254; ++i)    hdrRange += *i;                  // all hdr
    ++i;    // jump over (char) 254
    for (; *i != '\n' && *i != (char) 253; ++i)    qsRange += *i;     // all qs
    if (*i == '\n')  justPlus = false;                  // if 3rd line is just +
    ++i;   // jump over '\n' or (char) 253

    const size_t qsRangeLen  = qsRange.length();
    const size_t hdrRangeLen = hdrRange.length();
    US keyLen_hdr = 0;
    US keyLen_qs = 0;
    
    //todo. ehtemalan niaz nis chon ghablan sort shode
//    std::sort(hdrRange.begin(),hdrRange.end());


//    mutx.lock();
//    cerr<<hdrRange<<'\t'<<hdrRangeLen<<'\n'<<qsRange<<'\t'<<qsRangeLen<<'\n';
//    mutx.unlock();
    
    
    
    
    using unpackHdrPointer = string (*)(string::iterator&, vector<string>&);
//    using unpackHdrPointer = string (*)(string::iterator&, string*);
//    using unpackHdrPointer = string (*)(string::iterator&, string* &);
    unpackHdrPointer unpackHdr;                              // function pointer
    using unpackQSPointer = string (*)(string::iterator&, vector<string>&);
//    using unpackQSPointer = string (*)(string::iterator&, string*);
//    using unpackQSPointer = string (*)(string::iterator&, string* &);
    unpackQSPointer unpackQS;                                // function pointer

    // header
    if (hdrRangeLen > MAX_CAT_5)    keyLen_hdr = KEYLEN_CAT_5;

    else if (hdrRangeLen > MAX_CAT_4)                               // cat 5
    { keyLen_hdr = KEYLEN_CAT_5;    unpackHdr = &unpack_read2B; }

    else if (hdrRangeLen > MAX_CAT_3)                               // cat 4
    { keyLen_hdr = KEYLEN_CAT_4;    unpackHdr = &unpack_read1B; }

    else if (hdrRangeLen == MAX_CAT_3 || hdrRangeLen == MID_CAT_3   // cat 3
             || hdrRangeLen == MIN_CAT_3)
    { keyLen_hdr = KEYLEN_CAT_3;    unpackHdr = &unpack_read1B; }

    else if (hdrRangeLen == CAT_2)                                  // cat 2
    { keyLen_hdr = KEYLEN_CAT_2;    unpackHdr = &unpack_read1B; }

    else if (hdrRangeLen == CAT_1)                                  // cat 1
    { keyLen_hdr = KEYLEN_CAT_1;    unpackHdr = &unpack_read1B; }

    else { keyLen_hdr = 1;          unpackHdr = &unpack_read1B; }   // = 1
    
    // quality score
    if (qsRangeLen > MAX_CAT_5)     keyLen_qs = KEYLEN_CAT_5;
    
    else if (qsRangeLen > MAX_CAT_4)                                // cat 5
    { keyLen_qs = KEYLEN_CAT_5;     unpackQS = &unpack_read2B; }

    else if (qsRangeLen > MAX_CAT_3)                                // cat 4
    { keyLen_qs = KEYLEN_CAT_4;     unpackQS = &unpack_read1B; }

    else if (qsRangeLen == MAX_CAT_3 || qsRangeLen == MID_CAT_3     // cat 3
             || qsRangeLen == MIN_CAT_3)
    { keyLen_qs = KEYLEN_CAT_3;     unpackQS = &unpack_read1B; }

    else if (qsRangeLen == CAT_2)                                   // cat 2
    { keyLen_qs = KEYLEN_CAT_2;     unpackQS = &unpack_read1B; }

    else if (qsRangeLen == CAT_1)                                   // cat 1
    { keyLen_qs = KEYLEN_CAT_1;     unpackQS = &unpack_read1B; }

    else { keyLen_qs = 1;           unpackQS = &unpack_read1B; }    // = 1

    string plusMore;
//    if (hdrRangeLen > MAX_CAT_5 && qsRangeLen > MAX_CAT_5)
//    {
//        const string headers = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//        const string quality_scores = qsRange.substr(qsRangeLen-MAX_CAT_5);
//        // ASCII char after the last char in headers & quality_scores string
//        const char XChar_hdr = (char) (headers[headers.size()-1] + 1);
//        const char XChar_qs=(char)(quality_scores[quality_scores.size()-1] + 1);
//        string headers_X = headers;                  headers_X+=XChar_hdr;
//        string quality_scores_X = quality_scores;    quality_scores_X+=XChar_qs;
//
//        // tables for unpacking
//        HDR_UNPACK = buildUnpack(headers_X,        keyLen_hdr, HDR_UNPACK);
//        QS_UNPACK  = buildUnpack(quality_scores_X, keyLen_qs,  QS_UNPACK);
//
//        while (i != decText.end())
//        {
//            cout << '@';
//            cout << (plusMore = unpackLarge_read2B(i, XChar_hdr, HDR_UNPACK))
//                 << '\n';                                         ++i;    // hdr
//            cout << unpackSeqFQ_3to1(i) << '\n';                          // seq
//            cout << (justPlus ? "+" : "+" + plusMore) << '\n';    ++i;    // +
//            cout << unpackLarge_read2B(i, XChar_qs, QS_UNPACK) << '\n';   // qs
//            // end of file
//            if (*(++i) == (char) 252)   break;
//        }
//    }
//    else if (hdrRangeLen > MAX_CAT_5 && qsRangeLen <= MAX_CAT_5)
//    {
//        const string headers = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//        // ASCII char after the last char in headers string
//        const char XChar_hdr = (char) (headers[headers.size()-1] + 1);
//        string headers_X = headers;     headers_X += XChar_hdr;
//
//        // tables for unpacking
//        HDR_UNPACK = buildUnpack(headers_X, keyLen_hdr, HDR_UNPACK);
//        QS_UNPACK  = buildUnpack(qsRange,   keyLen_qs,  QS_UNPACK);
//
//        while (i != decText.end())
//        {
//            cout << '@';
//            cout << (plusMore = unpackLarge_read2B(i, XChar_hdr, HDR_UNPACK))
//                 << '\n';                                         ++i;    // hdr
//            cout << unpackSeqFQ_3to1(i)               << '\n';            // seq
//            cout << (justPlus ? "+" : "+" + plusMore) << '\n';    ++i;    // +
//            cout << unpackQS(i, QS_UNPACK)            << '\n';            // qs
//            // end of file
//            if (*(++i) == (char) 252)   break;
//        }
//    }
//    else if (hdrRangeLen <= MAX_CAT_5 && qsRangeLen > MAX_CAT_5)
//    {
//        const string quality_scores = qsRange.substr(qsRangeLen - MAX_CAT_5);
//        // ASCII char after the last char in quality_scores string
//        const char XChar_qs=(char)(quality_scores[quality_scores.size()-1] + 1);
//        string quality_scores_X=quality_scores;  quality_scores_X+=XChar_qs;
//
//        // tables for unpacking
//        HDR_UNPACK = buildUnpack(hdrRange,         keyLen_hdr, HDR_UNPACK);
//        QS_UNPACK  = buildUnpack(quality_scores_X, keyLen_qs,  QS_UNPACK);
//
//        while (i != decText.end())
//        {
//            cout << '@';
//            cout << (plusMore = unpackHdr(i, HDR_UNPACK)) << '\n';  ++i;  // hdr
//            cout << unpackSeqFQ_3to1(i)                   << '\n';        // seq
//            cout << (justPlus ? "+" : "+" + plusMore)     << '\n';  ++i;  // +
//            cout << unpackLarge_read2B(i, XChar_qs, QS_UNPACK) << '\n';   // qs
//            // end of file
//            if (*(++i) == (char) 252)   break;
//        }
//    }
//    else
    if (hdrRangeLen <= MAX_CAT_5 && qsRangeLen <= MAX_CAT_5)
    {
        // tables for unpacking
        HDR_UNPACK = buildUnpack(hdrRange, keyLen_hdr);
        QS_UNPACK  = buildUnpack(qsRange,  keyLen_qs);
//        HDR_UNPACK = buildUnpack(hdrRange, keyLen_hdr, HDR_UNPACK);
//        QS_UNPACK  = buildUnpack(qsRange,  keyLen_qs,  QS_UNPACK);
        
//        for(string s:HDR_UNPACK)
//            cerr<<s<<' ';



        while (i != decText.end())
        {

//            cerr << '@';
//            cerr << (plusMore = unpackHdr(i, HDR_UNPACK)) << '\n';  ++i;  // hdr
//            cerr << unpackSeqFQ_3to1(i)                   << '\n';        // seq
//            cerr << (justPlus ? "+" : "+" + plusMore)     << '\n';  ++i;  // +
//            cerr << unpackQS(i, QS_UNPACK)                << '\n';        // qs

            
            cout << '@';
            cout << (plusMore = unpackHdr(i, HDR_UNPACK)) << '\n';  ++i;  // hdr
            cout << unpackSeqFQ_3to1(i)                   << '\n';        // seq
            cout << (justPlus ? "+" : "+" + plusMore)     << '\n';  ++i;  // +
            cout << unpackQS(i, QS_UNPACK)                << '\n';        // qs
            // end of file
            if (*(++i) == (char) 252)   break;
        }
    }
}

/*******************************************************************************
    build IV
*******************************************************************************/
inline void EnDecrypto::buildIV (byte *iv, const string &pass)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evalPassSize(pass);  // pass size must be >= 8
    
    // using old rand to generate the new rand seed
    srand((UI) 7919 * pass[2] * pass[5] + 75653);
    ULL seed = 0;
//    for (byte i = 0; i != pass.size(); ++i)
    for (byte i = (byte) pass.size(); i--;)
        seed += ((ULL) pass[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

//    for (UI i = 0; i != AES::BLOCKSIZE; ++i)
    for (UI i = (UI) AES::BLOCKSIZE; i--;)
        iv[i] = (byte) (udist(rng) % 255);
}

/*******************************************************************************
    build key
*******************************************************************************/
inline void EnDecrypto::buildKey (byte *key, const string &pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evalPassSize(pwd);  // pass size must be >= 8
    
    // using old rand to generate the new rand seed
    srand((UI) 24593 * (pwd[0] * pwd[2]) + 49157);
    ULL seed = 0;
//    for (byte i = 0; i != pwd.size(); ++i)
    for (byte i = (byte) pwd.size(); i--;)
        seed += ((ULL) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

//    for (UI i = 0; i != AES::DEFAULT_KEYLENGTH; ++i)
    for (UI i = (UI) AES::DEFAULT_KEYLENGTH; i--;)
        key[i] = (byte) (udist(rng) % 255);
}

/*******************************************************************************
    print IV
*******************************************************************************/
inline void EnDecrypto::printIV (byte *iv) const
{
    cerr << "IV = [" << (int) iv[0];
    for (UI i = 1; i != AES::BLOCKSIZE; ++i)
        cerr << " " << (int) iv[i];
    cerr << "]\n";
}

/*******************************************************************************
    print key
*******************************************************************************/
inline void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (UI i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}

/*******************************************************************************
    get password from a file
*******************************************************************************/
inline string EnDecrypto::getPassFromFile () const
{
    ifstream input(keyFileName);
    string line;
    
    if (keyFileName.empty())
    {
        cerr << "Error: no password file has been set!\n";
        exit(1);
    }
    else if (!input.good())
    {
        cerr << "Error opening '" << keyFileName << "'.\n";
        exit(1);
    }
    
    while (getline(input, line).good())
    {
        if (line.empty()) {cerr<<"Error: empty password line file!\n"; exit(1);}
        return line;
    }
    
    return "unknown";
}

/*******************************************************************************
    evaluate password size >= 8
*******************************************************************************/
inline void EnDecrypto::evalPassSize (const string &pass) const
{
    if (pass.size() < 8)
    {
        cerr << "Error: password size must be at least 8!\n";
        exit(1);
    }
}










// single-threaded version
///*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//    Encryption / Decryption
//    - - - - - - - - - - - - - - - - - - -
//    Diogo Pratas        pratas@ua.pt
//    Morteza Hosseini    seyedmorteza@ua.pt
//    Armando J. Pinho    ap@ua.pt
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//
//#include <fstream>
//#include <functional>
//#include "EnDecrypto.h"
//#include "pack.h"
//#include "cryptopp/modes.h"
//#include "cryptopp/aes.h"
//#include "cryptopp/filters.h"
//#include "cryptopp/eax.h"
//using std::cout;
//using std::cerr;
//using std::ifstream;
//using std::getline;
//using CryptoPP::AES;
//using CryptoPP::CBC_Mode_ExternalCipher;
//using CryptoPP::StreamTransformationFilter;
//
///*******************************************************************************
//    constructor
//*******************************************************************************/
//EnDecrypto::EnDecrypto () {}
//
///*******************************************************************************
//    encrypt.
//    * reserved symbols:
//          FASTA:
//              (char) 255:  penalty if sequence length isn't multiple of 3
//              (char) 254:  end of each sequence line
//              (char) 253:  instead of '>' in header
//              (char) 252:  instead of empty line
//          FASTQ:
//              (char) 255:  penalty if sequence length isn't multiple of 3
//              (char) 254:  end of each line
//              (char) 253:  if third line contains only +
//              (char) 252:  end of file
//    * categories for headers and quality scores:
//                cat 1  = 2
//                cat 2  = 3
//           4 <= cat 3 <=  6
//           7 <= cat 4 <= 15
//          16 <= cat 5 <= 39
//*******************************************************************************/
//void EnDecrypto::encrypt (int argc, char **argv)
//{
//    ifstream in(argv[argc-1]);
//    const bool FASTA = (findFileType(in) == 'A');
//    const bool FASTQ = !FASTA;  // const bool FASTQ = (findFileType(in) == 'Q');
//    string line;                // each line of file
//    string seq;                 // sequence (FASTA/FASTQ)
//    // FASTA: context = header + seq (+ empty lines)
//    // FASTQ: context = header + seq + plus + qs
//    string context;
//    string hdrRange;            // header symbols presented in FASTQ file
//    string qsRange;             // quality scores presented in FASTQ file
//
//    if (!in.good())
//    { cerr << "Error: failed opening '" << argv[argc-1] << "'.\n";    exit(1); }
//
//    // FASTA
//    if (FASTA)
//    {
//        // to tell decryptor this isn't FASTQ
//        context += (char) 127;//      context += "\n";
//        while (getline(in, line).good())
//        {
//            // header
//            if (line[0] == '>')
//            {
//                if (!seq.empty())   // previous seq
//                    context += packSeq_3to1(seq);
//                seq.clear();
//
//                // header line. (char) 253 instead of '>'
//                context += (char) 253 + line.substr(1) + "\n";
//            }
//
//                // empty line. (char) 252 instead of line feed
//            else if (line.empty())    seq += (char) 252;
//
//                // sequence
//            else
//            {
//                if (line.find(' ') != string::npos)
//                { cerr << "Invalid sequence -- spaces not allowed.\n"; exit(1);}
//                // (char) 254 instead of '\n' at the end of each seq line
//                seq += line + (char) 254;
//            }
//        }
//        if (!seq.empty())   context += packSeq_3to1(seq);  // the last seq
//    }
//
//        // FASTQ
//    else //if (FASTQ)
//    {
//        // check if the third line contains only +
//        bool justPlus = true;
//        in.ignore(LARGE_NUMBER, '\n');                  // ignore header
//        in.ignore(LARGE_NUMBER, '\n');                  // ignore seq
//        if (getline(in, line).good()) { if (line.length() > 1) justPlus=false; }
////        else { cerr << "Error: file corrupted.\n";    return; }
//        in.clear();  in.seekg(0, in.beg);        // beginning of file
//
//        // gather all headers and quality scores
//        while(!in.eof())
//        {
//            if (getline(in, line).good())               // header
//            {
//                for (string::iterator i = line.begin(); i != line.end(); ++i)
//                    if (hdrRange.find_first_of(*i) == string::npos)
//                        hdrRange += *i;
//            }
////            else { cerr << "Error: file corrupted.\n";    return; }
//            in.ignore(LARGE_NUMBER, '\n');              // ignore seq
//            in.ignore(LARGE_NUMBER, '\n');              // ignore +
//            if (getline(in, line).good())               // quality score
//            {
//                for (string::iterator i = line.begin(); i != line.end(); ++i)
//                    if (qsRange.find_first_of(*i) == string::npos)
//                        qsRange += *i;
//            }
////            else { cerr << "Error: file corrupted.\n";    return; }
//        }
//        in.clear();  in.seekg(0, std::ios::beg);        // beginning of file
//
//        hdrRange.erase(hdrRange.begin());               // remove '@'
//
//        std::sort(hdrRange.begin(), hdrRange.end());    // sort values
//        std::sort(qsRange.begin(),  qsRange.end());     // sort ASCII values
//
//        using packHdrPointer = string (*)(string, string, htable_t&);
//        packHdrPointer packHdr;                         // function pointer
//        using packQSPointer  = string (*)(string, string, htable_t&);
//        packQSPointer packQS;                           // function pointer
//
//        string HEADERS_X;                             // extended HEADERS
//        string QUALITY_SCORES_X;                      // extended QUALITY_SCORES
//        const size_t qsRangeLen  = qsRange.length();
//        const size_t hdrRangeLen = hdrRange.length();
//
//        // header
//        if (hdrRangeLen > MAX_CAT_5)      // if len > 39 filter the last 39 ones
//        {
//            HEADERS   = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//            HEADERS_X = HEADERS;
//            // ASCII char after last char in HEADERS
//            HEADERS_X += (char) (HEADERS[HEADERS.size()-1] + 1);
//
//            buildHashTable(HDR_MAP, HEADERS_X, KEYLEN_CAT_5);
//            packHdr = &packLarge_3to2;
//        }
//        else
//        {
//            HEADERS = hdrRange;
//
//            if (hdrRangeLen > MAX_CAT_4)            // cat 5
//            {
//                buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_5);
//                packHdr = &pack_3to2;
//            }
//            else if (hdrRangeLen > MAX_CAT_3)       // cat 4
//            {
//                buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_4);
//                packHdr = &pack_2to1;
//            }
//            else if (hdrRangeLen == MAX_CAT_3 || hdrRangeLen == MID_CAT_3
//                     || hdrRangeLen == MIN_CAT_3)  // cat 3
//            {
//                buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_3);
//                packHdr = &pack_3to1;
//            }
//            else if (hdrRangeLen == CAT_2)          // cat 2
//            {
//                buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_2);
//                packHdr = &pack_5to1;
//            }
//            else if (hdrRangeLen == CAT_1)          // cat 1
//            {
//                buildHashTable(HDR_MAP, HEADERS, KEYLEN_CAT_1);
//                packHdr = &pack_7to1;
//            }
//            else    // hdrRangeLen = 1
//            {
//                buildHashTable(HDR_MAP, HEADERS, 1);
//                packHdr = &pack_1to1;
//            }
//        }
//
//        // quality score
//        if (qsRangeLen > MAX_CAT_5)       // if len > 39 filter the last 39 ones
//        {
//            QUALITY_SCORES   = qsRange.substr(qsRangeLen - MAX_CAT_5);
//            QUALITY_SCORES_X = QUALITY_SCORES;
//            QUALITY_SCORES_X +=  // ASCII char after last char in QUALITY_SCORES
//                    (char) (QUALITY_SCORES[QUALITY_SCORES.size()-1] + 1);
//
//            buildHashTable(QS_MAP, QUALITY_SCORES_X, KEYLEN_CAT_5);
//            packQS = &packLarge_3to2;
//        }
//        else
//        {
//            QUALITY_SCORES = qsRange;
//
//            if (qsRangeLen > MAX_CAT_4)             // cat 5
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_5);
//                packQS = &pack_3to2;
//            }
//            else if (qsRangeLen > MAX_CAT_3)        // cat 4
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_4);
//                packQS = &pack_2to1;
//            }
//            else if (qsRangeLen == MAX_CAT_3 || qsRangeLen == MID_CAT_3
//                     || qsRangeLen == MIN_CAT_3)   // cat 3
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_3);
//                packQS = &pack_3to1;
//            }
//            else if (qsRangeLen == CAT_2)           // cat 2
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_2);
//                packQS = &pack_5to1;
//            }
//            else if (qsRangeLen == CAT_1)           // cat 1
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, KEYLEN_CAT_1);
//                packQS = &pack_7to1;
//            }
//            else    // qsRangeLen = 1
//            {
//                buildHashTable(QS_MAP, QUALITY_SCORES, 1);
//                packQS = &pack_1to1;
//            }
//        }
//
//
//
//        //todo. nabas havijoori 'context+=' nevesht,
//        //todo. chon va3 file 10GB mitereke
//        //todo. bas hame kara ro block by block anjam dad
//
//        context += hdrRange;                       // send hdrRange to decryptor
//        context += (char) 254;                     // to detect hdrRange in dec.
//        context += qsRange;                        // send qsRange to decryptor
//        context += (justPlus ? (char) 253 : '\n'); //'+ or not just +' condition
//        while(!in.eof())    // process 4 lines by 4 lines
//        {
//            if (getline(in, line).good())          // header
//                context += packHdr(line.substr(1), HEADERS, HDR_MAP)
//                           + (char) 254;    // ignore '@'
//
//            if (getline(in, line).good())          // sequence
//                context += packSeq_3to1(line) + (char) 254;
//
//            in.ignore(LARGE_NUMBER, '\n');         // +. ignore
//
//            if (getline(in, line).good())          // quality score
//                context += packQS(line, QUALITY_SCORES, QS_MAP) + (char) 254;
//        }
//        context += (char) 252;  // end of file
//    }
//
//    in.close();
//
//    // cryptography
//    // AES encryption uses a secret key of a variable length (128, 196 or
//    // 256 bit). This key is secretly exchanged between two parties before
//    // communication begins. DEFAULT_KEYLENGTH= 16 bytes
//    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
//    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
//    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
//
//    const string pass = getPassFromFile();
//    buildKey(key, pass);
//    buildIV(iv, pass);
////    printIV(iv);      // debug
////    printKey(key);    // debug
//
////     // do random shuffle
////     srand(0);
////     std::random_shuffle(context.begin(),context.end());
////     * need to know the reverse of shuffle, for decryption!
//
//    string cipherText;
//    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
//    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
//    StreamTransformationFilter stfEncryptor(cbcEncryption,
//                                            new CryptoPP::StringSink(cipherText));
//    stfEncryptor.Put(reinterpret_cast<const byte*>
//                     (context.c_str()), context.length() + 1);
//    stfEncryptor.MessageEnd();
//
//    if (verbose)
//    {
//        cerr << "   sym size: " << context.size()    << '\n';
//        cerr << "cipher size: " << cipherText.size() << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE    << '\n';
//    }
//
//    // watermark for encrypted file
//    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
//            + std::to_string(RELEASE_CRYFA) + "\n";
//
//    // dump cyphertext for read
//    for (string::iterator i = cipherText.begin(); i != cipherText.end(); ++i)
//        cout << (char) (0xFF & static_cast<byte> (*i));
//    cout << '\n';
//}
//
///*******************************************************************************
//    decrypt.
//    * reserved symbols:
//          FASTA:
//              (char) 255:  penalty if sequence length isn't multiple of 3
//              (char) 254:  end of each sequence line
//              (char) 253:  instead of '>' in header
//              (char) 252:  instead of empty line
//          FASTQ:
//              (char) 255:  penalty if sequence length isn't multiple of 3
//              (char) 254:  end of each line
//              (char) 253:  if third line contains only +
//              (char) 252:  end of file
//    * categories for headers and quality scores:
//                cat 1  = 2
//                cat 2  = 3
//           4 <= cat 3 <=  6
//           7 <= cat 4 <= 15
//          16 <= cat 5 <= 39
//*******************************************************************************/
//void EnDecrypto::decrypt (int argc, char **argv)
//{
//    // cryptography
//    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
//    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
//    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
//
//    const string pass = getPassFromFile();
//    buildKey(key, pass);
//    buildIV(iv, pass);
////    printIV(iv);      // debug
////    printKey(key);    // debug
//
//    string line, decText;
//    ifstream in(argv[argc-1]);
//
//    if (!in.good())
//    {
//        cerr << "Error: failed opening '" << argv[argc-1] << "'.\n";
//        return;
//    }
//
//    string cipherText( (std::istreambuf_iterator<char> (in)),
//                       std::istreambuf_iterator<char> () );
//
//    // watermark
//    string watermark = "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
//                       + std::to_string(RELEASE_CRYFA) + "\n";
//
//    string::size_type watermarkIdx = cipherText.find(watermark);
//    if (watermarkIdx == string::npos)
//    { cerr << "Error: invalid encrypted file!\n";    return; }
//    else  cipherText.erase(watermarkIdx, watermark.length());
//
//    if (verbose)
//    {
//        cerr << "cipher size: " << cipherText.size() - 1 << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
//    }
//
//    AES::Decryption aesDecryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
//    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
//    StreamTransformationFilter stfDecryptor(cbcDecryption,
//                                            new CryptoPP::StringSink(decText));
//    stfDecryptor.Put(reinterpret_cast<const byte*>
//                     (cipherText.c_str()), cipherText.size() - 1);
//    stfDecryptor.MessageEnd();
//
//    // process decrypted text
//    string tpl;     // tuplet
//    const ULL decTxtSize = decText.size() - 1;
//    const bool FASTA = (decText[0] == (char) 127);
//    const bool FASTQ = !FASTA; // const bool FASTQ = (decText[0] != (char) 127);
//    string::iterator i = decText.begin();
//
//    // FASTA
//    if (FASTA)
//    {
//        bool isHeader = true;
//        byte s;
//
//        ++i;    // jump over decText[0]
//        for (; i != decText.end()-1; ++i)   // exclude last symbol of decText
//        {
//            s = (byte) *i;
//            //empty line OR end of each seq line
//            if (s == 252 || (s == 254 && !isHeader)) { cout << '\n'; }
//                //seq len not multiple of 3
//            else if (s == 255) { cout << penaltySym(*(++i)); }
//                // header
//            else if (s == 253) { cout << '>';  isHeader = true; }
//            else if (isHeader) { cout << s; if (s == '\n') isHeader = false; }
//                // sequence
//            else //if (!isHeader)
//            {
//                tpl = DNA_UNPACK[s];
//
//                if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')            // ...
//                { cout<<tpl; }
//                    // using just one 'cout' makes trouble
//                else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')       // X..
//                { cout<<penaltySym(*(++i));    cout<<tpl[1];    cout<<tpl[2]; }
//
//                else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')       // .X.
//                { cout<<tpl[0];    cout<<penaltySym(*(++i));    cout<<tpl[2]; }
//
//                else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')       // XX.
//                { cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i));
//                    cout<<tpl[2]; }
//
//                else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')       // ..X
//                { cout<<tpl[0];    cout<<tpl[1];    cout<<penaltySym(*(++i)); }
//
//                else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')       // X.X
//                { cout<<penaltySym(*(++i));    cout<<tpl[1];
//                    cout<<penaltySym(*(++i)); }
//
//                else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')       // .XX
//                { cout<<tpl[0];    cout<<penaltySym(*(++i));
//                    cout<<penaltySym(*(++i)); }
//
//                else { cout<<penaltySym(*(++i));                          // XXX
//                    cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i)); }
//            }
//        }
//    }
//
//        // FASTQ
//    else // if (FASTQ)
//    {
//        string qsRange;
//        string hdrRange;
//        bool justPlus = true;
//
//        for (; *i != (char) 254; ++i)    hdrRange += *i;              // all hdr
//        ++i;    // jump over (char) 254
//        for (; *i != '\n' && *i != (char) 253; ++i)    qsRange += *i; // all qs
//        if (*i == '\n')  justPlus = false;              // if 3rd line is just +
//        ++i;   // jump over '\n' or (char) 253
//
//        const size_t qsRangeLen  = qsRange.length();
//        const size_t hdrRangeLen = hdrRange.length();
//        short keyLen_hdr = 0;
//        short keyLen_qs = 0;
//
////        // TEST
////        cerr << hdrRange << '\n' << hdrRange.length() << '\n';
////        cerr << qsRange << '\n' << qsRange.length() << '\n';
//
//        using unpackHdrPointer = string (*)(string::iterator&, string* &);
//        unpackHdrPointer unpackHdr;                          // function pointer
//        using unpackQSPointer = string (*)(string::iterator&, string* &);
//        unpackQSPointer unpackQS;                            // function pointer
//
//        // header
//        if (hdrRangeLen > MAX_CAT_5)    keyLen_hdr = KEYLEN_CAT_5;
//
//        else if (hdrRangeLen > MAX_CAT_4)                               // cat 5
//        { keyLen_hdr = KEYLEN_CAT_5;    unpackHdr = &unpack_read2B; }
//
//        else if (hdrRangeLen > MAX_CAT_3)                               // cat 4
//        { keyLen_hdr = KEYLEN_CAT_4;    unpackHdr = &unpack_read1B; }
//
//        else if (hdrRangeLen == MAX_CAT_3 || hdrRangeLen == MID_CAT_3   // cat 3
//                 || hdrRangeLen == MIN_CAT_3)
//        { keyLen_hdr = KEYLEN_CAT_3;    unpackHdr = &unpack_read1B; }
//
//        else if (hdrRangeLen == CAT_2)                                  // cat 2
//        { keyLen_hdr = KEYLEN_CAT_2;    unpackHdr = &unpack_read1B; }
//
//        else if (hdrRangeLen == CAT_1)                                  // cat 1
//        { keyLen_hdr = KEYLEN_CAT_1;    unpackHdr = &unpack_read1B; }
//
//        else { keyLen_hdr = 1;          unpackHdr = &unpack_read1B; }   // = 1
//
//        // quality score
//        if (qsRangeLen > MAX_CAT_5)     keyLen_qs = KEYLEN_CAT_5;
//
//        else if (qsRangeLen > MAX_CAT_4)                                // cat 5
//        { keyLen_qs = KEYLEN_CAT_5;     unpackQS = &unpack_read2B; }
//
//        else if (qsRangeLen > MAX_CAT_3)                                // cat 4
//        { keyLen_qs = KEYLEN_CAT_4;     unpackQS = &unpack_read1B; }
//
//        else if (qsRangeLen == MAX_CAT_3 || qsRangeLen == MID_CAT_3     // cat 3
//                 || qsRangeLen == MIN_CAT_3)
//        { keyLen_qs = KEYLEN_CAT_3;     unpackQS = &unpack_read1B; }
//
//        else if (qsRangeLen == CAT_2)                                   // cat 2
//        { keyLen_qs = KEYLEN_CAT_2;     unpackQS = &unpack_read1B; }
//
//        else if (qsRangeLen == CAT_1)                                   // cat 1
//        { keyLen_qs = KEYLEN_CAT_1;     unpackQS = &unpack_read1B; }
//
//        else { keyLen_qs = 1;           unpackQS = &unpack_read1B; }    // = 1
//
//        string plusMore;
//        if (hdrRangeLen > MAX_CAT_5 && qsRangeLen > MAX_CAT_5)
//        {
//            const string headers = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//            const string quality_scores = qsRange.substr(qsRangeLen-MAX_CAT_5);
//            // ASCII char after the last char in headers & quality_scores string
//            const char XChar_hdr = (char) (headers[headers.size() - 1] + 1);
//            const char XChar_qs =
//                    (char) (quality_scores[quality_scores.size() - 1] + 1);
//            string headers_X = headers;     headers_X += XChar_hdr;
//            string quality_scores_X=quality_scores;  quality_scores_X+=XChar_qs;
//
//            // tables for unpacking
//            buildUnpack(headers_X,        keyLen_hdr, HDR_UNPACK);
//            buildUnpack(quality_scores_X, keyLen_qs,  QS_UNPACK);
//
//            while (i != decText.end())
//            {
//                cout << '@';
//                cout << (plusMore = unpackLarge_read2B(i,XChar_hdr,HDR_UNPACK))
//                     << '\n'; ++i;//hdr
//                cout << unpackSeqFQ_3to1(i)                   << '\n';     //seq
//                cout << (justPlus ? "+" : "+" + plusMore)     << '\n'; ++i;// +
//                cout << unpackLarge_read2B(i, XChar_qs, QS_UNPACK) << '\n';//qs
//                // end of file
//                if (*(++i) == (char) 252)   break;
//            }
//        }
//        else if (hdrRangeLen > MAX_CAT_5 && qsRangeLen <= MAX_CAT_5)
//        {
//            const string headers = hdrRange.substr(hdrRangeLen - MAX_CAT_5);
//            // ASCII char after the last char in headers string
//            const char XChar_hdr = (char) (headers[headers.size() - 1] + 1);
//            string headers_X = headers;     headers_X += XChar_hdr;
//
//            // tables for unpacking
//            buildUnpack(headers_X, keyLen_hdr, HDR_UNPACK);
//            buildUnpack(qsRange,   keyLen_qs,  QS_UNPACK);
//
//            while (i != decText.end())
//            {
//                cout << '@';
//                cout << (plusMore = unpackLarge_read2B(i,XChar_hdr,HDR_UNPACK))
//                     << '\n'; ++i;//hdr
//                cout << unpackSeqFQ_3to1(i)                   << '\n';     //seq
//                cout << (justPlus ? "+" : "+" + plusMore)     << '\n'; ++i;// +
//                cout << unpackQS(i, QS_UNPACK)                << '\n';     //qs
//                // end of file
//                if (*(++i) == (char) 252)   break;
//            }
//        }
//        else if (hdrRangeLen <= MAX_CAT_5 && qsRangeLen > MAX_CAT_5)
//        {
//            const string quality_scores = qsRange.substr(qsRangeLen - MAX_CAT_5);
//            // ASCII char after the last char in quality_scores string
//            const char XChar_qs =
//                    (char) (quality_scores[quality_scores.size() - 1] + 1);
//            string quality_scores_X=quality_scores;  quality_scores_X+=XChar_qs;
//
//            // tables for unpacking
//            buildUnpack(hdrRange,         keyLen_hdr, HDR_UNPACK);
//            buildUnpack(quality_scores_X, keyLen_qs,  QS_UNPACK);
//
//            while (i != decText.end())
//            {
//                cout << '@';
//                cout << (plusMore = unpackHdr(i, HDR_UNPACK)) << '\n'; ++i;//hdr
//                cout << unpackSeqFQ_3to1(i)                   << '\n';     //seq
//                cout << (justPlus ? "+" : "+" + plusMore)     << '\n'; ++i;// +
//                cout << unpackLarge_read2B(i, XChar_qs, QS_UNPACK) << '\n';//qs
//                // end of file
//                if (*(++i) == (char) 252)   break;
//            }
//        }
//        else if (hdrRangeLen <= MAX_CAT_5 && qsRangeLen <= MAX_CAT_5)
//        {
//            // tables for unpacking
//            buildUnpack(hdrRange, keyLen_hdr, HDR_UNPACK);
//            buildUnpack(qsRange,  keyLen_qs,  QS_UNPACK);
//
//            while (i != decText.end())
//            {
//                cout << '@';
//                cout << (plusMore = unpackHdr(i, HDR_UNPACK)) << '\n'; ++i;//hdr
//                cout << unpackSeqFQ_3to1(i)                   << '\n';     //seq
//                cout << (justPlus ? "+" : "+" + plusMore)     << '\n'; ++i;// +
//                cout << unpackQS(i, QS_UNPACK)                << '\n';     //qs
//                // end of file
//                if (*(++i) == (char) 252)   break;
//            }
//        }
//    }   // end--FASTQ
//}
//
///*******************************************************************************
//    build IV
//*******************************************************************************/
//inline void EnDecrypto::buildIV (byte *iv, string pass)
//{
//    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
//    rng_type rng;
//
//    evalPassSize(pass);  // pass size must be >= 8
//
//    // using old rand to generate the new rand seed
//    srand((unsigned int) 7919 * pass[2] * pass[5] + 75653);
//    ULL seed = 0;
////    for (byte i = 0; i != pass.size(); ++i)
//    for (byte i = (byte) pass.size(); i--;)
//        seed += ((ULL) pass[i] * rand()) + rand();
//    seed %= 4294967295;
//
//    const rng_type::result_type seedval = seed;
//    rng.seed(seedval);
//
////    for (unsigned int i = 0; i != AES::BLOCKSIZE; ++i)
//    for (unsigned int i = (unsigned int) AES::BLOCKSIZE; i--;)
//        iv[i] = (byte) (udist(rng) % 255);
//}
//
///*******************************************************************************
//    build key
//*******************************************************************************/
//inline void EnDecrypto::buildKey (byte *key, string pwd)
//{
//    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
//    rng_type rng;
//
//    evalPassSize(pwd);  // pass size must be >= 8
//
//    // using old rand to generate the new rand seed
//    srand((unsigned int) 24593 * (pwd[0] * pwd[2]) + 49157);
//    ULL seed = 0;
////    for (byte i = 0; i != pwd.size(); ++i)
//    for (byte i = (byte) pwd.size(); i--;)
//        seed += ((ULL) pwd[i] * rand()) + rand();
//    seed %= 4294967295;
//
//    const rng_type::result_type seedval = seed;
//    rng.seed(seedval);
//
////    for (unsigned int i = 0; i != AES::DEFAULT_KEYLENGTH; ++i)
//    for (unsigned int i = (unsigned int) AES::DEFAULT_KEYLENGTH; i--;)
//        key[i] = (byte) (udist(rng) % 255);
//}
//
///*******************************************************************************
//    print IV
//*******************************************************************************/
//inline void EnDecrypto::printIV (byte *iv) const
//{
//    cerr << "IV = [" << (int) iv[0];
//    for (unsigned int i = 1; i != AES::BLOCKSIZE; ++i)
//        cerr << " " << (int) iv[i];
//    cerr << "]\n";
//}
//
///*******************************************************************************
//    print key
//*******************************************************************************/
//inline void EnDecrypto::printKey (byte *key) const
//{
//    cerr << "KEY: [" << (int) key[0];
//    for (unsigned int i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
//        cerr << " " << (int) key[i];
//    cerr << "]\n";
//}
//
///*******************************************************************************
//    find file type: FASTA or FASTQ
//*******************************************************************************/
//inline char EnDecrypto::findFileType (std::ifstream &in)
//{
//    string line;
//
//    // FASTQ
//    while (getline(in, line).good())
//    {
//        if (line[0] == '@')
//        {
//            in.clear();
//            in.seekg(0, std::ios::beg); // go to the beginning of file
//            return 'Q';
//        }
//    }
//
//    // FASTA
//    in.clear();  in.seekg(0, std::ios::beg);  return 'A';
//}
//
///*******************************************************************************
//    get password from a file
//*******************************************************************************/
//inline string EnDecrypto::getPassFromFile () const
//{
//    ifstream input(keyFileName);
//    string line;
//
//    if (keyFileName.empty())
//    {
//        cerr << "Error: no password file has been set!\n";
//        exit(1);
//    }
//    else if (!input.good())
//    {
//        cerr << "Error opening '" << keyFileName << "'.\n";
//        exit(1);
//    }
//
//    while (getline(input, line).good())
//    {
//        if (line.empty()) {cerr<<"Error: empty password line file!\n"; exit(1);}
//        return line;
//    }
//
//    return "unknown";
//}
//
///*******************************************************************************
//    evaluate password size >= 8
//*******************************************************************************/
//inline void EnDecrypto::evalPassSize (const string &pass) const
//{
//    if (pass.size() < 8)
//    {
//        cerr << "Error: password size must be at least 8!\n";
//        exit(1);
//    }
//}