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
#include <algorithm>
#include "EnDecrypto.h"
#include "pack.h"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include "cryptopp/modes.h"
using std::vector;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::to_string;
using std::thread;
using std::stoull;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::CBC_Mode;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::FileSource;
using CryptoPP::FileSink;

std::mutex mutx;

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
//    ifstream in(inFileName);
//    if (!in.good())
//    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
//
//    string line, seq, context;  // FASTA: context = header + seq (+ empty lines)
//
//    // watermark for encrypted file
//    cout << "#cryfa v" + to_string(VERSION_CRYFA) + "."
//                       + to_string(RELEASE_CRYFA) + "\n";
//
//    // to tell decryptor this isn't FASTQ
//    context += (char) 127;      // context += "\n";
//    while (getline(in, line).good())
//    {
//        // header
//        if (line[0] == '>')
//        {
//            if (!seq.empty())   context += packSeq_3to1(seq);   // previous seq
//            seq.clear();
//
//            // header line. (char) 253 instead of '>'
//            context += (char) 253 + line.substr(1) + "\n";
//        }
//
//        // empty line. (char) 252 instead of line feed
//        else if (line.empty())    seq += (char) 252;
//
//        // sequence
//        else
//        {
//            if (line.find(' ') != string::npos)
//            { cerr << "Invalid sequence -- spaces not allowed.\n";    exit(1); }
//            // (char) 254 instead of '\n' at the end of each seq line
//            seq += line + (char) 254;
//        }
//    }
//    if (!seq.empty())   context += packSeq_3to1(seq);           // the last seq
//
//    in.close();
//
//    // encryption
//    encrypt();      // cout encrypted content
//    cout << '\n';   //todo. probably should comment this
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
    string line;
    thread arrThread[n_threads];
    byte t;         // for threads
    ull startLine;  // for each thread
    string headers, qscores;
    
    // gather all headers and quality scores
    gatherHdrQs(headers, qscores);
    
    // function pointers
    using packHdrPointer = string (*)(const string&, const htable_t&);
    packHdrPointer packHdr;
    using packQSPointer  = string (*)(const string&, const htable_t&);
    packQSPointer packQS;

    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();

    // header
    if (headersLen > MAX_C5)          // if len > 39 filter the last 39 ones
    {
        Hdrs = headers.substr(headersLen - MAX_C5);
        Hdrs_g = Hdrs;
        // ASCII char after the last char in Hdrs -- always <= (char) 127
        HdrsX = Hdrs;    HdrsX += (char) (Hdrs.back() + 1);
        HdrMap=buildHashTable(HdrsX, KEYLEN_C5);     packHdr=&packLargeHdr_3to2;
    }
    else
    {
        Hdrs = headers;
        Hdrs_g = Hdrs;

        if (headersLen > MAX_C4)                                        // cat 5
        { HdrMap = buildHashTable(Hdrs, KEYLEN_C5);   packHdr = &pack_3to2; }

        else if (headersLen > MAX_C3)                                   // cat 4
        { HdrMap = buildHashTable(Hdrs, KEYLEN_C4);   packHdr = &pack_2to1; }

        else if (headersLen == MAX_C3 || headersLen == MID_C3
                 || headersLen == MIN_C3)                               // cat 3
        { HdrMap = buildHashTable(Hdrs, KEYLEN_C3);   packHdr = &pack_3to1; }

        else if (headersLen == C2)                                      // cat 2
        { HdrMap = buildHashTable(Hdrs, KEYLEN_C2);   packHdr = &pack_5to1; }

        else if (headersLen == C1)                                      // cat 1
        { HdrMap = buildHashTable(Hdrs, KEYLEN_C1);   packHdr = &pack_7to1; }

        else                                                   // headersLen = 1
        { HdrMap = buildHashTable(Hdrs, 1);           packHdr = &pack_1to1; }
    }
    
    // quality score
    if (qscoresLen > MAX_C5)           // if len > 39 filter the last 39 ones
    {
        QSs = qscores.substr(qscoresLen - MAX_C5);
        QSs_g = QSs;
        // ASCII char after last char in QUALITY_SCORES
        QSsX = QSs;     QSsX += (char) (QSs.back() + 1);
        QsMap = buildHashTable(QSsX, KEYLEN_C5);    packQS = &packLargeQs_3to2;
    }
    else
    {
        QSs = qscores;
        QSs_g = QSs;
        
        if (qscoresLen > MAX_C4)                                        // cat 5
        { QsMap = buildHashTable(QSs, KEYLEN_C5);   packQS = &pack_3to2; }

        else if (qscoresLen > MAX_C3)                                   // cat 4
        { QsMap = buildHashTable(QSs, KEYLEN_C4);   packQS = &pack_2to1; }

        else if (qscoresLen == MAX_C3 || qscoresLen == MID_C3
                 || qscoresLen == MIN_C3)                               // cat 3
        { QsMap = buildHashTable(QSs, KEYLEN_C3);   packQS = &pack_3to1; }

        else if (qscoresLen == C2)                                      // cat 2
        { QsMap = buildHashTable(QSs, KEYLEN_C2);   packQS = &pack_5to1; }

        else if (qscoresLen == C1)                                      // cat 1
        { QsMap = buildHashTable(QSs, KEYLEN_C1);   packQS = &pack_7to1; }
        
        else                                                   // qscoresLen = 1
        { QsMap = buildHashTable(QSs, 1);           packQS = &pack_1to1; }
    }
    
    // distribute file among threads, for reading and packing
    for (ull i = 0; !isInEmpty; ++i)
    {
        isInEmpty = false;
        
        for (t = 0; t != n_threads; ++t)
        {
            startLine = (i*n_threads + t) * LINE_BUFFER;
            arrThread[t] = thread(&EnDecrypto::pack, this,
                                  startLine, t, packHdr, packQS);
        }
        for (t = 0; t != n_threads; ++t)    arrThread[t].join();
    }
    
    // join encrypted files
    ifstream encFile[n_threads];
    string context;
    
    // watermark for encrypted file
    cout << "#cryfa v" + to_string(VERSION_CRYFA) + "."
                       + to_string(RELEASE_CRYFA) + "\n";
    
    // open packed file
    ofstream pkdFile;
    pkdFile.open(PKD_FILENAME);
    
    pkdFile << headers;                             // send headers to decryptor
    pkdFile << (char) 254;                          // to detect headers in dec.
    pkdFile << qscores;                             // send qscores to decryptor
    pkdFile << (hasFQjustPlus() ? (char) 253 : '\n');             // if just '+'
    
