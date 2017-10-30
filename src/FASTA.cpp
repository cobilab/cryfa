/**
 * @file      FASTA.cpp
 * @brief     Compression/Decompression of FASTA
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include <cstring>
#include "FASTA.h"

//#include <functional>
//#include <algorithm>

using std::chrono::high_resolution_clock;
using std::thread;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::to_string;
using std::setprecision;
using std::memset;

std::mutex mutxFA;    /**< @brief Mutex */


/**
 * @brief Compress FASTA
 */
void FASTA::compressFA ()
{
    // Start timer for compression
    high_resolution_clock::time_point startTime = high_resolution_clock::now();

    thread   arrThread[n_threads];
    byte     t;           // For threads
    string   headers;
    packfa_s pkStruct;    // Collection of inputs to pass to pack...

    if (verbose)    cerr << "Calculating number of different characters...\n";
    // Gather different chars in all headers and max length in all bases
    gatherHdrBs(headers);

    const size_t headersLen = headers.length();
    // Show number of different chars in headers -- ignore '>'=62
    if (verbose)    cerr << "In headers, they are " << headersLen << ".\n";

    // Function pointer
    using packHdrPointer = void (EnDecrypto::*)
                                (string&, const string&, const htbl_t&);
    packHdrPointer packHdr;

    // Header
    if (headersLen > MAX_C5)             // If len > 39, filter the last 39 ones
    {
        Hdrs = headers.substr(headersLen - MAX_C5);
//        Hdrs_g = Hdrs;
        // ASCII char after the last char in Hdrs -- always <= (char) 127
        HdrsX = Hdrs;    HdrsX += (char) (Hdrs.back() + 1);
        buildHashTable(HdrMap, HdrsX, KEYLEN_C5);
        packHdr=&EnDecrypto::packLargeHdr_3to2;
    }
    else
    {
        Hdrs = headers;
//        Hdrs_g = Hdrs;

        if (headersLen > MAX_C4)                            // 16 <= cat 5 <= 39
        {
            buildHashTable(HdrMap, Hdrs, KEYLEN_C5);
            packHdr = &EnDecrypto::pack_3to2;
        }
        else if (headersLen > MAX_C3)                       // 7 <= cat 4 <= 15
        {
            buildHashTable(HdrMap, Hdrs, KEYLEN_C4);
            packHdr = &EnDecrypto::pack_2to1;
        }
        else if (headersLen==MAX_C3 || headersLen==MID_C3   // 4 <= cat 3 <= 6
                 || headersLen==MIN_C3)
        {
            buildHashTable(HdrMap, Hdrs, KEYLEN_C3);
            packHdr = &EnDecrypto::pack_3to1;
        }
        else if (headersLen == C2)                          // cat 2 = 3
        {
            buildHashTable(HdrMap, Hdrs, KEYLEN_C2);
            packHdr = &EnDecrypto::pack_5to1;
        }
        else if (headersLen == C1)                          // cat 1 = 2
        {
            buildHashTable(HdrMap, Hdrs, KEYLEN_C1);
            packHdr = &EnDecrypto::pack_7to1;
        }
        else                                                // headersLen = 1
        {
            buildHashTable(HdrMap, Hdrs, 1);
            packHdr = &EnDecrypto::pack_1to1;
        }
    }

    pkStruct.packHdrFPtr = packHdr;

    // Distribute file among threads, for reading and packing
    for (t = 0; t != n_threads; ++t)
        arrThread[t] = thread(&FASTA::packFA, this, pkStruct, t);
    for (t = 0; t != n_threads; ++t)
        if (arrThread[t].joinable())    arrThread[t].join();

    if (verbose)    cerr << "Shuffling done!\n";

    // Join partially packed files
    ifstream pkFile[n_threads];

    // Watermark for encrypted file
    cout << "#cryfa v" + to_string(VERSION_CRYFA) + "."
            + to_string(RELEASE_CRYFA) + "\n";

    // Open packed file
    ofstream pckdFile(PCKD_FILENAME);
    pckdFile << (char) 127;                // Let decryptor know this is FASTA
    pckdFile << (!disable_shuffle ? (char) 128 : (char) 129); //Shuffling on/off
    pckdFile << headers;                   // Send headers to decryptor
    pckdFile << (char) 254;                // To detect headers in decompressor

    // Open input files
    for (t = 0; t != n_threads; ++t)  pkFile[t].open(PK_FILENAME+to_string(t));

    string line;
    bool prevLineNotThrID;                 // If previous line was "THR=" or not
    while (!pkFile[0].eof())
    {
        for (t = 0; t != n_threads; ++t)
        {
            prevLineNotThrID = false;

            while (getline(pkFile[t], line).good() &&
                   line != THR_ID_HDR+to_string(t))
            {
                if (prevLineNotThrID)   pckdFile << '\n';
                pckdFile << line;

                prevLineNotThrID = true;
            }
        }
    }
    pckdFile << (char) 252;

    // Stop timer for compression
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Compression duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;

    cerr << (verbose ? "Compaction done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

    // Close/delete input/output files
    pckdFile.close();
    string pkFileName;
    for (t = 0; t != n_threads; ++t)
    {
        pkFile[t].close();
        pkFileName=PK_FILENAME;    pkFileName+=to_string(t);
        std::remove(pkFileName.c_str());
    }

    // Cout encrypted content
    encrypt();
}

/**
 * @brief Pack FASTA -- '>' at the beginning of headers not packed
 * @param pkStruct  Pack structure
 * @param threadID  Thread ID
 */
void FASTA::packFA (const packfa_s& pkStruct, byte threadID)
{
    using packHdrFPtr   = void (EnDecrypto::*)
                               (string&, const string&, const htbl_t&);
    packHdrFPtr packHdr = pkStruct.packHdrFPtr;              // Function pointer
    ifstream    in(inFileName);
    string      line, context, seq;
    ofstream    pkfile(PK_FILENAME+to_string(threadID), std::ios_base::app);

    // Lines ignored at the beginning
    for (u64 l = (u64) threadID*BlockLine; l--;)    IGNORE_THIS_LINE(in);

    while (in.peek() != EOF)
    {
        context.clear();
        seq.clear();

        for (u64 l = BlockLine; l-- && getline(in, line).good();)
        {
            // Header
            if (line[0] == '>')
            {
                // Previous seq
                if (!seq.empty())
                {
                    seq.pop_back();                      // Remove the last '\n'
                    packSeq_3to1(context, seq);
                    context += (char) 254;
                }
                seq.clear();

                // Header line
                context += (char) 253;
                (this->*packHdr) (context, line.substr(1), HdrMap);
                context += (char) 254;
            }

            // Empty line. (char) 252 instead of line feed
            else if (line.empty()) { seq += (char) 252; }

            // Sequence
            else
            {
                //todo. check if it's needed to check for blank char
//                if (line.find(' ') != string::npos)
//              { cerr<< "Invalid sequence -- spaces not allowed.\n"; exit(1); }

                // (char) 252 instead of '\n' at the end of each seq line
                seq += line;
                seq += (char) 252;
            }
        }
        if (!seq.empty())
        {
            seq.pop_back();                              // Remove the last '\n'

            // The last seq
            packSeq_3to1(context, seq);
            context += (char) 254;
        }

        // Shuffle
        if (!disable_shuffle)
        {
            mutxFA.lock();//------------------------------------------------------
            if (verbose && shuffInProgress)    cerr << "Shuffling...\n";

            shuffInProgress = false;
            mutxFA.unlock();//----------------------------------------------------

            shufflePkd(context);
        }

        // For unshuffling: insert the size of packed context in the beginning
        string contextSize;
        contextSize += (char) 253;
        contextSize += to_string(context.size());
        contextSize += (char) 254;
        context.insert(0, contextSize);

        // Write header containing threadID for each
        pkfile << THR_ID_HDR << to_string(threadID) << '\n';
        pkfile << context << '\n';

        // Ignore to go to the next related chunk
        for (u64 l = (u64) (n_threads-1)*BlockLine; l--;)  IGNORE_THIS_LINE(in);
    }

    pkfile.close();
    in.close();
}

/**
 * @brief      Gather chars of all headers & max length of DNA bases lines
 *             in FASTA, excluding '>'
 * @param[out] headers  Chars of all headers
 */
void FASTA::gatherHdrBs (string &headers)
{
    u32  maxBLen=0;           // Max length of each line of bases
    bool hChars[127];
    memset(hChars+32, false, 95);

    ifstream in(inFileName);
    string   line;
    while (getline(in, line).good())
    {
        if (line[0] == '>')
            for (const char &c : line)    hChars[c] = true;
        else
        if (line.size() > maxBLen)    maxBLen = (u32) line.size();
    }
    in.close();

    // Number of lines read from input file while compression
    BlockLine = (u32) (BLOCK_SIZE / maxBLen);
    if (!BlockLine)   BlockLine = 2;

    // Gather the characters -- Ignore '>'=62 for headers
    for (byte i = 32; i != 62;  ++i)    if (*(hChars+i))  headers += i;
    for (byte i = 63; i != 127; ++i)    if (*(hChars+i))  headers += i;
}
//
///**
// * @brief Decompress FASTA
// */
//void FASTA::decompressFA ()
//{
//    // Start timer for decompression
//    high_resolution_clock::time_point startTime = high_resolution_clock::now();
//
//    char       c;                   // Chars in file
//    string     headers;
//    unpackfa_s upkStruct;           // Collection of inputs to pass to unpack...
//    string     chunkSizeStr;        // Chunk size (string) -- For unshuffling
//    thread     arrThread[n_threads];// Array of threads
//    byte       t;                   // For threads
//    u64        offset;              // To traverse decompressed file
//
//    ifstream in(DEC_FILENAME);
//    in.ignore(1);                   // Jump over decText[0]==(char) 127
//    in.get(c);    shuffled = (c==(char) 128); // Check if file had been shuffled
//    while (in.get(c) && c != (char) 254)    headers += c;
//    const size_t headersLen = headers.length();
//    u16 keyLen_hdr = 0;
//
//    // Show number of different chars in headers -- Ignore '>'=62
//    if (verbose)  cerr<< headersLen <<" different characters are in headers.\n";
//
//    // Function pointer
//    using unpackHdrPtr =
//    void (EnDecrypto::*) (string&, string::iterator&, const vector<string>&);
//    unpackHdrPtr unpackHdr;
//
//    // Header
//    if      (headersLen > MAX_C5)           keyLen_hdr = KEYLEN_C5;
//    else if (headersLen > MAX_C4)                                       // Cat 5
//    {
//        unpackHdr  = &EnDecrypto::unpack_read2B;
//        keyLen_hdr = KEYLEN_C5;
//    }
//    else
//    {
//        unpackHdr  = &EnDecrypto::unpack_read1B;
//
//        if      (headersLen > MAX_C3)       keyLen_hdr = KEYLEN_C4;     // Cat 4
//        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
//            keyLen_hdr = KEYLEN_C3;     // Cat 3
//        else if (headersLen == C2)          keyLen_hdr = KEYLEN_C2;     // Cat 2
//        else if (headersLen == C1)          keyLen_hdr = KEYLEN_C1;     // Cat 1
//        else                                keyLen_hdr = 1;             // = 1
//    }
//
//    if (headersLen <= MAX_C5)
//    {
//        // Tables for unpacking
//        buildUnpack(upkStruct.hdrUnpack, headers, keyLen_hdr);
//
//        // Distribute file among threads, for reading and unpacking
//        for (t = 0; t != n_threads; ++t)
//        {
//            in.get(c);
//            if (c == (char) 253)
//            {
//                chunkSizeStr.clear();   // Chunk size
//                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
//                offset = stoull(chunkSizeStr);
//
//                upkStruct.begPos        = in.tellg();
//                upkStruct.chunkSize     = offset;
//                upkStruct.unpackHdrFPtr = unpackHdr;
//
//                arrThread[t]= thread(&FASTA::unpackHS, this, upkStruct, t);
//
//                // Jump to the beginning of the next chunk
//                in.seekg((std::streamoff) offset, std::ios_base::cur);
//            }
//            // End of file
//            if (in.peek() == 252)    break;
//        }
//        // Join threads
//        for (t = 0; t != n_threads; ++t)
//            if (arrThread[t].joinable())    arrThread[t].join();
//
//        if (verbose)    cerr << "Unshuffling done!\n";
//    }
//    else
//    {
//        const string decHeaders = headers.substr(headersLen - MAX_C5);
//        // ASCII char after the last char in headers string
//        string decHeadersX = decHeaders;
//        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));
//
//        // Tables for unpacking
//        buildUnpack(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
//
//        // Distribute file among threads, for reading and unpacking
//        for (t = 0; t != n_threads; ++t)
//        {
//            in.get(c);
//            if (c == (char) 253)
//            {
//                chunkSizeStr.clear();   // Chunk size
//                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
//                offset = stoull(chunkSizeStr);
//
//                upkStruct.begPos        = in.tellg();
//                upkStruct.chunkSize     = offset;
//
//                arrThread[t]= thread(&FASTA::unpackHL, this, upkStruct, t);
//
//                // Jump to the beginning of the next chunk
//                in.seekg((std::streamoff) offset, std::ios_base::cur);
//            }
//            // End of file
//            if (in.peek() == 252)    break;
//        }
//        // Join threads
//        for (t = 0; t != n_threads; ++t)
//            if (arrThread[t].joinable())    arrThread[t].join();
//
//        if (verbose)    cerr << "Unshuffling done!\n";
//    }
//
//    // Close/delete decrypted file
//    in.close();
//    const string decFileName = DEC_FILENAME;
//    std::remove(decFileName.c_str());
//
//    // Join unpacked files
//    ifstream upkdFile[n_threads];
//    string line;
//    for (t = n_threads; t--;)   upkdFile[t].open(UPK_FILENAME+to_string(t));
//
//    bool prevLineNotThrID;            // If previous line was "THRD=" or not
//    while (!upkdFile[0].eof())
//    {
//        for (t = 0; t != n_threads; ++t)
//        {
//            prevLineNotThrID = false;
//
//            while (getline(upkdFile[t], line).good() &&
//                   line != THR_ID_HDR+to_string(t))
//            {
//                if (prevLineNotThrID)
//                    cout << '\n';
//                cout << line;
//
//                prevLineNotThrID = true;
//            }
//
//            if (prevLineNotThrID)    cout << '\n';
//        }
//    }
//
//    // Stop timer for decompression
//    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
//    // Decompression duration in seconds
//    std::chrono::duration<double> elapsed = finishTime - startTime;
//
//    cerr << (verbose ? "Decompression done," : "Done,") << " in "
//         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
//
//    // Close/delete input/output files
//    string upkdFileName;
//    for (t = n_threads; t--;)
//    {
//        upkdFile[t].close();
//        upkdFileName=UPK_FILENAME;    upkdFileName+=to_string(t);
//        std::remove(upkdFileName.c_str());
//    }
//}
//
///**
// * @brief Unpack FASTA: small header
// * @param upkStruct  Unpack structure
// * @param threadID   Thread ID
// */
//inline void FASTA::unpackHS (const unpackfa_s &upkStruct, byte threadID)
//{
//    using unpackHdrFPtr =
//       void (EnDecrypto::*) (string&, string::iterator&, const vector<string>&);
//    unpackHdrFPtr    unpackHdr = upkStruct.unpackHdrFPtr;    // Function pointer
//    pos_t            begPos    = upkStruct.begPos;
//    u64              chunkSize = upkStruct.chunkSize;
//    ifstream         in(DEC_FILENAME);
//    string           decText, chunkSizeStr;
//    string::iterator i;
//    char             c;
//    pos_t            endPos;
//    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
//    string upkhdrOut, upkSeqOut;
//
//    while (in.peek() != EOF)
//    {
//        in.seekg(begPos);      // Read the file from this position
//        // Take a chunk of decrypted file
//        decText.clear();
//        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
//        i = decText.begin();
//        endPos = in.tellg();   // Set the end position
//
//        // Unshuffle
//        if (shuffled)
//        {
//            mutxFA.lock();//------------------------------------------------------
//            if (verbose && shuffInProgress)    cerr << "Unshuffling...\n";
//
//            shuffInProgress = false;
//            mutxFA.unlock();//----------------------------------------------------
//
//            unshufflePkd(i, chunkSize);
//        }
//
//        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
//        do {
//            if (*i == (char) 253)                                         // Hdr
//            {
//                (this->*unpackHdr) (upkhdrOut, ++i, upkStruct.hdrUnpack);
//                upkfile << '>' << upkhdrOut << '\n';
//            }
//            else                                                          // Seq
//            {
//                unpackSeqFA_3to1(upkSeqOut, i);
//                upkfile << upkSeqOut << '\n';
//            }
//        } while (++i != decText.end());        // If trouble: change "!=" to "<"
//
//        // Update the chunk size and positions (beg & end)
//        for (byte t = n_threads; t--;)
//        {
//            in.seekg(endPos);
//            in.get(c);
//            if (c == (char) 253)
//            {
//                chunkSizeStr.clear();
//                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
//
//                chunkSize = stoull(chunkSizeStr);
//                begPos    = in.tellg();
//                endPos    = begPos + (pos_t) chunkSize;
//            }
//        }
//    }
//
//    upkfile.close();
//    in.close();
//}
//
///**
// * @brief Unpack FASTA: large header
// * @param upkStruct  Unpack structure
// * @param threadID   Thread ID
// */
//inline void FASTA::unpackHL (const unpackfa_s &upkStruct, byte threadID)
//{
//    pos_t            begPos    = upkStruct.begPos;
//    u64              chunkSize = upkStruct.chunkSize;
//    ifstream         in(DEC_FILENAME);
//    string           decText, chunkSizeStr;
//    string::iterator i;
//    char             c;
//    pos_t            endPos;
//    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
//    string upkHdrOut, upkSeqOut;
//
//    while (in.peek() != EOF)
//    {
//        in.seekg(begPos);      // Read the file from this position
//        // Take a chunk of decrypted file
//        decText.clear();
//        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
//        i = decText.begin();
//        endPos = in.tellg();   // Set the end position
//
//        // Unshuffle
//        if (shuffled)
//        {
//            mutxFA.lock();//------------------------------------------------------
//            if (verbose && shuffInProgress)    cerr << "Unshuffling...\n";
//
//            shuffInProgress = false;
//            mutxFA.unlock();//----------------------------------------------------
//
//            unshufflePkd(i, chunkSize);
//        }
//
//        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
//        do {
//            if (*i == (char) 253)                                         // Hdr
//            {
//                unpackLarge_read2B(upkHdrOut, ++i,
//                                   upkStruct.XChar_hdr, upkStruct.hdrUnpack);
//                upkfile << '>' << upkHdrOut << '\n';
//            }
//            else                                                          // Seq
//            {
//                unpackSeqFA_3to1(upkSeqOut, i);
//                upkfile << upkSeqOut << '\n';
//            }
//        } while (++i != decText.end());        // If trouble: change "!=" to "<"
//
//        // Update the chunk size and positions (beg & end)
//        for (byte t = n_threads; t--;)
//        {
//            in.seekg(endPos);
//            in.get(c);
//            if (c == (char) 253)
//            {
//                chunkSizeStr.clear();
//                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
//
//                chunkSize = stoull(chunkSizeStr);
//                begPos    = in.tellg();
//                endPos    = begPos + (pos_t) chunkSize;
//            }
//        }
//    }
//
//    upkfile.close();
//    in.close();
//}
//
///**
// * @brief      Unpack 1 byte to 3 DNA bases -- FASTA
// * @param[out] out  DNA bases
// * @param[in]  i    Input string iterator
// */
//inline void FASTA::unpackSeqFA_3to1 (string &out, string::iterator &i)
//{
//    string tpl;    tpl.reserve(3);     // Tuplet
//    out.clear();
//    byte s;
//
//    for (; *i != (char) 254; ++i)
//    {
//        s = (byte) *i;
//
//        if (s == 255) { out += penaltySym(*(++i)); }
//        else
//        {
//            tpl = DNA_UNPACK[s];
//
//            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')                // ...
//            { out+=tpl;                                                        }
//                // Using just one 'out' makes trouble
//            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')           // X..
//            { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2];         }
//
//            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')           // .X.
//            { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2];         }
//
//            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')           // XX.
//            { out+=penaltySym(*(++i));  out+=penaltySym(*(++i));  out+=tpl[2]; }
//
//            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')           // ..X
//            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i));         }
//
//            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')           // X.X
//            { out+=penaltySym(*(++i));  out+=tpl[1];  out+=penaltySym(*(++i)); }
//
//            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')           // .XX
//            { out+=tpl[0];  out+=penaltySym(*(++i));  out+=penaltySym(*(++i)); }
//
//            else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));   // XXX
//                out+=penaltySym(*(++i));                                    }
//        }
//    }
//}