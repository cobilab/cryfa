/**
 * @file      fastq.cpp
 * @brief     Compression/Decompression of FASTQ
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include "fastq.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>  // setw, std::setprecision
#include <mutex>
#include <thread>

#include "string.hpp"
#include "time.hpp"
using namespace cryfa;

std::mutex mutxFQ; /**< @brief Mutex */

/**
 * @brief  Check if the third line contains only +
 * @return True or false
 */
bool Fastq::has_just_plus() const {
  std::ifstream in(in_file);
  std::string line;

  IGNORE_THIS_LINE(in);  // Ignore header
  IGNORE_THIS_LINE(in);  // Ignore seq
  bool justPlus = !(getline(in, line).good() && line.length() > 1);

  in.close();
  return justPlus;

  /* If input was std::string, instead of file
  // check if the third line contains only +
  bool justPlus = true;
  const auto lFFirst = std::find(in.begin(), in.end(), '\n');
  const auto lFSecond = std::find(lFFirst+1, in.end(), '\n');
  if (*(lFSecond+2) != '\n')  justPlus = false;    // check the symbol after +
  */
}

/**
 * @brief Compress
 */
void Fastq::compress() {
  if (!verbose) std::cerr << bold("[+]") << " Compacting ...";
  const auto start = now();  // Start timer
  std::thread arrThread[n_threads];
  std::string headers, qscores;
  packfq_s pkStruct;  // Collection of inputs to pass to pack...

  if (verbose)
    std::cerr << bold("[+]") << " Calculating no. unique characters ...";
  // Gather different chars and max length in all headers and quality scores
  gather_h_q(headers, qscores);
  // Show number of different chars in headers and qs -- Ignore '@'=64 in hdr
  if (verbose)
    std::cerr << "\r" << bold("[+]") << " No. unique characters: headers => "
              << headers.length() << ", qscores => " << qscores.length()
              << "\n";

  // Set Hash table and pack function
  set_hashTbl_packFn(pkStruct, headers, qscores);

  // Distribute file among threads, for reading and packing
  for (byte t = 0; t != n_threads; ++t)
    arrThread[t] = std::thread(&Fastq::pack, this, pkStruct, t);
  for (auto& thr : arrThread)
    if (thr.joinable()) thr.join();

  if (verbose) {
    std::cerr << "\r" << bold("[+]") << " Shuffling done in "
              << hms(now() - shuffle_timer);
    std::cerr << bold("[+]") << " Compacting ...";
  }

  // Join partially packed and/or shuffled files
  join_packed_files(headers, qscores, 'Q', has_just_plus());

  const auto finish = now();  // Stop timer
  std::cerr << "\r" << bold("[+]") << " Compacting done in "
            << hms(finish - start);

  // Cout encrypted content
  encrypt();
}

/**
 * @brief Set hash table and pack function
 * @param[out] pkStruct  Pack structure
 * @param[in]  headers   Headers
 * @param[in]  qscores   Quality scores
 */