//  context += headers;                             // send headers to decryptor
//  context += (char) 254;                          // to detect headers in dec.
//  context += qscores;                             // send qscores to decryptor
//  context += (hasFQjustPlus() ? (char) 253 : '\n');             // if just '+'
////    out << context ;//<< '\n';    //todo. too aes cbc mode nemishe
    
    // open input files
    for (t = n_threads; t--;)   encFile[t].open(ENC_FILENAME + to_string(t));
    
    bool prevLineNotThrID;                 // if previous line was "THR=" or not
    while (!encFile[0].eof())
    {
        for (t = 0; t != n_threads; ++t)
        {
            prevLineNotThrID = false;
            
            while (getline(encFile[t], line).good() &&
                    line.compare(THR_ID_HDR+to_string(t)))
            {
                if (prevLineNotThrID)   pkdFile << '\n';
                pkdFile << line;
//                if (prevLineNotThrID)   context += '\n';
//                context += line;
                
                prevLineNotThrID = true;
            }
        }
    }
    pkdFile << (char) 252;
//    context += (char) 252;
    
    for (t = n_threads; t--;)  encFile[t].close(); // close input & output files
    pkdFile.close();                               // close packed file
    
    encrypt();      // cout encrypted content
//    cout << '\n';
    
    /*
    // get size of file
    infile.seekg(0, infile.end);
    long size = 1000000;//infile.tellg();
    infile.seekg(0);

    // allocate memory for file content
    char *buffer = new char[size];

    // read content of infile
    infile.read(buffer, size);

    // write to outfile
    outfile.write(buffer, size);

    // release dynamically-allocated memory
    delete[] buffer;

    outfile.close();
    infile.close();
    */
}

