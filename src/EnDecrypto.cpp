/**
 * @file      EnDecrypto.cpp
 * @brief     Encryption / Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include "EnDecrypto.h"
//#include "pack.h"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"

using std::vector;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::to_string;
using std::thread;
using std::stoull;
using std::chrono::high_resolution_clock;
using std::setprecision;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::CBC_Mode;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::FileSource;
using CryptoPP::FileSink;

std::mutex mutx;    /**< @brief Mutex */


/**
 * @brief Compress FASTA
 */
void EnDecrypto::compressFA ()
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
    using packHdrPointer = void (*) (string&, const string&, const htbl_t&);
    packHdrPointer packHdr;
    
    // Header
    if (headersLen > MAX_C5)             // If len > 39, filter the last 39 ones
    {
        Hdrs = headers.substr(headersLen - MAX_C5);
//        Hdrs_g = Hdrs;
        // ASCII char after the last char in Hdrs -- always <= (char) 127
        HdrsX = Hdrs;    HdrsX += (char) (Hdrs.back() + 1);
        buildHashTable(HdrMap, HdrsX, KEYLEN_C5);    packHdr=&packLargeHdr_3to2;
    }
    else
    {
        Hdrs = headers;
//        Hdrs_g = Hdrs;
        
        if (headersLen > MAX_C4)                            // 16 <= cat 5 <= 39
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C5);    packHdr = &pack_3to2; }

        else if (headersLen > MAX_C3)                       // 7 <= cat 4 <= 15
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C4);    packHdr = &pack_2to1; }
                                                            // 4 <= cat 3 <= 6
        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C3);    packHdr = &pack_3to1; }

        else if (headersLen == C2)                          // cat 2 = 3
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C2);    packHdr = &pack_5to1; }

        else if (headersLen == C1)                          // cat 1 = 2
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C1);    packHdr = &pack_7to1; }

        else                                                // headersLen = 1
        { buildHashTable(HdrMap, Hdrs, 1);            packHdr = &pack_1to1; }
    }
    
    pkStruct.packHdrFPtr = packHdr;
    
    // Distribute file among threads, for reading and packing
    for (t = 0; t != n_threads; ++t)
        arrThread[t] = thread(&EnDecrypto::packFA, this, pkStruct, t);
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
inline void EnDecrypto::packFA (const packfa_s& pkStruct, byte threadID)
{
    using packHdrFPtr   = void (*) (string&, const string&, const htbl_t&);
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
                packHdr(context, line.substr(1), HdrMap);
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
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Shuffling...\n";
            
            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------
            
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
 * @brief Compress FASTQ
 */
void EnDecrypto::compressFQ ()
{
    // Start timer for compression
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    string   line;
    thread   arrThread[n_threads];
    byte     t;                   // For threads
    string   headers, qscores;
    packfq_s pkStruct;            // Collection of inputs to pass to pack...
    
    if (verbose)    cerr << "Calculating number of different characters...\n";
    
    // Gather different chars and max length in all headers and quality scores
    gatherHdrQs(headers, qscores);
    
    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();
    
    // Show number of different chars in headers and qs -- Ignore '@'=64 in hdr
    if (verbose)
        cerr << "In headers, they are " << headersLen << ".\n"
             << "In quality scores, they are " << qscoresLen << ".\n";
    
    // Function pointers
    using packHdrPointer = void (*) (string&, const string&, const htbl_t&);
    packHdrPointer packHdr;
    using packQSPointer  = void (*) (string&, const string&, const htbl_t&);
    packQSPointer  packQS;

    // Header
    if (headersLen > MAX_C5)          // If len > 39 filter the last 39 ones
    {
        Hdrs = headers.substr(headersLen - MAX_C5);
//        Hdrs_g = Hdrs;
        // ASCII char after the last char in Hdrs -- Always <= (char) 127
        HdrsX = Hdrs;    HdrsX += (char) (Hdrs.back() + 1);
        buildHashTable(HdrMap, HdrsX, KEYLEN_C5);    packHdr=&packLargeHdr_3to2;
    }
    else
    {
        Hdrs = headers;
//        Hdrs_g = Hdrs;

        if (headersLen > MAX_C4)                            // 16 <= cat 5 <= 39
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C5);    packHdr = &pack_3to2; }

        else if (headersLen > MAX_C3)                       // 7 <= cat 4 <= 15
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C4);    packHdr = &pack_2to1; }
                                                            // 4 <= cat 3 <= 6
        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C3);    packHdr = &pack_3to1; }

        else if (headersLen == C2)                          // cat 2 = 3
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C2);    packHdr = &pack_5to1; }

        else if (headersLen == C1)                          // cat 1 = 2
        { buildHashTable(HdrMap, Hdrs, KEYLEN_C1);    packHdr = &pack_7to1; }

        else                                                // headersLen = 1
        { buildHashTable(HdrMap, Hdrs, 1);            packHdr = &pack_1to1; }
    }

    // Quality score
    if (qscoresLen > MAX_C5)              // If len > 39 filter the last 39 ones
    {
        QSs = qscores.substr(qscoresLen - MAX_C5);
//        QSs_g = QSs;
        // ASCII char after last char in QUALITY_SCORES
        QSsX = QSs;     QSsX += (char) (QSs.back() + 1);
        buildHashTable(QsMap, QSsX, KEYLEN_C5);     packQS = &packLargeQs_3to2;
    }
    else
    {
        QSs = qscores;
//        QSs_g = QSs;

        if (qscoresLen > MAX_C4)                            // 16 <= cat 5 <= 39
        { buildHashTable(QsMap, QSs, KEYLEN_C5);    packQS = &pack_3to2; }

        else if (qscoresLen > MAX_C3)                       // 7 <= cat 4 <= 15
        { buildHashTable(QsMap, QSs, KEYLEN_C4);    packQS = &pack_2to1; }
                                                            // 4 <= cat 3 <= 6
        else if (qscoresLen==MAX_C3 || qscoresLen==MID_C3 || qscoresLen==MIN_C3)
        { buildHashTable(QsMap, QSs, KEYLEN_C3);    packQS = &pack_3to1; }

        else if (qscoresLen == C2)                          // cat 2 = 3
        { buildHashTable(QsMap, QSs, KEYLEN_C2);    packQS = &pack_5to1; }

        else if (qscoresLen == C1)                          // cat 1 = 2
        { buildHashTable(QsMap, QSs, KEYLEN_C1);    packQS = &pack_7to1; }

        else                                                // qscoresLen = 1
        { buildHashTable(QsMap, QSs, 1);            packQS = &pack_1to1; }
    }

    pkStruct.packHdrFPtr = packHdr;
    pkStruct.packQSFPtr  = packQS;

    // Distribute file among threads, for reading and packing
    for (t = 0; t != n_threads; ++t)
        arrThread[t] = thread(&EnDecrypto::packFQ, this, pkStruct, t);
    for (t = 0; t != n_threads; ++t)
        if (arrThread[t].joinable())    arrThread[t].join();
    
    if (verbose)    cerr << "Shuffling done!\n";
    
    // Join partially packed files
    ifstream pkFile[n_threads];
    string context;

    // Watermark for encrypted file
    cout << "#cryfa v" + to_string(VERSION_CRYFA) + "."
                       + to_string(RELEASE_CRYFA) + "\n";

    // Open packed file
    ofstream pckdFile(PCKD_FILENAME);
    pckdFile << (!disable_shuffle ? (char) 128 : (char) 129); //Shuffling on/off
    pckdFile << headers;                            // Send headers to decryptor
    pckdFile << (char) 254;                         // To detect headers in dec.
    pckdFile << qscores;                            // Send qscores to decryptor
    pckdFile << (hasFQjustPlus() ? (char) 253 : '\n');        // If just '+'

    // Open input files
    for (t = 0; t != n_threads; ++t)  pkFile[t].open(PK_FILENAME+to_string(t));

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

    // Close/delete input/output files
    pckdFile.close();
    string pkFileName;
    for (t = 0; t != n_threads; ++t)
    {
        pkFile[t].close();
        pkFileName=PK_FILENAME;    pkFileName+=to_string(t);
        std::remove(pkFileName.c_str());
    }
    
    // Stop timer for compression
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Compression duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    
    cerr << (verbose ? "Compaction done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    // Cout encrypted content
    encrypt();
    
    
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

/**
 * @brief Pack FASTQ -- '@' at the beginning of headers is not packed
 * @param pkStruct  Pack structure
 * @param threadID  Thread ID
 */
inline void EnDecrypto::packFQ (const packfq_s& pkStruct, byte threadID)
{
    // Function pointers
    using packHdrFPtr   = void (*) (string&, const string&, const htbl_t&);
    packHdrFPtr packHdr = pkStruct.packHdrFPtr;
    using packQSPtr     = void (*) (string&, const string&, const htbl_t&);
    packQSPtr   packQS  = pkStruct.packQSFPtr;

    ifstream in(inFileName);
    string   context;       // Output string
    string   line;
    ofstream pkfile(PK_FILENAME+to_string(threadID), std::ios_base::app);

    // Lines ignored at the beginning
    for (u64 l = (u64) threadID*BlockLine; l--;)    IGNORE_THIS_LINE(in);

    while (in.peek() != EOF)
    {
        context.clear();

        for (u64 l = 0; l != BlockLine; l += 4)  // Process 4 lines by 4 lines
        {
            if (getline(in, line).good())          // Header -- Ignore '@'
            { packHdr(context, line.substr(1), HdrMap);   context+=(char) 254; }

            if (getline(in, line).good())          // Sequence
            { packSeq_3to1(context, line);                context+=(char) 254; }
    
            IGNORE_THIS_LINE(in);                  // +. ignore

            if (getline(in, line).good())          // Quality score
            { packQS(context, line, QsMap);               context+=(char) 254; }
        }

        // shuffle
        if (!disable_shuffle)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Shuffling...\n";
    
            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------
    
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
 * @brief   Encrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
inline void EnDecrypto::encrypt ()
{
    cerr << "Encrypting...\n";
    
    // Start timer for encryption
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // Debug
//    printKey(key);    // Debug
    
    // Encrypt
    const char* inFile = PCKD_FILENAME;
    CBC_Mode<CryptoPP::AES>::Encryption
            cbcEnc(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(inFile, true,
               new StreamTransformationFilter(cbcEnc, new FileSink(cout)));
    
    // Stop timer for encryption
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Encryption duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    
    cerr << (verbose ? "Encryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    // Delete packed file
    const string pkdFileName = PCKD_FILENAME;
    std::remove(pkdFileName.c_str());
    
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

/**
 * @brief   Decrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void EnDecrypto::decrypt ()
{
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening \"" << inFileName << "\".\n";    exit(1); }
    
    // Watermark
    string watermark = "#cryfa v";
    watermark += to_string(VERSION_CRYFA);    watermark += ".";
    watermark += to_string(RELEASE_CRYFA);    watermark += "\n";
    
    // Invalid encrypted file
    string line;    getline(in, line);
    if ((line + "\n") != watermark)
    {
        cerr << "Error: \"" << inFileName << '"'
             << " is not a valid file encrypted by cryfa.\n";
        exit(1);
    }
    
////    string::size_type watermarkIdx = cipherText.find(watermark);
////    if (watermarkIdx == string::npos)
////    { cerr << "Error: invalid encrypted file!\n";    exit(1); }
////    else  cipherText.erase(watermarkIdx, watermark.length());
    
    cerr << "Decrypting...\n";
    
    // Start timer for decryption
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // Debug
//    printKey(key);    // Debug

//    string cipherText( (std::istreambuf_iterator<char> (in)),
//                       std::istreambuf_iterator<char> () );

//    if (verbose)
//    {
//        cerr << "cipher size: " << cipherText.size()-1 << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
//    }
    
    const char* outFile = DEC_FILENAME;
    CBC_Mode<CryptoPP::AES>::Decryption
            cbcDec(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(in, true,
               new StreamTransformationFilter(cbcDec, new FileSink(outFile)));
    
    // Stop timer for decryption
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Decryption duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    
    cerr << (verbose ? "Decryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    in.close();
}

/**
 * @brief Decompress FASTA
 */
void EnDecrypto::decompressFA ()
{
    // Start timer for decompression
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    char       c;                   // Chars in file
    string     headers;
    unpackfa_s upkStruct;           // Collection of inputs to pass to unpack...
    string     chunkSizeStr;        // Chunk size (string) -- For unshuffling
    thread     arrThread[n_threads];// Array of threads
    byte       t;                   // For threads
    u64        offset;              // To traverse decompressed file
    
    ifstream in(DEC_FILENAME);
    in.ignore(1);                   // Jump over decText[0]==(char) 127
    in.get(c);    shuffled = (c==(char) 128); // Check if file had been shuffled
    while (in.get(c) && c != (char) 254)    headers += c;
    const size_t headersLen = headers.length();
    u16 keyLen_hdr = 0;
    
    // Show number of different chars in headers -- Ignore '>'=62
    if (verbose)  cerr<< headersLen <<" different characters are in headers.\n";
    
    // Function pointer
    using unpackHdrPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackHdrPtr unpackHdr;
    
    // Header
    if      (headersLen > MAX_C5)           keyLen_hdr = KEYLEN_C5;
    else if (headersLen > MAX_C4)                                       // Cat 5
    {   unpackHdr = &unpack_read2B;         keyLen_hdr = KEYLEN_C5; }
    else
    {   unpackHdr = &unpack_read1B;

        if      (headersLen > MAX_C3)       keyLen_hdr = KEYLEN_C4;     // Cat 4
        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
                                            keyLen_hdr = KEYLEN_C3;     // Cat 3
        else if (headersLen == C2)          keyLen_hdr = KEYLEN_C2;     // Cat 2
        else if (headersLen == C1)          keyLen_hdr = KEYLEN_C1;     // Cat 1
        else                                keyLen_hdr = 1;             // = 1
    }

    if (headersLen <= MAX_C5)
    {
        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, headers, keyLen_hdr);

        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos        = in.tellg();
                upkStruct.chunkSize     = offset;
                upkStruct.unpackHdrFPtr = unpackHdr;

                arrThread[t]= thread(&EnDecrypto::unpackHS, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }
    else
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        // ASCII char after the last char in headers string
        string decHeadersX = decHeaders;
        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));
        
        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
        
        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos        = in.tellg();
                upkStruct.chunkSize     = offset;

                arrThread[t]= thread(&EnDecrypto::unpackHL, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }
    
    // Close/delete decrypted file
    in.close();
    const string decFileName = DEC_FILENAME;
    std::remove(decFileName.c_str());

    // Join unpacked files
    ifstream upkdFile[n_threads];
    string line;
    for (t = n_threads; t--;)   upkdFile[t].open(UPK_FILENAME+to_string(t));

    bool prevLineNotThrID;            // If previous line was "THRD=" or not
    while (!upkdFile[0].eof())
    {
        for (t = 0; t != n_threads; ++t)
        {
            prevLineNotThrID = false;

            while (getline(upkdFile[t], line).good() &&
                   line != THR_ID_HDR+to_string(t))
            {
                if (prevLineNotThrID)
                    cout << '\n';
                cout << line;

                prevLineNotThrID = true;
            }

            if (prevLineNotThrID)    cout << '\n';
        }
    }

    // Stop timer for decompression
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Decompression duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;

    cerr << (verbose ? "Decompression done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

    // Close/delete input/output files
    string upkdFileName;
    for (t = n_threads; t--;)
    {
        upkdFile[t].close();
        upkdFileName=UPK_FILENAME;    upkdFileName+=to_string(t);
        std::remove(upkdFileName.c_str());
    }
}

/**
 * @brief Unpack FASTA: small header
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHS (const unpackfa_s &upkStruct, byte threadID)
{
    using unpackHdrFPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackHdrFPtr    unpackHdr = upkStruct.unpackHdrFPtr;    // Function pointer
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string upkhdrOut, upkSeqOut;

    while (in.peek() != EOF)
    {
        in.seekg(begPos);      // Read the file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();   // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            if (*i == (char) 253)                                         // Hdr
            {
                unpackHdr(upkhdrOut, ++i, upkStruct.hdrUnpack);
                upkfile << '>' << upkhdrOut << '\n';
            }
            else                                                          // Seq
            {
                unpackSeqFA_3to1(upkSeqOut, i);
                upkfile << upkSeqOut << '\n';
            }
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief Unpack FASTA: large header
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHL (const unpackfa_s &upkStruct, byte threadID)
{
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string upkHdrOut, upkSeqOut;
    
    while (in.peek() != EOF)
    {
        in.seekg(begPos);      // Read the file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();   // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            if (*i == (char) 253)                                         // Hdr
            {
                unpackLarge_read2B(upkHdrOut, ++i,
                                   upkStruct.XChar_hdr, upkStruct.hdrUnpack);
                upkfile << '>' << upkHdrOut << '\n';
            }
            else                                                          // Seq
            {
                unpackSeqFA_3to1(upkSeqOut, i);
                upkfile << upkSeqOut << '\n';
            }
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief Decompress FASTQ
 */
void EnDecrypto::decompressFQ ()
{
    // Start timer for decompression
    high_resolution_clock::time_point startTime = high_resolution_clock::now();

    char       c;                   // Chars in file
    string     headers, qscores;
    unpackfq_s upkStruct;           // Collection of inputs to pass to unpack...
    string     chunkSizeStr;        // Chunk size (string) -- For unshuffling
    thread     arrThread[n_threads];// Array of threads
    byte       t;                   // For threads
    u64        offset;              // To traverse decompressed file

    ifstream in(DEC_FILENAME);
    in.get(c);    shuffled = (c==(char) 128); // Check if file had been shuffled
    while (in.get(c) && c != (char) 254)                 headers += c;
    while (in.get(c) && c != '\n' && c != (char) 253)    qscores += c;
    if (c == '\n')    justPlus = false;                 // If 3rd line is just +

    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();
    u16 keyLen_hdr = 0,  keyLen_qs = 0;

    // Show number of different chars in headers and qs -- ignore '@'=64
    if (verbose)
        cerr << headersLen << " different characters are in headers.\n"
             << qscoresLen << " different characters are in quality scores.\n";

    using unpackHdrPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackHdrPtr unpackHdr;                                  // Function pointer
    using unpackQSPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackQSPtr unpackQS;                                    // Function pointer

    // Header
    if      (headersLen > MAX_C5)           keyLen_hdr = KEYLEN_C5;
    else if (headersLen > MAX_C4)                                       // Cat 5
    {   unpackHdr = &unpack_read2B;         keyLen_hdr = KEYLEN_C5; }
    else
    {   unpackHdr = &unpack_read1B;

        if      (headersLen > MAX_C3)       keyLen_hdr = KEYLEN_C4;     // Cat 4
        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
                                            keyLen_hdr = KEYLEN_C3;     // Cat 3
        else if (headersLen == C2)          keyLen_hdr = KEYLEN_C2;     // Cat 2
        else if (headersLen == C1)          keyLen_hdr = KEYLEN_C1;     // Cat 1
        else                                keyLen_hdr = 1;             // = 1
    }

    // Quality score
    if          (qscoresLen > MAX_C5)       keyLen_qs = KEYLEN_C5;
    else if     (qscoresLen > MAX_C4)                                   // Cat 5
    {   unpackQS = &unpack_read2B;          keyLen_qs = KEYLEN_C5; }
    else
    {   unpackQS = &unpack_read1B;

        if      (qscoresLen > MAX_C3)       keyLen_qs = KEYLEN_C4;      // Cat 4
        else if (qscoresLen==MAX_C3 || qscoresLen==MID_C3 || qscoresLen==MIN_C3)
                                            keyLen_qs = KEYLEN_C3;      // Cat 3
        else if (qscoresLen == C2)          keyLen_qs = KEYLEN_C2;      // Cat 2
        else if (qscoresLen == C1)          keyLen_qs = KEYLEN_C1;      // Cat 1
        else                                keyLen_qs = 1;              // = 1
    }

    string plusMore;
    if (headersLen <= MAX_C5 && qscoresLen <= MAX_C5)
    {
        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, headers, keyLen_hdr);
        buildUnpack(upkStruct.qsUnpack,  qscores, keyLen_qs);

        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos        = in.tellg();
                upkStruct.chunkSize     = offset;
                upkStruct.unpackHdrFPtr = unpackHdr;
                upkStruct.unpackQSFPtr  = unpackQS;

                arrThread[t] =
                        thread(&EnDecrypto::unpackHSQS, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }
    else if (headersLen <= MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decQscores = qscores.substr(qscoresLen - MAX_C5);
        // ASCII char after the last char in decQscores string
        string decQscoresX  = decQscores;
        decQscoresX += (upkStruct.XChar_qs = (char) (decQscores.back() + 1));

        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, headers,     keyLen_hdr);
        buildUnpack(upkStruct.qsUnpack,  decQscoresX, keyLen_qs);

        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos        = in.tellg();
                upkStruct.chunkSize     = offset;
                upkStruct.unpackHdrFPtr = unpackHdr;

                arrThread[t] =
                        thread(&EnDecrypto::unpackHSQL, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }
    else if (headersLen > MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        const string decQscores = qscores.substr(qscoresLen-MAX_C5);
        // ASCII char after the last char in headers & quality_scores string
        string decHeadersX = decHeaders;
        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));
        string decQscoresX = decQscores;
        decQscoresX += (upkStruct.XChar_qs  = (char) (decQscores.back() + 1));

        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
        buildUnpack(upkStruct.qsUnpack,  decQscoresX, keyLen_qs);

        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos    = in.tellg();
                upkStruct.chunkSize = offset;

                arrThread[t] =
                        thread(&EnDecrypto::unpackHLQL, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }
    else if (headersLen > MAX_C5 && qscoresLen <= MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        // ASCII char after the last char in headers string
        string decHeadersX = decHeaders;
        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));

        // Tables for unpacking
        buildUnpack(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
        buildUnpack(upkStruct.qsUnpack,  qscores,     keyLen_qs);

        // Distribute file among threads, for reading and unpacking
        for (t = 0; t != n_threads; ++t)
        {
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();   // Chunk size
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
                offset = stoull(chunkSizeStr);

                upkStruct.begPos       = in.tellg();
                upkStruct.chunkSize    = offset;
                upkStruct.unpackQSFPtr = unpackQS;

                arrThread[t] =
                        thread(&EnDecrypto::unpackHLQS, this, upkStruct, t);

                // Jump to the beginning of the next chunk
                in.seekg((std::streamoff) offset, std::ios_base::cur);
            }
            // End of file
            if (in.peek() == 252)    break;
        }
        // Join threads
        for (t = 0; t != n_threads; ++t)
            if (arrThread[t].joinable())    arrThread[t].join();

        if (verbose)    cerr << "Unshuffling done!\n";
    }

    // Close/delete decrypted file
    in.close();
    const string decFileName = DEC_FILENAME;
    std::remove(decFileName.c_str());

    // Join unpacked files
    ifstream upkdFile[n_threads];
    string line;
    for (t = n_threads; t--;)   upkdFile[t].open(UPK_FILENAME+to_string(t));

    bool prevLineNotThrID;            // If previous line was "THRD=" or not
    while (!upkdFile[0].eof())
    {
        for (t = 0; t != n_threads; ++t)
        {
            prevLineNotThrID = false;

            while (getline(upkdFile[t], line).good() &&
                   line != THR_ID_HDR+to_string(t))
            {
                if (prevLineNotThrID)
                    cout << '\n';
                cout << line;

                prevLineNotThrID = true;
            }

            if (prevLineNotThrID)    cout << '\n';
        }
    }

    // Stop timer for decompression
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Decompression duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;

    cerr << (verbose ? "Decompression done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

    // Close/delete input/output files
    string upkdFileName;
    for (t = n_threads; t--;)
    {
        upkdFile[t].close();
        upkdFileName=UPK_FILENAME;    upkdFileName+=to_string(t);
        std::remove(upkdFileName.c_str());
    }
}

/**
 * @brief Unpack FQ: small header, small quality score
 *        -- '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHSQS (const unpackfq_s &upkStruct, byte threadID)
{
    // Function pointers
    using unpackHdrFPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackHdrFPtr    unpackHdr = upkStruct.unpackHdrFPtr;
    using unpackQSFPtr  =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackQSFPtr     unpackQS  = upkStruct.unpackQSFPtr;
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, plusMore, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string upkHdrOut, upkSeqOut, upkQsOut;

    while (in.peek() != EOF)
    {
        in.seekg(begPos);      // Read the file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();   // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackHdr(upkHdrOut, i, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeqFQ_3to1(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            unpackQS(upkQsOut, i, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief Unpack FQ: small header, large quality score
 *        -- '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHSQL (const unpackfq_s &upkStruct, byte threadID)
{
    using unpackHdrFPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackHdrFPtr    unpackHdr = upkStruct.unpackHdrFPtr;    // Function pointer
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, plusMore, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME + to_string(threadID), std::ios_base::app);
    string upkHdrOut, upkSeqOut, upkQsOut;

    while (in.peek() != EOF)
    {
        in.seekg(begPos);       // Read file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();    // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackHdr(upkHdrOut, i, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';               ++i; // Hdr

            unpackSeqFQ_3to1(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';    ++i; // +

            unpackLarge_read2B(upkQsOut, i,
                               upkStruct.XChar_qs, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief Unpack FQ: large header, small quality score
 *        -- '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHLQS (const unpackfq_s &upkStruct, byte threadID)
{
    using unpackQSFPtr =
                   void (*) (string&, string::iterator&, const vector<string>&);
    unpackQSFPtr     unpackQS = upkStruct.unpackQSFPtr;      // Function pointer
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, plusMore, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME + to_string(threadID), std::ios_base::app);
    string upkHdrOut, upkSeqOut, upkQsOut;

    while (in.peek() != EOF)
    {
        in.seekg(begPos);       // Read file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();    // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackLarge_read2B(upkHdrOut, i,
                               upkStruct.XChar_hdr, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeqFQ_3to1(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            unpackQS(upkQsOut, i, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief Unpack FQ: large header, large quality score
 *        -- '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
inline void EnDecrypto::unpackHLQL (const unpackfq_s &upkStruct, byte threadID)
{
    pos_t            begPos    = upkStruct.begPos;
    u64              chunkSize = upkStruct.chunkSize;
    ifstream         in(DEC_FILENAME);
    string           decText, plusMore, chunkSizeStr;
    string::iterator i;
    char             c;
    pos_t            endPos;
    ofstream upkfile(UPK_FILENAME + to_string(threadID), std::ios_base::app);
    string upkHdrOut, upkSeqOut, upkQsOut;

    while (in.peek() != EOF)
    {
        in.seekg(begPos);       // Read file from this position
        // Take a chunk of decrypted file
        decText.clear();
        for (u64 u = chunkSize; u--;) { in.get(c);    decText += c; }
        i = decText.begin();
        endPos = in.tellg();    // Set the end position

        // Unshuffle
        if (shuffled)
        {
            mutx.lock();//------------------------------------------------------
            if (verbose && shufflingInProgress)    cerr << "Unshuffling...\n";

            shufflingInProgress = false;
            mutx.unlock();//----------------------------------------------------

            unshufflePkd(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackLarge_read2B(upkHdrOut, i,
                               upkStruct.XChar_hdr, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeqFQ_3to1(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            unpackLarge_read2B(upkQsOut, i,
                               upkStruct.XChar_qs, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = n_threads; t--;)
        {
            in.seekg(endPos);
            in.get(c);
            if (c == (char) 253)
            {
                chunkSizeStr.clear();
                while (in.get(c) && c != (char) 254)    chunkSizeStr += c;

                chunkSize = stoull(chunkSizeStr);
                begPos    = in.tellg();
                endPos    = begPos + (pos_t) chunkSize;
            }
        }
    }

    upkfile.close();
    in.close();
}

/**
 * @brief  Check if the third line of FASTQ file contains only +
 * @return True or false
 */
inline bool EnDecrypto::hasFQjustPlus () const
{
    ifstream in(inFileName);
    string   line;
    
    IGNORE_THIS_LINE(in);    // Ignore header
    IGNORE_THIS_LINE(in);    // Ignore seq
    bool justPlus = !(getline(in, line).good() && line.length() > 1);
    
    in.close();
    return justPlus;
    
    /* If input was string, instead of file
    // check if the third line contains only +
    bool justPlus = true;
    string::const_iterator lFFirst = std::find(in.begin(), in.end(), '\n');
    string::const_iterator lFSecond = std::find(lFFirst+1, in.end(), '\n');
    if (*(lFSecond+2) != '\n')  justPlus = false;   // check symbol after +
    */
}

/**
 * @brief      Gather chars of all headers & max length of DNA bases lines
 *             in FASTA, excluding '>'
 * @param[out] headers  Chars of all headers
 */
inline void EnDecrypto::gatherHdrBs (string &headers)
{
    u32  maxBLen=0;           // Max length of each line of bases
    bool hChars[127];
    std::memset(hChars+32, false, 95);
    
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

/**
 * @brief      Gather chars of all headers & quality scores
 *             in FASTQ, excluding '@' in headers
 * @param[out] headers  Chars of all headers
 * @param[out] qscores  Chars of all quality scores
 */
inline void EnDecrypto::gatherHdrQs (string& headers, string& qscores)
{
    u32  maxHLen=0,   maxQLen=0;       // Max length of headers & quality scores
    bool hChars[127], qChars[127];
    std::memset(hChars+32, false, 95);
    std::memset(qChars+32, false, 95);
    
    ifstream in(inFileName);
    string line;
    while (!in.eof())
    {
        if (getline(in, line).good())
        {
            for (const char &c : line)    hChars[c] = true;
            if (line.size() > maxHLen)    maxHLen = (u32) line.size();
        }
    
        IGNORE_THIS_LINE(in);    // Ignore sequence
        IGNORE_THIS_LINE(in);    // Ignore +
        
        if (getline(in, line).good())
        {
            for (const char &c : line)    qChars[c] = true;
            if (line.size() > maxQLen)    maxQLen = (u32) line.size();
        }
    }
    in.close();
    
    // Number of lines read from input file while compression
    BlockLine = (u32) (4 * (BLOCK_SIZE / (maxHLen + 2*maxQLen)));
    if (!BlockLine)   BlockLine = 4;
    
    // Gather the characters -- ignore '@'=64 for headers
    for (byte i = 32; i != 64;  ++i)    if (*(hChars+i))  headers += i;
    for (byte i = 65; i != 127; ++i)    if (*(hChars+i))  headers += i;
    for (byte i = 32; i != 127; ++i)    if (*(qChars+i))  qscores += i;
    
    
    /* IDEA -- Slower
    u32 hL=0, qL=0;
    u64 hH=0, qH=0;
    string headers, qscores;
    ifstream in(inFileName);
    string line;
    while (!in.eof())
    {
        if (getline(in, line).good())
            for (const char &c : line)
                (c & 0xC0) ? (hH |= 1ULL<<(c-64)) : (hL |= 1U<<(c-32));
        IGNORE_THIS_LINE(in);    // ignore sequence
        IGNORE_THIS_LINE(in);    // ignore +
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
}













//todo added
/**
 * @brief      Build a hash table
 * @param[out] map     Hash table
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
inline void EnDecrypto::buildHashTable (htbl_t &map, const string &strIn, short keyLen)
{
    u64 elementNo = 0;
    string element;    element.reserve(keyLen);
    map.clear();
    map.reserve((u64) std::pow(strIn.size(), keyLen));
    
    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                map.insert(make_pair(element, elementNo++));
////            map.insert({element, elementNo++});
////            map[element] = elementNo++;
            }
            break;
    
        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        default: break;
    }
    
    
    // Test
//    for (htbl_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << "elementNo = " << elementNo << '\n';
}

/**
 * @brief      Build a table for unpacking
 * @param[out] unpack  Table (vector of strings)
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
inline void EnDecrypto::buildUnpack(vector<string> &unpack, const string &strIn, u16 keyLen)
{
    u64 elementNo = 0;
    string element;    element.reserve(keyLen);
    unpack.clear();
    unpack.reserve((u64) std::pow(strIn.size(), keyLen));
    
    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                unpack.push_back(element);
            }
            break;

        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                unpack.push_back(element);
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                unpack.push_back(element);
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                unpack.push_back(element);
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                unpack.push_back(element);
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                unpack.push_back(element);
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                unpack.push_back(element);
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                unpack.push_back(element);
            }
            break;
            
        default: break;
    }
    
    
//    // Test
//    u64 arrSize = (u64) std::pow(strIn.size(), keyLen);
//    for (u64 i = 0; i != arrSize; ++i)
//        cerr << unpack[i] << '\n';
}

/**
 * @brief  Index of each DNA bases pack
 * @param  key  Key
 * @return Value (based on the idea of key-value in a hash table)
 */
inline byte EnDecrypto::dnaPack (const string &key)     // Maybe byte <-> u16 replacement
{
    htbl_t::const_iterator got = DNA_MAP.find(key);
    if (got == DNA_MAP.end())
    { cerr << "Error: key '" << key << "'not found!\n";    return 0; }
    else  return got->second;
}

/**
 * @brief  Index of each pack, when # > 39
 * @param  key  Key
 * @param  map  Hash table
 * @return Value (based on the idea of key-value in a hash table)
 */
inline u16 EnDecrypto::largePack (const string &key, const htbl_t &map)
{
    htbl_t::const_iterator got = map.find(key);
    if (got == map.end())
    { cerr << "Error: key '" << key << "' not found!\n";    return 0; }
    else  return got->second;
}

/**
 * @brief      Encapsulate each 3 DNA bases in 1 byte. Reduction: ~2/3
 * @param[out] packedSeq  Packed sequence
 * @param[in]  seq        Sequence
 */
inline void EnDecrypto::packSeq_3to1 (string &packedSeq, const string &seq)
{
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    string tuple;   tuple.reserve(3);
    string::const_iterator i = seq.begin(),   iEnd = seq.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);
        
        tuple.clear();
        tuple +=
           (firstNotIn = (s0!='A' && s0!='C' && s0!='G' && s0!='T' && s0!='N'))
           ? 'X' : s0;
        tuple +=
           (secondNotIn = (s1!='A' && s1!='C' && s1!='G' && s1!='T' && s1!='N'))
           ? 'X' : s1;
        tuple +=
           (thirdNotIn = (s2!='A' && s2!='C' && s2!='G' && s2!='T' && s2!='N'))
           ? 'X' : s2;
        
        packedSeq += dnaPack(tuple);
        if (firstNotIn)  packedSeq += s0;
        if (secondNotIn) packedSeq += s1;
        if (thirdNotIn)  packedSeq += s2;
    }
    
    // If seq len isn't multiple of 3, add (char) 255 before each sym
    switch (seq.length() % 3)
    {
        case 1:
            packedSeq += 255;   packedSeq += *i;
            break;

        case 2:
            packedSeq += 255;   packedSeq += *i;
            packedSeq += 255;   packedSeq += *(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 3 header symbols in 2 bytes, when # >= 40.
 *             Reduction ~1/3
 * @param[out] packed  Packed header
 * @param[in]  strIn   Header
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::packLargeHdr_3to2 (string &packed, const string &strIn,
                               const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    u16 shortTuple;
//    string hdrs = Hdrs_g;
    string hdrs = Hdrs;
    // ASCII char after the last char in HEADERS string
    const char XChar = (char) (hdrs.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (hdrs.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (hdrs.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (hdrs.find(s2)==string::npos)) ? XChar : s2;
    
        shortTuple = largePack(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // Left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // Right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;
        
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
        
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 quality score symbols in 2 bytes, when # >= 40.
 *             Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Quality scores
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::packLargeQs_3to2 (string &packed, const string &strIn,
                              const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    u16 shortTuple;
//    string qss = QSs_g;
    string qss = QSs;
    // ASCII char after the last char in QUALITY_SCORES string
    const char XChar = (char) (qss.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),  s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (qss.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (qss.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (qss.find(s2)==string::npos)) ? XChar : s2;
        
        shortTuple = largePack(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // Left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // Right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;
        
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
        
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 symbols in 2 bytes, when 16 <= # <= 39.
 *             Reduction ~1/3
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_3to2 (string &packed, const string &strIn, const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    u16 shortTuple;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        shortTuple = map.find(tuple)->second;
        packed += (byte) (shortTuple >> 8);      // Left byte
        packed += (byte) (shortTuple & 0xFF);    // Right byte
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 2 symbols in 1 byte, when 7 <= # <= 15.
 *             Reduction ~1/2
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_2to1 (string &packed, const string &strIn, const htbl_t &map)
{
    string tuple;    tuple.reserve(2);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-1;
    
    for (; i < iEnd; i += 2)
    {
        tuple.clear();    tuple = *i;    tuple += *(i+1);
        packed += (char) map.find(tuple)->second;
    }
    
    // If len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (strIn.length() & 1) { packed += 255;    packed += *i; }
}

/**
 * @brief         Encapsulate 3 symbols in 1 byte, when # = 4, 5, 6.
 *                Reduction ~2/3
 * @param packed  Packed string
 * @param strIn   Input string
 * @param map     Hash table
 */
inline void EnDecrypto::pack_3to1 (string &packed, const string &strIn, const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;

    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 5 symbols in 1 byte, when # = 3. Reduction ~4/5
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_5to1 (string &packed, const string &strIn, const htbl_t &map)
{
    string tuple;    tuple.reserve(5);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-4;
    
    for (; i < iEnd; i += 5)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);
        tuple+=*(i+2);    tuple+=*(i+3);    tuple+=*(i+4);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 5, add (char) 255 before each sym
    switch (strIn.length() % 5)
    {
        case 1:
            packed += 255;   packed += *i;
            break;
            
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
            
        case 3:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            break;

        case 4:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 7 symbols in 1 byte, when # = 2. Reduction ~6/7
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_7to1 (string &packed, const string &strIn, const htbl_t &map)
{
    string tuple;    tuple.reserve(7);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-6;
    
    for (; i < iEnd; i += 7)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);    tuple+=*(i+2);
        tuple+=*(i+3);    tuple+=*(i+4);    tuple+=*(i+5);    tuple+=*(i+6);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 7, add (char) 255 before each sym
    switch (strIn.length() % 7)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;

        case 3:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            break;

        case 4:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            break;

        case 5:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            packed += 255;   packed += *(i+4);
            break;

        case 6:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            packed += 255;   packed += *(i+4);
            packed += 255;   packed += *(i+5);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 1 symbol in 1 byte, when # = 1.
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_1to1 (string &packed, const string &strIn, const htbl_t &map)
{
    string single;    single.reserve(1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end();
    
    for (; i < iEnd; ++i)
    {
        single.clear();   single = *i;
        packed += (char) map.find(single)->second;
    }
}

/**
 * @brief  Penalty symbol
 * @param  c  Input char
 * @return Input char or (char)10='\\n'
 */
inline char EnDecrypto::penaltySym (char c)
{
    const char lookupTable[2] = {c, (char) 10};
    return lookupTable[c==(char) 254 || c==(char) 252];

//    // More readable; Perhaps slower, because of conditional branch
//    return (c != (char) 254 && c != (char) 252) ? c : (char) 10;
}

/**
 * @brief      Unpack 1 byte to 3 DNA bases -- FASTA
 * @param[out] out  DNA bases
 * @param[in]  i    Input string iterator
 */
inline void EnDecrypto::unpackSeqFA_3to1 (string &out, string::iterator &i)
{
    string tpl;    tpl.reserve(3);     // Tuplet
    out.clear();
    byte s;

    for (; *i != (char) 254; ++i)
    {
        s = (byte) *i;
        
        if (s == 255) { out += penaltySym(*(++i)); }
        else
        {
            tpl = DNA_UNPACK[s];

            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')                // ...
            { out+=tpl;                                                        }
            // Using just one 'out' makes trouble
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')           // X..
            { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2];         }

            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')           // .X.
            { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2];         }

            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')           // XX.
            { out+=penaltySym(*(++i));  out+=penaltySym(*(++i));  out+=tpl[2]; }

            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')           // ..X
            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i));         }

            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')           // X.X
            { out+=penaltySym(*(++i));  out+=tpl[1];  out+=penaltySym(*(++i)); }

            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')           // .XX
            { out+=tpl[0];  out+=penaltySym(*(++i));  out+=penaltySym(*(++i)); }

            else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));   // XXX
                   out+=penaltySym(*(++i));                                    }
        }
    }
}

/**
 * @brief      Unpack 1 byte to 3 DNA bases -- FASTQ
 * @param[out] out  Unpacked DNA bases
 * @param[in]  i    Input string iterator
 */
inline void EnDecrypto::unpackSeqFQ_3to1 (string &out, string::iterator &i)
{
    string tpl;    tpl.reserve(3);
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        // Seq len not multiple of 3
        if (*i == (char) 255) { out += penaltySym(*(++i));    continue; }

        tpl = DNA_UNPACK[(byte) *i];

        if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')    out+=tpl;       // ...

        else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')               // X..
        { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2];             }

        else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')               // .X.
        { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2];             }

        else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')               // XX.
        { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));    out+=tpl[2]; }

        else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')               // ..X
        { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i));             }

        else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')               // X.X
        { out+=penaltySym(*(++i));    out+=tpl[1];    out+=penaltySym(*(++i)); }

        else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')               // .XX
        { out+=tpl[0];    out+=penaltySym(*(++i));    out+=penaltySym(*(++i)); }

        else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));       // XXX
               out+=penaltySym(*(++i));                                        }
    }
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte, when # > 39
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  XChar   Extra character for unpacking
 * @param[in]  unpack  Table for unpacking
 */