void Fastq::set_hashTbl_packFn(packfq_s& pkStruct, const std::string& headers,
                               const std::string& qscores) {
  const auto headersLen = headers.length();
  const auto qscoresLen = qscores.length();

  // Header
  if (headersLen > MAX_C5) {  // If len > 39 filter the last 39 ones
    Hdrs = headers.substr(headersLen - MAX_C5);
    // ASCII char after the last char in Hdrs -- Always <= (char) 127
    HdrsX = Hdrs;
    HdrsX += (char)(Hdrs.back() + 1);
    build_hash_tbl(HdrMap, HdrsX, KEYLEN_C5);
    pkStruct.packHdrFPtr = &EnDecrypto::pack_hL_fa_fq;
  } else {
    Hdrs = headers;

    if (headersLen > MAX_C4) {  // 16 <= cat 5 <= 39
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C5);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_3to2;
    } else if (headersLen > MAX_C3) {  // 7 <= cat 4 <= 15
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C4);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_2to1;
    } else if (headersLen == MAX_C3 || headersLen == MID_C3  // 4 <= cat 3 <= 6
               || headersLen == MIN_C3) {
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C3);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_3to1;
    } else if (headersLen == C2) {  // cat 2 = 3
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C2);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_5to1;
    } else if (headersLen == C1) {  // cat 1 = 2
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C1);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_7to1;
    } else {  // headersLen = 1
      build_hash_tbl(HdrMap, Hdrs, 1);
      pkStruct.packHdrFPtr = &EnDecrypto::pack_1to1;
    }
  }

  // Quality score
  if (qscoresLen > MAX_C5) {  // If len > 39 filter the last 39 ones
    QSs = qscores.substr(qscoresLen - MAX_C5);
    // ASCII char after last char in QUALITY_SCORES
    QSsX = QSs;
    QSsX += (char)(QSs.back() + 1);
    build_hash_tbl(QsMap, QSsX, KEYLEN_C5);
    pkStruct.packQSFPtr = &EnDecrypto::pack_qL_fq;
  } else {
    QSs = qscores;

    if (qscoresLen > MAX_C4) {  // 16 <= cat 5 <= 39
      build_hash_tbl(QsMap, QSs, KEYLEN_C5);
      pkStruct.packQSFPtr = &EnDecrypto::pack_3to2;
    } else if (qscoresLen > MAX_C3) {  // 7 <= cat 4 <= 15
      build_hash_tbl(QsMap, QSs, KEYLEN_C4);
      pkStruct.packQSFPtr = &EnDecrypto::pack_2to1;
    } else if (qscoresLen == MAX_C3 || qscoresLen == MID_C3  // 4 <= cat 3 <= 6
               || qscoresLen == MIN_C3) {
      build_hash_tbl(QsMap, QSs, KEYLEN_C3);
      pkStruct.packQSFPtr = &EnDecrypto::pack_3to1;
    } else if (qscoresLen == C2) {  // cat 2 = 3
      build_hash_tbl(QsMap, QSs, KEYLEN_C2);
      pkStruct.packQSFPtr = &EnDecrypto::pack_5to1;
    } else if (qscoresLen == C1) {  // cat 1 = 2
      build_hash_tbl(QsMap, QSs, KEYLEN_C1);
      pkStruct.packQSFPtr = &EnDecrypto::pack_7to1;
    } else {  // qscoresLen = 1
      build_hash_tbl(QsMap, QSs, 1);
      pkStruct.packQSFPtr = &EnDecrypto::pack_1to1;
    }
  }
}

/**
 * @brief Pack. '@' at the beginning of headers is not packed
 * @param pkStruct  Pack structure
 * @param threadID  Thread ID
 */
void Fastq::pack(const packfq_s& pkStruct, byte threadID) {
  packFP_t packHdr = pkStruct.packHdrFPtr;  // Function pointer
  packFP_t packQS = pkStruct.packQSFPtr;    // Function pointer
  std::ifstream in(in_file);
  std::ofstream pkfile(PK_FNAME + std::to_string(threadID), std::ios_base::app);

  // Lines ignored at the beginning
  for (u64 l = (u64)threadID * BlockLine; l--;) IGNORE_THIS_LINE(in);

  while (in.peek() != EOF) {
    std::string context;  // Output std::string

    std::string line;
    for (u64 l = 0; l != BlockLine; l += 4) {  // Process 4 lines by 4 lines
      if (getline(in, line).good()) {          // Header -- Ignore '@'
        (this->*packHdr)(context, line.substr(1), HdrMap);
        context += (char)254;
      }
      if (getline(in, line).good()) {  // Sequence
        pack_seq(context, line);
        context += (char)254;
      }
      IGNORE_THIS_LINE(in);            // +. ignore
      if (getline(in, line).good()) {  // Quality score
        (this->*packQS)(context, line, QsMap);
        context += (char)254;
      }
    }

    // shuffle
    if (!stop_shuffle) {
      mutxFQ.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Shuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFQ.unlock();  //--------------------------------------------------

      shuffle(context);
    }

    // For unshuffling: insert the size of packed context in the beginning
    std::string contextSize;
    contextSize += (char)253;
    contextSize += std::to_string(context.size());
    contextSize += (char)254;
    context.insert(0, contextSize);

    // Write header containing threadID for each
    pkfile << THR_ID_HDR << std::to_string(threadID) << '\n';
    pkfile << context << '\n';

    // Ignore to go to the next related chunk
    for (u64 l = (u64)(n_threads - 1) * BlockLine; l--;) IGNORE_THIS_LINE(in);
  }

  pkfile.close();
  in.close();
}

