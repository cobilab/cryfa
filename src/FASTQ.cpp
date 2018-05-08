/**
 * @file      FASTQ.cpp
 * @brief     Compression/Decompression of FASTQ
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>      // setw, setprecision
#include <cstring>
#include "FASTQ.hpp"

using std::chrono::high_resolution_clock;
using std::thread;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::to_string;
using std::setprecision;
using std::memset;

std::mutex mutxFQ;    /**< @brief Mutex */


/**
 * @brief  Check if the third line contains only +
 * @return True or false
 */
bool FASTQ::hasJustPlus ()  const
{
    ifstream in(IN_FILE_NAME);
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
    if (*(lFSecond+2) != '\n')  justPlus = false;    // check the symbol after +
    */
}

/**
 * @brief Compress
 */
void FASTQ::compress ()
{
    auto     startTime = high_resolution_clock::now();            // Start timer
    string   line;
    thread   arrThread[N_THREADS];
    byte     t;                   // For threads
    string   headers, qscores;
    packfq_s pkStruct;            // Collection of inputs to pass to pack...

    if (VERBOSE)    cerr << "Calculating number of different characters...\n";
    // Gather different chars and max length in all headers and quality scores
    gatherHdrQs(headers, qscores);
    // Show number of different chars in headers and qs -- Ignore '@'=64 in hdr
    if (VERBOSE)
        cerr << "In headers, they are " << headers.length() << ".\n"
             << "In quality scores, they are " << qscores.length() << ".\n";
    
    // Set Hash table and pack function
    set_hashTbl_packFn(pkStruct, headers, qscores);

    // Distribute file among threads, for reading and packing
    for (t = 0; t != N_THREADS; ++t)
        arrThread[t] = thread(&FASTQ::pack, this, pkStruct, t);
    for (t = 0; t != N_THREADS; ++t)
        if (arrThread[t].joinable())    arrThread[t].join();

    if (VERBOSE)    cerr << "Shuffling done!\n";
    
    // Join partially packed and/or shuffled files
    joinPackedFiles(headers, qscores, 'Q', hasJustPlus());

    auto finishTime = high_resolution_clock::now();                 //Stop timer
    std::chrono::duration<double> elapsed = finishTime - startTime; //Dur. (sec)

    cerr << (VERBOSE ? "Compaction done" : "Done") << ", in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

    // Cout encrypted content
    encrypt();
}

/**
 * @brief      Set hash table and pack function
 * @param[out] pkStruct  Pack structure
 * @param[in]  headers   Headers
 * @param[in]  qscores   Quality scores
 */