inline void EnDecrypto::unpackLarge_read2B (string &out, string::iterator &i,
                                char XChar, const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;                      // Double byte
    string tpl;    tpl.reserve(3);    // Tuplet
    out.clear();

    while (*i != (char) 254)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));   i+=2;   continue; }
        
        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // Join two bytes
        
        tpl = unpack[doubleB];

        if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]!=XChar)              // ...
        { out+=tpl;                                                      i+=2; }

        else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]!=XChar)         // X..
        { out+=penaltySym(*(i+2));    out+=tpl[1];    out+=tpl[2];       i+=3; }

        else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]!=XChar)         // .X.
        { out+=tpl[0];    out+=penaltySym(*(i+2));    out+=tpl[2];       i+=3; }

        else if (tpl[0]==XChar && tpl[1]==XChar && tpl[2]!=XChar)         // XX.
        { out+=penaltySym(*(i+2)); out+=penaltySym(*(i+3)); out+=tpl[2]; i+=4; }

        else if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]==XChar)         // ..X
        { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(i+2));       i+=3; }

        else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]==XChar)         // X.X
        { out+=penaltySym(*(i+2)); out+=tpl[1]; out+=penaltySym(*(i+3)); i+=4; }

        else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]==XChar)         // .XX
        { out+=tpl[0]; out+=penaltySym(*(i+2)); out+=penaltySym(*(i+3)); i+=4; }

        else { out+=penaltySym(*(i+2));    out+=penaltySym(*(i+3));       // XXX
               out+=penaltySym(*(i+4));                                  i+=5; }
    }
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
inline void EnDecrypto::unpack_read2B (string &out, string::iterator &i,
                           const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;     // Double byte
    out.clear();
    
    for (; *i != (char) 254; i += 2)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    continue; }

        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // Join two bytes
        
        out += unpack[doubleB];
    }
}