/**
 * @brief Gather chars of all headers & quality scores, excluding '@' in headers
 * @param[out] headers  Chars of all headers
 * @param[out] qscores  Chars of all quality scores
 */
void Fastq::gather_h_q(std::string& headers, std::string& qscores) {
  u32 maxHLen = 0, maxQLen = 0;  // Max length of headers & quality scores
  bool hChars[127], qChars[127];
  std::memset(hChars + 32, false, 95);
  std::memset(qChars + 32, false, 95);

  std::ifstream in(in_file);
  for (std::string line; !in.eof();) {
    if (getline(in, line).good()) {
      for (char c : line) hChars[c] = true;
      if (line.size() > maxHLen) maxHLen = (u32)line.size();
    }

    IGNORE_THIS_LINE(in);  // Ignore sequence
    IGNORE_THIS_LINE(in);  // Ignore +

    if (getline(in, line).good()) {
      for (char c : line) qChars[c] = true;
      if (line.size() > maxQLen) maxQLen = (u32)line.size();
    }
  }
  in.close();

  // Number of lines read from input file while compression
  BlockLine = (u32)(4 * (BLOCK_SIZE / (maxHLen + 2 * maxQLen)));
  if (!BlockLine) BlockLine = 4;

  // Gather the characters -- ignore '@'=64 for headers
  for (byte i = 32; i != 64; ++i)
    if (*(hChars + i)) headers += i;
  for (byte i = 65; i != 127; ++i)
    if (*(hChars + i)) headers += i;
  for (byte i = 32; i != 127; ++i)
    if (*(qChars + i)) qscores += i;
}

/**
 * @brief Decompress
 */