void FASTQ::set_hashTbl_packFn (packfq_s &pkStruct, const string &headers,
                                const string &qscores)
{
    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();
    
    // Header
    if (headersLen > MAX_C5)          // If len > 39 filter the last 39 ones
    {
        Hdrs = headers.substr(headersLen - MAX_C5);
        // ASCII char after the last char in Hdrs -- Always <= (char) 127
        HdrsX = Hdrs;    HdrsX += (char) (Hdrs.back() + 1);
        buildHashTbl(HdrMap, HdrsX, KEYLEN_C5);
        pkStruct.packHdrFPtr= &EnDecrypto::packLHdrFaFq;
    }
    else
    {
        Hdrs = headers;
        
        if (headersLen > MAX_C4)                            // 16 <= cat 5 <= 39
        {
            buildHashTbl(HdrMap, Hdrs, KEYLEN_C5);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_3to2;
        }
        else if (headersLen > MAX_C3)                       // 7 <= cat 4 <= 15
        {
            buildHashTbl(HdrMap, Hdrs, KEYLEN_C4);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_2to1;
        }
        else if (headersLen==MAX_C3 || headersLen==MID_C3   // 4 <= cat 3 <= 6
                 || headersLen==MIN_C3)
        {
            buildHashTbl(HdrMap, Hdrs, KEYLEN_C3);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_3to1;
        }
        else if (headersLen == C2)                          // cat 2 = 3
        {
            buildHashTbl(HdrMap, Hdrs, KEYLEN_C2);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_5to1;
        }
        else if (headersLen == C1)                          // cat 1 = 2
        {
            buildHashTbl(HdrMap, Hdrs, KEYLEN_C1);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_7to1;
        }
        else                                                // headersLen = 1
        {
            buildHashTbl(HdrMap, Hdrs, 1);
            pkStruct.packHdrFPtr = &EnDecrypto::pack_1to1;
        }
    }
    
    // Quality score
    if (qscoresLen > MAX_C5)              // If len > 39 filter the last 39 ones
    {
        QSs = qscores.substr(qscoresLen - MAX_C5);
        // ASCII char after last char in QUALITY_SCORES
        QSsX = QSs;     QSsX += (char) (QSs.back() + 1);
        buildHashTbl(QsMap, QSsX, KEYLEN_C5);
        pkStruct.packQSFPtr = &EnDecrypto::packLQsFq;
    }
    else
    {
        QSs = qscores;
        
        if (qscoresLen > MAX_C4)                            // 16 <= cat 5 <= 39
        {
            buildHashTbl(QsMap, QSs, KEYLEN_C5);
            pkStruct.packQSFPtr = &EnDecrypto::pack_3to2;
        }
        else if (qscoresLen > MAX_C3)                       // 7 <= cat 4 <= 15
        {
            buildHashTbl(QsMap, QSs, KEYLEN_C4);
            pkStruct.packQSFPtr = &EnDecrypto::pack_2to1;
        }
        else if (qscoresLen==MAX_C3 || qscoresLen==MID_C3   // 4 <= cat 3 <= 6
                 || qscoresLen==MIN_C3)
        {
            buildHashTbl(QsMap, QSs, KEYLEN_C3);
            pkStruct.packQSFPtr = &EnDecrypto::pack_3to1;
        }
        else if (qscoresLen == C2)                          // cat 2 = 3
        {
            buildHashTbl(QsMap, QSs, KEYLEN_C2);
            pkStruct.packQSFPtr = &EnDecrypto::pack_5to1;
        }
        else if (qscoresLen == C1)                          // cat 1 = 2
        {
            buildHashTbl(QsMap, QSs, KEYLEN_C1);
            pkStruct.packQSFPtr = &EnDecrypto::pack_7to1;
        }
        else                                                // qscoresLen = 1
        {
            buildHashTbl(QsMap, QSs, 1);
            pkStruct.packQSFPtr = &EnDecrypto::pack_1to1;
        }
    }
}

/**
 * @brief Pack. '@' at the beginning of headers is not packed
 * @param pkStruct  Pack structure
 * @param threadID  Thread ID
 */
void FASTQ::pack (const packfq_s &pkStruct, byte threadID)
{
    packFP_t packHdr = pkStruct.packHdrFPtr;    // Function pointer
    packFP_t packQS  = pkStruct.packQSFPtr;     // Function pointer
    ifstream in(IN_FILE_NAME);
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
            {
                (this->*packHdr) (context, line.substr(1), HdrMap);
                context+=(char) 254;
            }
            if (getline(in, line).good())          // Sequence
            {
                packSeq(context, line);
                context+=(char) 254;
            }
            IGNORE_THIS_LINE(in);                  // +. ignore
            if (getline(in, line).good())          // Quality score
            {
                (this->*packQS) (context, line, QsMap);
                context+=(char) 254;
            }
        }

        // shuffle
        if (!DISABLE_SHUFFLE)
        {
            mutxFQ.lock();//----------------------------------------------------
            if (VERBOSE && shuffInProgress)    cerr << "Shuffling...\n";
            shuffInProgress = false;
            mutxFQ.unlock();//--------------------------------------------------

            shuffle(context);
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
        for (u64 l = (u64) (N_THREADS-1)*BlockLine; l--;)  IGNORE_THIS_LINE(in);
    }

    pkfile.close();
    in.close();
}

/**
 * @brief      Gather chars of all headers & quality scores, excluding
 *             '@' in headers
 * @param[out] headers  Chars of all headers
 * @param[out] qscores  Chars of all quality scores
 */
void FASTQ::gatherHdrQs (string& headers, string& qscores)
{
    u32  maxHLen=0,   maxQLen=0;       // Max length of headers & quality scores
    bool hChars[127], qChars[127];
    memset(hChars+32, false, 95);
    memset(qChars+32, false, 95);

    ifstream in(IN_FILE_NAME);
    string   line;
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
}