/**
 * @brief      Unpack by reading 1 byte by 1 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
inline void EnDecrypto::unpack_read1B (string &out, string::iterator &i,
                           const vector<string> &unpack)
{
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(++i));    continue; }
        out += unpack[(byte) *i];
    }
}


















/**
 * @brief  Random number engine
 * @return The classic Minimum Standard rand0
 */
inline std::minstd_rand0 &EnDecrypto::randomEngine ()
{
    static std::minstd_rand0 e{};
    return e;
}

/**
 * @brief    Random number seed -- Emulate C srand()
 * @param s  Seed
 */
inline void EnDecrypto::my_srand (u32 s)
{
    randomEngine().seed(s);
}

/**
 * @brief  Random number generate -- Emulate C rand()
 * @return Random number
 */
inline int EnDecrypto::my_rand ()
{
    return (int) (randomEngine()() - randomEngine().min());
}

/**
 * @brief Shuffle/unshuffle seed generator -- For each chunk
 */
//inline u64 EnDecrypto::un_shuffleSeedGen (const u32 seedInit)
inline void EnDecrypto::un_shuffleSeedGen ()
{
    const string pass = extractPass();
    
    u64 passDigitsMult = 1;    // Multiplication of all pass digits
    for (u32 i = (u32) pass.size(); i--;)    passDigitsMult *= pass[i];
    
    // Using old rand to generate the new rand seed
    u64 seed = 0;
    
    mutx.lock();//--------------------------------------------------------------
//    my_srand(20543 * seedInit * (u32) passDigitsMult + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += ((u64) pass[i] * my_rand()) + my_rand();

//    my_srand(20543 * seedInit + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += (u64) pass[i] * my_rand();
    my_srand(20543 * (u32) passDigitsMult + 81647);
    for (byte i = (byte) pass.size(); i--;)
        seed += (u64) pass[i] * my_rand();
    mutx.unlock();//------------------------------------------------------------
    
//    seed %= 2106945901;
 
    seed_shared = seed;
//    return seed;
}