void Fastq::decompress() {
  if (!verbose) std::cerr << bold("[+]") << " Decompressing ...";
  const auto start = now();  // Start timer

  char c;  // Chars in file
  std::string headers, qscores;
  unpackfq_s upkStruct;  // Collection of inputs to pass to unpack...
  std::thread arrThread[n_threads];  // Array of threads
  std::ifstream in(DEC_FNAME);

  in.ignore(1);  // Jump over decText[0]==(char) 126
  in.get(c);
  shuffled = (c == (char)128);  // Check if file had been shuffled
  if (verbose)
    std::cerr << bold("[+]") << " Extracting no. unique characters ...";
  while (in.get(c) && c != (char)254) headers += c;
  while (in.get(c) && c != '\n' && c != (char)253) qscores += c;
  // Show number of different chars in headers and qs -- ignore '@'=64
  if (verbose)
    std::cerr << "\r" << bold("[+]") << " No. unique characters: headers => "
              << headers.length() << ", qscores => " << qscores.length()
              << "\n";
  if (c == '\n') justPlus = false;  // If 3rd line is just +

  // Header -- Set unpack table and unpack function
  set_unpackTbl_unpackFn(upkStruct, headers, qscores);

  // Distribute file among threads, for reading and unpacking
  using unpackHQFP = void (Fastq::*)(const unpackfq_s&, byte);
  unpackHQFP unpackHQ =
      (headers.length() <= MAX_C5)
          ? (qscores.length() <= MAX_C5 ? &Fastq::unpack_hS_qS
                                        : &Fastq::unpack_hS_qL)
          : (qscores.length() > MAX_C5 ? &Fastq::unpack_hL_qL
                                       : &Fastq::unpack_hL_qS);

  for (byte t = 0; t != n_threads; ++t) {
    in.get(c);
    if (c == (char)253) {
      std::string chunkSizeStr;  // Chunk size (std::string) -- For unshuffling
      while (in.get(c) && c != (char)254) chunkSizeStr += c;
      const u64 offset = stoull(chunkSizeStr);  // To traverse decompressed file

      upkStruct.begPos = in.tellg();
      upkStruct.chunkSize = offset;

      arrThread[t] = std::thread(unpackHQ, this, upkStruct, t);

      // Jump to the beginning of the next chunk
      in.seekg((std::streamoff)offset, std::ios_base::cur);
    }
    // End of file
    if (in.peek() == 252) break;
  }
  // Join threads
  for (auto& thr : arrThread)
    if (thr.joinable()) thr.join();

  if (verbose) {
    std::cerr << "\r" << bold("[+]") << " Unshuffling done in "
              << hms(now() - shuffle_timer);
    std::cerr << bold("[+]") << " Decompressing ...";
  }

  // Close/delete decrypted file
  in.close();
  const std::string decFileName = DEC_FNAME;
  std::remove(decFileName.c_str());

  // Join partially unpacked files
  join_unpacked_files();

  const auto finish = now();  // Stop timer
  std::cerr << "\r" << bold("[+]") << " Decompressing done in "
            << hms(finish - start);
}

/**
 * @brief Set unpack table and unpack function
 * @param[out] upkStruct  Unpack structure
 * @param[in]  headers    Headers
 * @param[in]  qscores    Quality scores
 */