/**
 * @brief Decompress
 */
void FASTQ::decompress ()
{
    auto       startTime = high_resolution_clock::now();          // Start timer
    char       c;                   // Chars in file
    string     headers, qscores;
    unpackfq_s upkStruct;           // Collection of inputs to pass to unpack...
    string     chunkSizeStr;        // Chunk size (string) -- For unshuffling
    thread     arrThread[N_THREADS];// Array of threads
    byte       t;                   // For threads
    u64        offset;              // To traverse decompressed file
    ifstream   in(DEC_FILENAME);

    in.ignore(1);                   // Jump over decText[0]==(char) 126
    in.get(c);    shuffled = (c==(char) 128); // Check if file had been shuffled
    while (in.get(c) && c != (char) 254)                 headers += c;
    while (in.get(c) && c != '\n' && c != (char) 253)    qscores += c;
    if (c == '\n')    justPlus = false;                 // If 3rd line is just +
    
    // Show number of different chars in headers and qs -- ignore '@'=64
    if (VERBOSE)
     cerr<< headers.length() <<" different characters are in headers.\n"
         << qscores.length() <<" different characters are in quality scores.\n";
    
    // Header -- Set unpack table and unpack function
    set_unpackTbl_unpackFn(upkStruct, headers, qscores);
    
    // Distribute file among threads, for reading and unpacking
    using unpackHQFP    = void (FASTQ::*) (const unpackfq_s&, byte);
    unpackHQFP unpackHQ =
       (headers.length() <= MAX_C5)
       ? (qscores.length() <= MAX_C5 ? &FASTQ::unpackHSQS : &FASTQ::unpackHSQL)
       : (qscores.length() >  MAX_C5 ? &FASTQ::unpackHLQL : &FASTQ::unpackHLQS);
    
    for (t = 0; t != N_THREADS; ++t)
    {
        in.get(c);
        if (c == (char) 253)
        {
            chunkSizeStr.clear();   // Chunk size
            while (in.get(c) && c != (char) 254)    chunkSizeStr += c;
            offset = stoull(chunkSizeStr);
            
            upkStruct.begPos    = in.tellg();
            upkStruct.chunkSize = offset;

            arrThread[t] = thread(unpackHQ, this, upkStruct, t);

            // Jump to the beginning of the next chunk
            in.seekg((std::streamoff) offset, std::ios_base::cur);
        }
        // End of file
        if (in.peek() == 252)    break;
    }
    // Join threads
    for (t = 0; t != N_THREADS; ++t)
        if (arrThread[t].joinable())    arrThread[t].join();

    if (VERBOSE)    cerr << "Unshuffling done!\n";

    // Close/delete decrypted file
    in.close();
    const string decFileName = DEC_FILENAME;
    std::remove(decFileName.c_str());
    
    // Join partially unpacked files
    joinUnpackedFiles();
    
    auto finishTime = high_resolution_clock::now();                 //Stop timer
    std::chrono::duration<double> elapsed = finishTime - startTime; //Dur. (sec)

    cerr << (VERBOSE ? "Decompression done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
}

/**
 * @brief      Set unpack table and unpack function
 * @param[out] upkStruct  Unpack structure
 * @param[in]  headers    Headers
 * @param[in]  qscores    Quality scores
 */
void FASTQ::set_unpackTbl_unpackFn(unpackfq_s &upkStruct, const string &headers,
                                   const string &qscores)
{
    const size_t headersLen = headers.length();
    const size_t qscoresLen = qscores.length();
    u16 keyLen_hdr=0,  keyLen_qs=0;
    
    // Header
    if (headersLen > MAX_C5)                keyLen_hdr = KEYLEN_C5;
    else if (headersLen > MAX_C4)                                       // Cat 5
    {
        upkStruct.unpackHdrFPtr = &EnDecrypto::unpack_2B;
        keyLen_hdr = KEYLEN_C5;
    }
    else
    {
        upkStruct.unpackHdrFPtr = &EnDecrypto::unpack_1B;

        if (headersLen > MAX_C3)            keyLen_hdr = KEYLEN_C4;     // Cat 4
        else if (headersLen==MAX_C3 || headersLen==MID_C3 || headersLen==MIN_C3)
                                            keyLen_hdr = KEYLEN_C3;     // Cat 3
        else if (headersLen == C2)          keyLen_hdr = KEYLEN_C2;     // Cat 2
        else if (headersLen == C1)          keyLen_hdr = KEYLEN_C1;     // Cat 1
        else                                keyLen_hdr = 1;             // = 1
    }

    // Quality score
    if (qscoresLen > MAX_C5)                keyLen_qs = KEYLEN_C5;
    else if (qscoresLen > MAX_C4)                                   // Cat 5
    {
        upkStruct.unpackQSFPtr = &EnDecrypto::unpack_2B;
        keyLen_qs = KEYLEN_C5;
    }
    else
    {
        upkStruct.unpackQSFPtr = &EnDecrypto::unpack_1B;

        if (qscoresLen > MAX_C3)            keyLen_qs = KEYLEN_C4;      // Cat 4
        else if (qscoresLen==MAX_C3 || qscoresLen==MID_C3 || qscoresLen==MIN_C3)
                                            keyLen_qs = KEYLEN_C3;      // Cat 3
        else if (qscoresLen == C2)          keyLen_qs = KEYLEN_C2;      // Cat 2
        else if (qscoresLen == C1)          keyLen_qs = KEYLEN_C1;      // Cat 1
        else                                keyLen_qs = 1;              // = 1
    }
    
    // Build unpacking tables
    if (headersLen <= MAX_C5 && qscoresLen <= MAX_C5)
    {
        buildUnpackTbl(upkStruct.hdrUnpack, headers, keyLen_hdr);
        buildUnpackTbl(upkStruct.qsUnpack,  qscores, keyLen_qs);
    }
    else if (headersLen <= MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decQscores = qscores.substr(qscoresLen - MAX_C5);
        // ASCII char after the last char in decQscores string
        string decQscoresX  = decQscores;
        decQscoresX += (upkStruct.XChar_qs = (char) (decQscores.back() + 1));
    
        buildUnpackTbl(upkStruct.hdrUnpack, headers, keyLen_hdr);
        buildUnpackTbl(upkStruct.qsUnpack,  decQscoresX, keyLen_qs);
    }
    else if (headersLen > MAX_C5 && qscoresLen > MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        const string decQscores = qscores.substr(qscoresLen - MAX_C5);
        // ASCII char after the last char in headers & quality_scores string
        string decHeadersX = decHeaders;
        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));
        string decQscoresX = decQscores;
        decQscoresX += (upkStruct.XChar_qs  = (char) (decQscores.back() + 1));

        buildUnpackTbl(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
        buildUnpackTbl(upkStruct.qsUnpack,  decQscoresX, keyLen_qs);
    }
    else if (headersLen > MAX_C5 && qscoresLen <= MAX_C5)
    {
        const string decHeaders = headers.substr(headersLen - MAX_C5);
        // ASCII char after the last char in headers string
        string decHeadersX = decHeaders;
        decHeadersX += (upkStruct.XChar_hdr = (char) (decHeaders.back() + 1));

        buildUnpackTbl(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
        buildUnpackTbl(upkStruct.qsUnpack,  qscores, keyLen_qs);
    }
}

/**
 * @brief Unpack: small header, small quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void FASTQ::unpackHSQS (const unpackfq_s &upkStruct, byte threadID)
{
    unpackFP_t unpackHdr = upkStruct.unpackHdrFPtr;    // Function pointer
    unpackFP_t unpackQS  = upkStruct.unpackQSFPtr;     // Function pointer
    pos_t      begPos    = upkStruct.begPos;
    u64        chunkSize = upkStruct.chunkSize;
    ifstream   in(DEC_FILENAME);
    string     decText, plusMore, chunkSizeStr;
    char       c;
    pos_t      endPos;
    ofstream   upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string     upkHdrOut, upkSeqOut, upkQsOut;
    string::iterator i;

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
            mutxFQ.lock();//----------------------------------------------------
            if (VERBOSE && shuffInProgress)    cerr << "Unshuffling...\n";
            shuffInProgress = false;
            mutxFQ.unlock();//--------------------------------------------------

            unshuffle(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            (this->*unpackHdr) (upkHdrOut, i, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeq(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            (this->*unpackQS) (upkQsOut, i, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = N_THREADS; t--;)
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
 * @brief Unpack: small header, large quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void FASTQ::unpackHSQL (const unpackfq_s &upkStruct, byte threadID)
{
    unpackFP_t unpackHdr = upkStruct.unpackHdrFPtr;    // Function pointer
    pos_t      begPos    = upkStruct.begPos;
    u64        chunkSize = upkStruct.chunkSize;
    ifstream   in(DEC_FILENAME);
    string     decText, plusMore, chunkSizeStr;
    char       c;
    pos_t      endPos;
    ofstream   upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string     upkHdrOut, upkSeqOut, upkQsOut;
    string::iterator i;

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
            mutxFQ.lock();//----------------------------------------------------
            if (VERBOSE && shuffInProgress)    cerr << "Unshuffling...\n";
            shuffInProgress = false;
            mutxFQ.unlock();//--------------------------------------------------

            unshuffle(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            (this->*unpackHdr) (upkHdrOut, i, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';               ++i; // Hdr

            unpackSeq(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';    ++i; // +

            unpackLarge(upkQsOut, i, upkStruct.XChar_qs, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = N_THREADS; t--;)
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
 * @brief Unpack: large header, small quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void FASTQ::unpackHLQS (const unpackfq_s &upkStruct, byte threadID)
{
    unpackFP_t unpackQS  = upkStruct.unpackQSFPtr;    // Function pointer
    pos_t      begPos    = upkStruct.begPos;
    u64        chunkSize = upkStruct.chunkSize;
    ifstream   in(DEC_FILENAME);
    string     decText, plusMore, chunkSizeStr;
    char       c;
    pos_t      endPos;
    ofstream   upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string     upkHdrOut, upkSeqOut, upkQsOut;
    string::iterator i;

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
            mutxFQ.lock();//----------------------------------------------------
            if (VERBOSE && shuffInProgress)    cerr << "Unshuffling...\n";
            shuffInProgress = false;
            mutxFQ.unlock();//--------------------------------------------------

            unshuffle(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackLarge(upkHdrOut, i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeq(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            (this->*unpackQS) (upkQsOut, i, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = N_THREADS; t--;)
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
 * @brief Unpack: large header, large quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void FASTQ::unpackHLQL (const unpackfq_s &upkStruct, byte threadID)
{
    pos_t    begPos    = upkStruct.begPos;
    u64      chunkSize = upkStruct.chunkSize;
    ifstream in(DEC_FILENAME);
    string   decText, plusMore, chunkSizeStr;
    char     c;
    pos_t    endPos;
    ofstream upkfile(UPK_FILENAME+to_string(threadID), std::ios_base::app);
    string   upkHdrOut, upkSeqOut, upkQsOut;
    string::iterator i;

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
            mutxFQ.lock();//----------------------------------------------------
            if (VERBOSE && shuffInProgress)    cerr << "Unshuffling...\n";
            shuffInProgress = false;
            mutxFQ.unlock();//--------------------------------------------------

            unshuffle(i, chunkSize);
        }

        upkfile << THR_ID_HDR + to_string(threadID) << '\n';
        do {
            upkfile << '@';

            unpackLarge(upkHdrOut, i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
            upkfile << (plusMore = upkHdrOut) << '\n';              ++i;  // Hdr

            unpackSeq(upkSeqOut, i);
            upkfile << upkSeqOut << '\n';                                 // Seq

            upkfile << (justPlus ? "+" : "+" + plusMore) << '\n';   ++i;  // +

            unpackLarge(upkQsOut, i, upkStruct.XChar_qs, upkStruct.qsUnpack);
            upkfile << upkQsOut << '\n';                                  // Qs
        } while (++i != decText.end());        // If trouble: change "!=" to "<"

        // Update the chunk size and positions (beg & end)
        for (byte t = N_THREADS; t--;)
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