/**
 * @brief          Shuffle
 * @param[in, out] str  String to be shuffled
 */
inline void EnDecrypto::shufflePkd (string &str)
{
//    const u64 seed = un_shuffleSeedGen((u32) in.size());    // Shuffling seed
//    std::shuffle(in.begin(), in.end(), std::mt19937(seed));
    un_shuffleSeedGen();    // shuffling seed
    std::shuffle(str.begin(), str.end(), std::mt19937(seed_shared));
}

/**
 * @brief       Unshuffle
 * @param i     Shuffled string iterator
 * @param size  Size of shuffled string
 */
inline void EnDecrypto::unshufflePkd (string::iterator &i, u64 size)
{
    string shuffledStr;     // Copy of shuffled string
    for (u64 j = 0; j != size; ++j, ++i)    shuffledStr += *i;
    string::iterator shIt = shuffledStr.begin();
    i -= size;
    
    // Shuffle vector of positions
    vector<u64> vPos(size);
    std::iota(vPos.begin(), vPos.end(), 0);     // Insert 0 .. N-1
//    const u64 seed = un_shuffleSeedGen((u32) size);
//    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed));
    un_shuffleSeedGen();
    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed_shared));

    // Insert unshuffled data
    for (const u64& vI : vPos)  *(i + vI) = *shIt++;       // *shIt, then ++shIt
}