void Fastq::set_unpackTbl_unpackFn(unpackfq_s& upkStruct,
                                   const std::string& headers,
                                   const std::string& qscores) {
  const auto headersLen = headers.length();
  const auto qscoresLen = qscores.length();
  u16 keyLen_hdr = 0, keyLen_qs = 0;

  // Header
  if (headersLen > MAX_C5)
    keyLen_hdr = KEYLEN_C5;
  else if (headersLen > MAX_C4) {  // Cat 5
    upkStruct.unpackHdrFPtr = &EnDecrypto::unpack_2B;
    keyLen_hdr = KEYLEN_C5;
  } else {
    upkStruct.unpackHdrFPtr = &EnDecrypto::unpack_1B;

    if (headersLen > MAX_C3)
      keyLen_hdr = KEYLEN_C4;  // Cat 4
    else if (headersLen == MAX_C3 || headersLen == MID_C3 ||
             headersLen == MIN_C3)
      keyLen_hdr = KEYLEN_C3;  // Cat 3
    else if (headersLen == C2)
      keyLen_hdr = KEYLEN_C2;  // Cat 2
    else if (headersLen == C1)
      keyLen_hdr = KEYLEN_C1;  // Cat 1
    else
      keyLen_hdr = 1;  // = 1
  }

  // Quality score
  if (qscoresLen > MAX_C5)
    keyLen_qs = KEYLEN_C5;
  else if (qscoresLen > MAX_C4) {  // Cat 5
    upkStruct.unpackQSFPtr = &EnDecrypto::unpack_2B;
    keyLen_qs = KEYLEN_C5;
  } else {
    upkStruct.unpackQSFPtr = &EnDecrypto::unpack_1B;

    if (qscoresLen > MAX_C3)
      keyLen_qs = KEYLEN_C4;  // Cat 4
    else if (qscoresLen == MAX_C3 || qscoresLen == MID_C3 ||
             qscoresLen == MIN_C3)
      keyLen_qs = KEYLEN_C3;  // Cat 3
    else if (qscoresLen == C2)
      keyLen_qs = KEYLEN_C2;  // Cat 2
    else if (qscoresLen == C1)
      keyLen_qs = KEYLEN_C1;  // Cat 1
    else
      keyLen_qs = 1;  // = 1
  }

  // Build unpacking tables
  if (headersLen <= MAX_C5 && qscoresLen <= MAX_C5) {
    build_unpack_tbl(upkStruct.hdrUnpack, headers, keyLen_hdr);
    build_unpack_tbl(upkStruct.qsUnpack, qscores, keyLen_qs);
  } else if (headersLen <= MAX_C5 && qscoresLen > MAX_C5) {
    const std::string decQscores = qscores.substr(qscoresLen - MAX_C5);
    // ASCII char after the last char in decQscores std::string
    std::string decQscoresX = decQscores;
    decQscoresX += (upkStruct.XChar_qs = (char)(decQscores.back() + 1));

    build_unpack_tbl(upkStruct.hdrUnpack, headers, keyLen_hdr);
    build_unpack_tbl(upkStruct.qsUnpack, decQscoresX, keyLen_qs);
  } else if (headersLen > MAX_C5 && qscoresLen > MAX_C5) {
    const std::string decHeaders = headers.substr(headersLen - MAX_C5);
    const std::string decQscores = qscores.substr(qscoresLen - MAX_C5);
    // ASCII char after the last char in headers & quality_scores std::string
    std::string decHeadersX = decHeaders;
    decHeadersX += (upkStruct.XChar_hdr = (char)(decHeaders.back() + 1));
    std::string decQscoresX = decQscores;
    decQscoresX += (upkStruct.XChar_qs = (char)(decQscores.back() + 1));

    build_unpack_tbl(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
    build_unpack_tbl(upkStruct.qsUnpack, decQscoresX, keyLen_qs);
  } else if (headersLen > MAX_C5 && qscoresLen <= MAX_C5) {
    const std::string decHeaders = headers.substr(headersLen - MAX_C5);
    // ASCII char after the last char in headers std::string
    std::string decHeadersX = decHeaders;
    decHeadersX += (upkStruct.XChar_hdr = (char)(decHeaders.back() + 1));

    build_unpack_tbl(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
    build_unpack_tbl(upkStruct.qsUnpack, qscores, keyLen_qs);
  }
}

/**
 * @brief Unpack: small header, small quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void Fastq::unpack_hS_qS(const unpackfq_s& upkStruct, byte threadID) {
  unpackFP_t unpackHdr = upkStruct.unpackHdrFPtr;  // Function pointer
  unpackFP_t unpackQS = upkStruct.unpackQSFPtr;    // Function pointer
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(UPK_FNAME + std::to_string(threadID),
                        std::ios_base::app);
  std::string upkHdrOut, upkSeqOut, upkQsOut;
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read the file from this position
    // Take a chunk of decrypted file
    std::string decText;
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFQ.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFQ.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }

    content += THR_ID_HDR + std::to_string(threadID) + "\n";
    do {
      content += '@';
      std::string plusMore;

      (this->*unpackHdr)(upkHdrOut, i, upkStruct.hdrUnpack);
      plusMore = upkHdrOut;
      content += upkHdrOut + "\n";
      ++i;  // Hdr

      unpack_seq(upkSeqOut, i);
      content += upkSeqOut + "\n";  // Seq

      content += (justPlus ? "+" : "+" + plusMore) + "\n";
      ++i;  // +

      (this->*unpackQS)(upkQsOut, i, upkStruct.qsUnpack);
      content += upkQsOut + "\n";    // Qs
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) chunkSizeStr += c;

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= BLOCK_SIZE) {
      write_content();
      content.clear();
      content.reserve(BLOCK_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}

/**
 * @brief Unpack: small header, large quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void Fastq::unpack_hS_qL(const unpackfq_s& upkStruct, byte threadID) {
  unpackFP_t unpackHdr = upkStruct.unpackHdrFPtr;  // Function pointer
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(UPK_FNAME + std::to_string(threadID),
                        std::ios_base::app);
  std::string upkHdrOut, upkSeqOut, upkQsOut;
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read file from this position
    // Take a chunk of decrypted file
    std::string decText;
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFQ.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFQ.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }

    content += THR_ID_HDR + std::to_string(threadID) + "\n";
    do {
      content += '@';
      std::string plusMore;

      (this->*unpackHdr)(upkHdrOut, i, upkStruct.hdrUnpack);
      plusMore = upkHdrOut;
      content += upkHdrOut + "\n";
      ++i;  // Hdr

      unpack_seq(upkSeqOut, i);
      content += upkSeqOut + "\n";  // Seq

      content += (justPlus ? "+" : "+" + plusMore) + "\n";
      ++i;  // +

      unpack_large(upkQsOut, i, upkStruct.XChar_qs, upkStruct.qsUnpack);
      content += upkQsOut + "\n";    // Qs
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) chunkSizeStr += c;

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= BLOCK_SIZE) {
      write_content();
      content.clear();
      content.reserve(BLOCK_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}

/**
 * @brief Unpack: large header, small quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void Fastq::unpack_hL_qS(const unpackfq_s& upkStruct, byte threadID) {
  unpackFP_t unpackQS = upkStruct.unpackQSFPtr;  // Function pointer
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(UPK_FNAME + std::to_string(threadID),
                        std::ios_base::app);
  std::string upkHdrOut, upkSeqOut, upkQsOut;
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read file from this position
    // Take a chunk of decrypted file
    std::string decText;
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFQ.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFQ.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }

    content += THR_ID_HDR + std::to_string(threadID) + "\n";
    do {
      content += "@";
      std::string plusMore;

      unpack_large(upkHdrOut, i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
      plusMore = upkHdrOut;
      content += upkHdrOut + "\n";
      ++i;  // Hdr

      unpack_seq(upkSeqOut, i);
      content += upkSeqOut + "\n";  // Seq

      content += (justPlus ? "+" : "+" + plusMore) + "\n";
      ++i;  // +

      (this->*unpackQS)(upkQsOut, i, upkStruct.qsUnpack);
      content += upkQsOut + "\n";    // Qs
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) chunkSizeStr += c;

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= BLOCK_SIZE) {
      write_content();
      content.clear();
      content.reserve(BLOCK_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}

/**
 * @brief Unpack: large header, large quality score.
 *        '@' at the beginning of headers not packed
 * @param upkStruct  Unpack structure
 * @param threadID   Thread ID
 */
void Fastq::unpack_hL_qL(const unpackfq_s& upkStruct, byte threadID) {
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(UPK_FNAME + std::to_string(threadID),
                        std::ios_base::app);
  std::string upkHdrOut, upkSeqOut, upkQsOut;
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read file from this position
    // Take a chunk of decrypted file
    std::string decText;
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFQ.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFQ.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }

    content += THR_ID_HDR + std::to_string(threadID) + "\n";
    do {
      content += "@";
      std::string plusMore;

      unpack_large(upkHdrOut, i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
      plusMore = upkHdrOut;
      content += upkHdrOut + "\n";
      ++i;  // Hdr

      unpack_seq(upkSeqOut, i);
      content += upkSeqOut + "\n";  // Seq

      content += (justPlus ? "+" : "+" + plusMore) + "\n";
      ++i;  // +

      unpack_large(upkQsOut, i, upkStruct.XChar_qs, upkStruct.qsUnpack);
      content += upkQsOut + "\n";    // Qs
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) chunkSizeStr += c;

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= BLOCK_SIZE) {
      write_content();
      content.clear();
      content.reserve(BLOCK_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}