/*******************************************************************************
    pack -- '@' at the beginning of headers is not packed
*******************************************************************************/
inline void EnDecrypto::pack (const ull startLine, const byte threadID,
                              string (*packHdr)(const string&, const htable_t&),
                              string (*packQS)(const string&, const htable_t&))
{
    ifstream in(inFileName);
    string context; // output string
    string inTempStr;
    string line;
    
    for (ull l = 0; l != startLine; ++l)    in.ignore(LARGE_NUMBER, '\n');
    
//    // beginning of the part of file for this thread
//    pos_t pos_beg = in.tellg();
    
    if (in.peek()==EOF) { isInEmpty = true;    return; }
    
    for (ull l = 0; l != LINE_BUFFER; l += 4)     // process 4 lines by 4 lines
    {
        if (getline(in, line).good())           // header -- ignore '@'
            context += packHdr(line.substr(1), HdrMap) + (char) 254;

        if (getline(in, line).good())           // sequence
            context += packSeq_3to1(line) + (char) 254;

        in.ignore(LARGE_NUMBER, '\n');          // +. ignore

        if (getline(in, line).good())           // quality score
            context += packQS(line, QsMap) + (char) 254;
    }
    
    // shuffle
    if (!disable_shuffle)    shufflePkd(context);
    
    // for unshuffling: insert the size of packed context in the beginning of it
    string contextSize;
    contextSize += (char) 253;
    contextSize += to_string(context.size());
    contextSize += (char) 254;
    context.insert(0, contextSize);
    
    /*
    i = in.begin();
    while (i != in.end())
    {
        // header -- ignore '@'
        inTempStr.clear();
        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
        context += packHdr(inTempStr, HEADERS, HDR_MAP) + (char) 254;

        // sequence
        inTempStr.clear();
        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
        context += packSeq_3to1(inTempStr) + (char) 254;

        // +. ignore
        for (i += 1; *i != '\n'; ++i);

        // quality score
        inTempStr.clear();
        for (i += 1; *i != '\n'; ++i)   inTempStr += *i;
        context += packQS(inTempStr, QUALITY_SCORES, QS_MAP) + (char) 254;

        i += 1;
    }
    */
    
    ofstream encfile;
    encfile.open(ENC_FILENAME+to_string(threadID), std::ios_base::app);
    
    // write header containing threadID for each
    encfile << THR_ID_HDR + to_string(threadID) << '\n';
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
inline void EnDecrypto::encrypt ()
{
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    // encrypt
    const char* inFile = PKD_FILENAME;
    CBC_Mode<CryptoPP::AES>::Encryption
            cbcEnc(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(inFile, true,
               new StreamTransformationFilter(cbcEnc, new FileSink(cout)));
    
    /*
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
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
    for (const char &c : cipherText)
        encryptedText += (char) (c & 0xFF);
////        encryptedText += (char) (0xFF & static_cast<byte> (c));

////    encryptedText+='\n';
    return encryptedText;
    */
}

/*******************************************************************************
    decompress
*******************************************************************************/
void EnDecrypto::decompress ()
{
    decrypt();                                                        // decrypt
    ifstream in(DEC_FILENAME);
    (in.peek() == (char) 127) ? decompFA() : decompFQ();              // decomp
    in.close();
}

/*******************************************************************************
    decrypt.
    AES encryption uses a secret key of a variable length (128, 196 or 256 bit).
    This key is secretly exchanged between two parties before communication
    begins. DEFAULT_KEYLENGTH = 16 bytes.
*******************************************************************************/
inline void EnDecrypto::decrypt ()
{
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    // watermark
    string watermark = "#cryfa v" + to_string(VERSION_CRYFA) + "."
                                  + to_string(RELEASE_CRYFA) + "\n";
    string line;    getline(in, line);
    if ((line+"\n") != watermark)
    { cerr << "Error: invalid encrypted file!\n";    exit(1); }
    
    ofstream encnw(ENW_FILENAME);
    char c;     while (in.get(c)) encnw << c;
    
    // close open files -- is a MUST (for encnw)
    encnw.close();
    in.close();
    
////    string::size_type watermarkIdx = cipherText.find(watermark);
////    if (watermarkIdx == string::npos)
////    { cerr << "Error: invalid encrypted file!\n";    exit(1); }
////    else  cipherText.erase(watermarkIdx, watermark.length());

    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug

//    string cipherText( (std::istreambuf_iterator<char> (in)),
//                       std::istreambuf_iterator<char> () );

//    if (verbose)
//    {
//        cerr << "cipher size: " << cipherText.size()-1 << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
//    }
    
    const char* inFile  = ENW_FILENAME;
    const char* outFile = DEC_FILENAME;
    CBC_Mode<CryptoPP::AES>::Decryption
            cbcDec(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(inFile, true,
               new StreamTransformationFilter(cbcDec, new FileSink(outFile)));
}

/*******************************************************************************
    decompress FASTA.
    * reserved symbols:
          (char) 255:  penalty if sequence length isn't multiple of 3
          (char) 254:  end of each sequence line
          (char) 253:  instead of '>' in header
          (char) 252:  instead of empty line
*******************************************************************************/
//inline void EnDecrypto::decompFA (string decText)
inline void EnDecrypto::decompFA ()
{
//    string line;
//    string tpl;     // tuplet
//    string::iterator i = decText.begin();
//
//    bool isHeader = true;
//    byte s;
//
//    ++i;    // jump over decText[0]
//    for (; i != decText.end()-1; ++i)   // exclude last symbol of decText
//    {
//        s = (byte) *i;
//        //empty line OR end of each seq line
//        if (s == 252 || (s == 254 && !isHeader)) { cout << '\n'; }
//            //seq len not multiple of 3
//        else if (s == 255) { cout << penaltySym(*(++i)); }
//            // header
//        else if (s == 253) { cout << '>';  isHeader = true; }
//        else if (isHeader) { cout << s; if (s == '\n') isHeader = false; }
//            // sequence
//        else //if (!isHeader)
//        {
//            tpl = DNA_UNPACK[s];
//
//            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')                // ...
//            { cout<<tpl; }
//                // using just one 'cout' makes trouble
//            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')           // X..
//            { cout<<penaltySym(*(++i));    cout<<tpl[1];    cout<<tpl[2]; }
//
//            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')           // .X.
//            { cout<<tpl[0];    cout<<penaltySym(*(++i));    cout<<tpl[2]; }
//
//            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')           // XX.
//            { cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i));
//              cout<<tpl[2]; }
//
//            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')           // ..X
//            { cout<<tpl[0];    cout<<tpl[1];    cout<<penaltySym(*(++i)); }
//
//            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')           // X.X
//            { cout<<penaltySym(*(++i));    cout<<tpl[1];
//              cout<<penaltySym(*(++i)); }
//
//            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')           // .XX
//            { cout<<tpl[0];    cout<<penaltySym(*(++i));
//              cout<<penaltySym(*(++i)); }
//
//            else { cout<<penaltySym(*(++i));                              // XXX
//                cout<<penaltySym(*(++i));    cout<<penaltySym(*(++i)); }
//        }
//    }
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
inline void EnDecrypto::decompFQ ()
{
    string decText;
    string::iterator i;         // iterator in decText
    ull chunkSize;              // size of each chunk of file
    char c;                     // chars in file
    vector<string> hdrUnpack;   // for unpacking header
    vector<string> qsUnpack;    // for unpacking quality score
    string headers, qscores;
    bool justPlus = true;
    string chunkSizeStr;        // chunk size (string) -- for unshuffling
    
    ifstream in(DEC_FILENAME);
    while (in.get(c) && c != (char) 254)                 headers += c;
    while (in.get(c) && c != '\n' && c != (char) 253)    qscores += c;
    if (c == '\n')    justPlus = false;                 // if 3rd line is just +
    
    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();
    us keyLen_hdr = 0,  keyLen_qs = 0;
    
    using unpackHdrPointer = string (*)(string::iterator&, vector<string>&);
    unpackHdrPointer unpackHdr;                              // function pointer
    using unpackQSPointer = string (*)(string::iterator&, vector<string>&);
    unpackQSPointer unpackQS;                                // function pointer
    
    // header
    if (headersLen > MAX_C5)    keyLen_hdr = KEYLEN_C5;

    else if (headersLen > MAX_C4)                               // cat 5
    { keyLen_hdr = KEYLEN_C5;   unpackHdr = &unpack_read2B; }

    else if (headersLen > MAX_C3)                               // cat 4
    { keyLen_hdr = KEYLEN_C4;   unpackHdr = &unpack_read1B; }

    else if (headersLen == MAX_C3 || headersLen == MID_C3       // cat 3
             || headersLen == MIN_C3)
    { keyLen_hdr = KEYLEN_C3;   unpackHdr = &unpack_read1B; }

    else if (headersLen == C2)                                  // cat 2
    { keyLen_hdr = KEYLEN_C2;   unpackHdr = &unpack_read1B; }

    else if (headersLen == C1)                                  // cat 1
    { keyLen_hdr = KEYLEN_C1;   unpackHdr = &unpack_read1B; }

    else { keyLen_hdr = 1;      unpackHdr = &unpack_read1B; }   // = 1
    
    // quality score
    if (qscoresLen > MAX_C5)    keyLen_qs = KEYLEN_C5;

    else if (qscoresLen > MAX_C4)                               // cat 5
    { keyLen_qs = KEYLEN_C5;    unpackQS = &unpack_read2B; }

    else if (qscoresLen > MAX_C3)                               // cat 4
    { keyLen_qs = KEYLEN_C4;    unpackQS = &unpack_read1B; }

    else if (qscoresLen == MAX_C3 || qscoresLen == MID_C3       // cat 3
             || qscoresLen == MIN_C3)
    { keyLen_qs = KEYLEN_C3;    unpackQS = &unpack_read1B; }

    else if (qscoresLen == C2)                                  // cat 2
    { keyLen_qs = KEYLEN_C2;    unpackQS = &unpack_read1B; }

    else if (qscoresLen == C1)                                  // cat 1
    { keyLen_qs = KEYLEN_C1;    unpackQS = &unpack_read1B; }

    else { keyLen_qs = 1;       unpackQS = &unpack_read1B; }    // = 1

    string plusMore;
    if (headersLen > MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        const string decQscores = qscores.substr(qscoresLen-MAX_C5);
        // ASCII char after the last char in headers & quality_scores string
        const char XChar_hdr = (char) (decHeaders[decHeaders.size()-1] + 1);
        const char XChar_qs  = (char) (decQscores[decQscores.size()-1] + 1);
        string decHeadersX = decHeaders;    decHeadersX += XChar_hdr;
        string decQscoresX = decQscores;    decQscoresX += XChar_qs;

        // tables for unpacking
        hdrUnpack = buildUnpack(decHeadersX, keyLen_hdr);
        qsUnpack  = buildUnpack(decQscoresX, keyLen_qs);
    
        while (in.get(c) && c != (char) 252)
        {
            if (c == (char) 253)
            {
                decText.clear();
                chunkSizeStr.clear();   // chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                chunkSize = stoull(chunkSizeStr);
            
                // take a chunk of decrypted file
                for (ull u = chunkSize; u--;) { in.get(c);    decText += c; }
                i = decText.begin();
                
                // unshuffle
                if (!disable_shuffle)    unshufflePkd(i, chunkSize);
            }
        
            do {
                cout << '@';
                cout << (plusMore = unpackLarge_read2B(i, XChar_hdr, hdrUnpack))
                     <<'\n';  ++i; //hdr
            
                cout << unpackSeqFQ_3to1(i)                  <<'\n';       //seq
                cout << (justPlus ? "+" : "+" + plusMore)    <<'\n';  ++i; //+
                cout << unpackLarge_read2B(i, XChar_qs, qsUnpack) <<'\n';  //qs
            } while (++i != decText.end());    // if trouble: change "!=" to "<"
        }
    }
    else if (headersLen > MAX_C5 && qscoresLen <= MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        // ASCII char after the last char in headers string
        const char XChar_hdr = (char) (decHeaders[decHeaders.size()-1] + 1);
        string decHeadersX = decHeaders;     decHeadersX += XChar_hdr;

        // tables for unpacking
        hdrUnpack = buildUnpack(decHeadersX, keyLen_hdr);
        qsUnpack  = buildUnpack(qscores,     keyLen_qs);
    
        while (in.get(c) && c != (char) 252)
        {
            if (c == (char) 253)
            {
                decText.clear();
                chunkSizeStr.clear();   // chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                chunkSize = stoull(chunkSizeStr);
            
                // take a chunk of decrypted file
                for (ull u = chunkSize; u--;) { in.get(c);    decText += c; }
                i = decText.begin();
            
                // unshuffle
                if (!disable_shuffle)    unshufflePkd(i, chunkSize);
            }
        
            do {
                cout << '@';
                cout << (plusMore = unpackLarge_read2B(i, XChar_hdr, hdrUnpack))
                                                             <<'\n';  ++i; //hdr

                cout << unpackSeqFQ_3to1(i)                  <<'\n';       //seq
                cout << (justPlus ? "+" : "+" + plusMore)    <<'\n';  ++i; //+
                cout << unpackQS(i, qsUnpack)                <<'\n';       //qs
            } while (++i != decText.end());    // if trouble: change "!=" to "<"
        }
    }
    else if (headersLen <= MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decQscores = qscores.substr(qscoresLen - MAX_C5);
        // ASCII char after the last char in decQscores string
        const char XChar_qs=(char)(decQscores[decQscores.size()-1] + 1);
        string decQscoresX = decQscores;    decQscoresX += XChar_qs;

        // tables for unpacking
        hdrUnpack = buildUnpack(headers,     keyLen_hdr);
        qsUnpack  = buildUnpack(decQscoresX, keyLen_qs);
    
        while (in.get(c) && c != (char) 252)
        {
            if (c == (char) 253)
            {
                decText.clear();
                chunkSizeStr.clear();   // chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                chunkSize = stoull(chunkSizeStr);
            
                // take a chunk of decrypted file
                for (ull u = chunkSize; u--;) { in.get(c);    decText += c; }
                i = decText.begin();
            
                // unshuffle
                if (!disable_shuffle)    unshufflePkd(i, chunkSize);
            }
            
            do {
                cout << '@';
                cout << (plusMore = unpackHdr(i, hdrUnpack)) <<'\n';  ++i; //hdr
                cout << unpackSeqFQ_3to1(i)                  <<'\n';       //seq
                cout << (justPlus ? "+" : "+" + plusMore)    <<'\n';  ++i; //+
                cout << unpackLarge_read2B(i, XChar_qs, qsUnpack) <<'\n';  // qs
            } while (++i != decText.end());    // if trouble: change "!=" to "<"
        }
    }
    else if (headersLen <= MAX_C5 && qscoresLen <= MAX_C5)
    {
        // tables for unpacking
        hdrUnpack = buildUnpack(headers, keyLen_hdr);
        qsUnpack  = buildUnpack(qscores, keyLen_qs);
        
        while (in.get(c) && c != (char) 252)
        {
            if (c == (char) 253)
            {
                decText.clear();
                chunkSizeStr.clear();   // chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                chunkSize = stoull(chunkSizeStr);
                
                // take a chunk of decrypted file
                for (ull u = chunkSize; u--;) { in.get(c);    decText += c; }
                i = decText.begin();
                
                // unshuffle
                if (!disable_shuffle)    unshufflePkd(i, chunkSize);
            }
            
            do {
                cout << '@';
                cout << (plusMore = unpackHdr(i, hdrUnpack)) <<'\n';  ++i; //hdr
                cout << unpackSeqFQ_3to1(i)                  <<'\n';       //seq
                cout << (justPlus ? "+" : "+" + plusMore)    <<'\n';  ++i; //+
                cout << unpackQS(i, qsUnpack)                <<'\n';       //qs
            } while (++i != decText.end());    // if trouble: change "!=" to "<"
        }
    }
    
    // close decrypted file
    in.close();
}

/*******************************************************************************
    check if the third line contains only +
*******************************************************************************/
inline bool EnDecrypto::hasFQjustPlus () const
{
    ifstream in(inFileName);
    string line;
    
    in.ignore(LARGE_NUMBER, '\n');          // ignore header
    in.ignore(LARGE_NUMBER, '\n');          // ignore seq
    bool justPlus = !(getline(in, line).good() && line.length() > 1);
    
    in.close();
    return justPlus;
    
    /** if input was string, instead of file
    // check if the third line contains only +
    bool justPlus = true;
    string::const_iterator lFFirst = std::find(in.begin(), in.end(), '\n');
    string::const_iterator lFSecond = std::find(lFFirst+1, in.end(), '\n');
    if (*(lFSecond+2) != '\n')  justPlus = false;   // check symbol after +
    */
}

/*******************************************************************************
    gather all headers and quality scores. ignore '@' from headers and sort them
*******************************************************************************/
inline void EnDecrypto::gatherHdrQs (string& headers, string& qscores) const
{
    bool hChars[127], qChars[127];
    std::memset(hChars+32, false, 95);
    std::memset(qChars+32, false, 95);
    
    ifstream in(inFileName);
    string line;
    while (!in.eof())
    {
        if (getline(in,line).good())  for(const char &c : line)  hChars[c]=true;
        in.ignore(LARGE_NUMBER, '\n');                        // ignore sequence
        in.ignore(LARGE_NUMBER, '\n');                        // ignore +
        if (getline(in,line).good())  for(const char &c : line)  qChars[c]=true;
    }
    in.close();
    
    // gather the characters -- ignore '@'=64 for headers
    for (byte i = 32; i != 64;  ++i)    if (*(hChars+i))  headers += i;
    for (byte i = 65; i != 127; ++i)    if (*(hChars+i))  headers += i;
    for (byte i = 32; i != 127; ++i)    if (*(qChars+i))  qscores += i;
    
    
    /** IDEA
    ui  hL=0, qL=0;
    ull hH=0, qH=0;
    string headers, qscores;
    ifstream in(inFileName);
    string line;
    while (!in.eof())
    {
        if (getline(in, line).good())
            for (const char &c : line)
                (c & 0xC0) ? (hH |= 1ULL<<(c-64)) : (hL |= 1U<<(c-32));
        in.ignore(LARGE_NUMBER, '\n');                        // ignore sequence
        in.ignore(LARGE_NUMBER, '\n');                        // ignore +
        if (getline(in, line).good())
            for (const char &c : line)
                (c & 0xC0) ? (qH |= 1ULL<<(c-64)) : (qL |= 1U<<(c-32));
    }
    in.close();

    // gather the characters -- ignore '@'=64 for headers
    for (byte i = 0; i != 32; ++i)    if (hL>>i & 1)  headers += i+32;
    for (byte i = 1; i != 62; ++i)    if (hH>>i & 1)  headers += i+64;
    for (byte i = 0; i != 32; ++i)    if (qL>>i & 1)  qscores += i+32;
    for (byte i = 1; i != 62; ++i)    if (qH>>i & 1)  qscores += i+64;
    */
    
    /** OLD
    while (!in.eof())
    {
        if (getline(in, line).good())               // header
        {
            for (const char &c : line)
                if (headers.find(c) == string::npos)
                    headers += c;
        }
        in.ignore(LARGE_NUMBER, '\n');              // ignore sequence
        in.ignore(LARGE_NUMBER, '\n');              // ignore +
        if (getline(in, line).good())               // quality score
        {
            for (const char &c : line)
                if (qscores.find(c) == string::npos)
                    qscores += c;
        }
    }
    in.close();

    headers.erase(headers.begin());                 // ignore '@'
    std::sort(headers.begin(), headers.end());      // sort values
    std::sort(qscores.begin(), qscores.end());      // sort ASCII values
    */
}

/*******************************************************************************
    random number engine
*******************************************************************************/
inline std::minstd_rand0 &EnDecrypto::randomEngine ()
{
    static std::minstd_rand0 e{};
    return e;
}

/*******************************************************************************
    random number seed -- emulate C srand()
*******************************************************************************/
inline void EnDecrypto::my_srand (const ui s)
{
    randomEngine().seed(s);
}

/*******************************************************************************
    random number generate -- emulate C rand()
*******************************************************************************/
inline int EnDecrypto::my_rand ()
{
    return (int) (randomEngine()() - randomEngine().min());
}

/*******************************************************************************
    shuffle/unshuffle seed generator -- for each chunk
*******************************************************************************/
//inline ull EnDecrypto::un_shuffleSeedGen (const ui seedInit)
inline void EnDecrypto::un_shuffleSeedGen ()
{//todo. can't we generate this seed once?
    const string pass = extractPass();
    evalPassSize(pass);     // pass size must be >= 8
    
    ull passDigitsMult = 1; // multiplication of all pass digits
    for (ui i = (ui) pass.size(); i--;)    passDigitsMult *= pass[i];
    
    // using old rand to generate the new rand seed
    ull seed = 0;
    
    mutx.lock();//--------------------------------------------------------------
//    my_srand(20543 * seedInit * (ui) passDigitsMult + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += ((ull) pass[i] * my_rand()) + my_rand();

//    my_srand(20543 * seedInit + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += (ull) pass[i] * my_rand();
    my_srand(20543 * (ui) passDigitsMult + 81647);
    for (byte i = (byte) pass.size(); i--;)
        seed += (ull) pass[i] * my_rand();
    mutx.unlock();//------------------------------------------------------------
    
//    seed %= 2106945901;
 
    seed_shared = seed;
//    return seed;
}

/*******************************************************************************
    shuffle
*******************************************************************************/
inline void EnDecrypto::shufflePkd (string &in)
{
//    const ull seed = un_shuffleSeedGen((ui) in.size());    // shuffling seed
//    std::shuffle(in.begin(), in.end(), std::mt19937(seed));
    un_shuffleSeedGen();    // shuffling seed
    std::shuffle(in.begin(), in.end(), std::mt19937(seed_shared));
}

/*******************************************************************************
    unshuffle
*******************************************************************************/
inline void EnDecrypto::unshufflePkd (string::iterator &i, const ull size)
{
    string shuffledStr;     // copy of shuffled string
    for (ull j = 0; j != size; ++j, ++i)    shuffledStr += *i;
    string::iterator shIt = shuffledStr.begin();
    i -= size;
    
    // shuffle vector of positions
    vector<ull> vPos(size);
    std::iota(vPos.begin(), vPos.end(), 0);     // insert 0 .. N-1
//    const ull seed = un_shuffleSeedGen((ui) size);
//    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed));
    un_shuffleSeedGen();
    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed_shared));

    // insert unshuffled data
    for (const ull& vI : vPos)  *(i + vI) = *shIt++;       // *shIt, then ++shIt
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
    srand((ui) 7919 * pass[2] * pass[5] + 75653);
    ull seed = 0;
    for (byte i = (byte) pass.size(); i--;)
        seed += ((ull) pass[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (ui i = (ui) AES::BLOCKSIZE; i--;)
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
    srand((ui) 24593 * (pwd[0] * pwd[2]) + 49157);
    ull seed = 0;
    for (byte i = (byte) pwd.size(); i--;)
        seed += ((ull) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (ui i = (ui) AES::DEFAULT_KEYLENGTH; i--;)
        key[i] = (byte) (udist(rng) % 255);
}

/*******************************************************************************
    print IV
*******************************************************************************/
inline void EnDecrypto::printIV (byte *iv) const
{
    cerr << "IV = [" << (int) iv[0];
    for (ui i = 1; i != AES::BLOCKSIZE; ++i)
        cerr << " " << (int) iv[i];
    cerr << "]\n";
}

/*******************************************************************************
    print key
*******************************************************************************/
inline void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (ui i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}

/*******************************************************************************
    get password from a file
*******************************************************************************/
inline string EnDecrypto::extractPass () const
{
    ifstream in(keyFileName);
    string l;   // each line of file
    
    if (keyFileName.empty())
    { cerr << "Error: no password file has been set!\n";     exit(1); }
    else if (!in.good())
    { cerr << "Error opening '" << keyFileName << "'.\n";    exit(1); }
    
    while (getline(in, l).good())
    {
        if (l.empty()) { cerr<<"Error: empty password line file!\n";  exit(1); }
        return l;
    }
    
    return "UNKNOWN";
}

/*******************************************************************************
    evaluate password size >= 8
*******************************************************************************/
inline void EnDecrypto::evalPassSize (const string &pass) const
{
    if (pass.size() < 8)
    { cerr << "Error: password size must be at least 8!\n";    exit(1); }
}