/**
 * @brief Build initialization vector (IV) for cryption
 * @param iv    IV
 * @param pass  Password
 */
inline void EnDecrypto::buildIV (byte *iv, const string &pass)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    // Using old rand to generate the new rand seed
    my_srand((u32) 7919 * pass[2] * pass[5] + 75653);
//    srand((u32) 7919 * pass[2] * pass[5] + 75653);
    u64 seed = 0;
    for (byte i = (byte) pass.size(); i--;)
        seed += ((u64) pass[i] * my_rand()) + my_rand();
//    seed += ((u64) pass[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (u32 i = (u32) AES::BLOCKSIZE; i--;)
        iv[i] = (byte) (udist(rng) % 255);
}

/**
 * @brief Build key for cryption
 * @param key  Key
 * @param pwd  password
 */
inline void EnDecrypto::buildKey (byte *key, const string &pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    // Using old rand to generate the new rand seed
    my_srand((u32) 24593 * (pwd[0] * pwd[2]) + 49157);
//    srand((u32) 24593 * (pwd[0] * pwd[2]) + 49157);
    u64 seed = 0;
    for (byte i = (byte) pwd.size(); i--;)
        seed += ((u64) pwd[i] * my_rand()) + my_rand();
//    seed += ((u64) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (u32 i = (u32) AES::DEFAULT_KEYLENGTH; i--;)
        key[i] = (byte) (udist(rng) % 255);
}

/**
 * @brief Print IV
 * @param iv  IV
 */
inline void EnDecrypto::printIV (byte *iv) const
{
    cerr << "IV = [" << (int) iv[0];
    for (u32 i = 1; i != AES::BLOCKSIZE; ++i)
        cerr << " " << (int) iv[i];
    cerr << "]\n";
}

/**
 * @brief Print key
 * @param key  Key
 */
inline void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (u32 i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}

/**
 * @brief  Get password from a file
 * @return Password (string)
 */
inline string EnDecrypto::extractPass () const
{
    ifstream in(keyFileName);
    char     c;
    string   pass;
    pass.clear();
    
    while (in.get(c))    pass += c;
    
    in.close();
    return pass;
}