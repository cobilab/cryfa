// SPDX-FileCopyrightText: 2026 Morteza Hosseini
// SPDX-License-Identifier: GPL-3.0-only

/**
 * @file fasta.cpp
 * @brief Compression/Decompression of FASTA
 */

#include "fasta.hpp"

#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iomanip>  // setw, std::setprecision
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "ordered_pipeline.hpp"
#include "plaintext_stream.hpp"
#include "string.hpp"
#include "time.hpp"
using namespace cryfa;

std::mutex mutxFA;

namespace {
struct FastaRecord {
  std::string header;
  std::vector<std::string> sequence_lines;
};

struct FastaChunk {
  std::vector<FastaRecord> records;
};
}  // namespace

/**
 * @brief Compress
 */
void Fasta::compress() {
  if (!verbose) {
    std::cerr << bold("[+]") << " Compacting ...";
  }
  const auto start = now();  // Start timer

  std::string headers;
  packfa_s pkStruct;  // Collection of inputs to pass to pack...

  if (verbose) {
    std::cerr << bold("[+]") << " Calculating no. unique characters ...";
  }
  // Gather different chars in all headers and max length in all bases
  gather_h_bs(headers);
  // Show number of different chars in headers -- ignore '>'=62
  if (verbose) {
    std::cerr << "\r" << bold("[+]") << " No. unique characters: headers => " << headers.length()
              << "   \n";
  }

  // Set Hash table and pack function
  set_hashTbl_packFn(pkStruct, headers);

  auto read_chunk = [this, in = std::ifstream(in_file),
                     pending_header = std::string{}]() mutable -> std::optional<FastaChunk> {
    FastaChunk chunk;
    std::string line;
    u64 chunk_bytes = 0;

    if (pending_header.empty()) {
      while (std::getline(in, line)) {
        if (!line.empty() && line.front() == '>') {
          pending_header = std::move(line);
          break;
        }
      }
    }

    if (pending_header.empty()) {
      return std::nullopt;
    }

    while (!pending_header.empty()) {
      FastaRecord record;
      record.header = std::move(pending_header);
      pending_header.clear();
      chunk_bytes += record.header.size() + 1;

      while (std::getline(in, line)) {
        if (!line.empty() && line.front() == '>') {
          pending_header = std::move(line);
          break;
        }

        chunk_bytes += line.size() + 1;
        record.sequence_lines.push_back(std::move(line));
      }

      chunk.records.push_back(std::move(record));
      if (chunk_bytes >= CHUNK_TARGET_SIZE) {
        break;
      }
    }

    return chunk;
  };

  auto pack_chunk = [this, pkStruct](FastaChunk chunk) {
    packFP_t packHdr = pkStruct.packHdrFP;
    std::string context;
    context.reserve(CHUNK_TARGET_SIZE);
    std::string seq;
    seq.reserve(CHUNK_TARGET_SIZE);

    for (const FastaRecord& record : chunk.records) {
      context += (char)253;
      (this->*packHdr)(context, record.header.substr(1), HdrMap);
      context += (char)254;

      seq.clear();
      for (const std::string& line : record.sequence_lines) {
        seq += line;
        seq += (char)252;
      }
      if (!seq.empty()) {
        seq.pop_back();
        pack_seq(context, seq);
        context += (char)254;
      }
    }

    if (!stop_shuffle) {
      mutxFA.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Shuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFA.unlock();  //--------------------------------------------------

      shuffle(context);
    }

    std::string packed = std::format("{}{}{}", (char)253, context.size(), (char)254);
    packed += context;
    return packed;
  };

  encrypt_stream([&](const PlaintextSink& emit) {
    std::string header;
    header.reserve(headers.size() + 3);
    header += (char)127;
    header += (!stop_shuffle ? (char)128 : (char)129);
    header += headers;
    header += (char)254;
    emit(header);

    run_ordered_pipeline<FastaChunk>(n_threads, read_chunk, pack_chunk, emit);
    emit(std::string(1, (char)252));

    if (verbose && !stop_shuffle) {
      std::cerr << "\r" << bold("[+]") << " Shuffling done in " << hms(now() - shuffle_timer);
      std::cerr << bold("[+]") << " Compacting ...";
    }

    const auto finish = now();  // Stop timer
    std::cerr << "\r" << bold("[+]") << " Compacting done in " << hms(finish - start);
  });
}

/**
 * @brief Set hash table and pack function
 * @param[out] pkStruct Pack structure
 * @param headers Headers
 */
void Fasta::set_hashTbl_packFn(packfa_s& pkStruct, const std::string& headers) {
  const size_t headersLen = headers.length();

  // Header
  if (headersLen > MAX_C5) {  // If len > 39, filter the last 39 ones
    Hdrs = headers.substr(headersLen - MAX_C5);
    // ASCII char after the last char in Hdrs -- always <= (char) 127
    HdrsX = Hdrs;
    HdrsX += (char)(Hdrs.back() + 1);
    build_hash_tbl(HdrMap, HdrsX, KEYLEN_C5);
    pkStruct.packHdrFP = &EnDecrypto::pack_hL_fa_fq;
  } else {
    Hdrs = headers;

    if (headersLen > MAX_C4) {  // 16 <= cat 5 <= 39
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C5);
      pkStruct.packHdrFP = &EnDecrypto::pack_3to2;
    } else if (headersLen > MAX_C3) {  // 7 <= cat 4 <= 15
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C4);
      pkStruct.packHdrFP = &EnDecrypto::pack_2to1;
    } else if (headersLen == MAX_C3 || headersLen == MID_C3  // 4 <= cat 3 <= 6
               || headersLen == MIN_C3) {
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C3);
      pkStruct.packHdrFP = &EnDecrypto::pack_3to1;
    } else if (headersLen == C2) {  // cat 2 = 3
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C2);
      pkStruct.packHdrFP = &EnDecrypto::pack_5to1;
    } else if (headersLen == C1) {  // cat 1 = 2
      build_hash_tbl(HdrMap, Hdrs, KEYLEN_C1);
      pkStruct.packHdrFP = &EnDecrypto::pack_7to1;
    } else {  // headersLen = 1
      build_hash_tbl(HdrMap, Hdrs, 1);
      pkStruct.packHdrFP = &EnDecrypto::pack_1to1;
    }
  }
}

/**
 * @brief Pack. '>' at the beginning of headers is not packed
 * @param pkStruct Pack structure
 * @param threadID Thread ID
 */
void Fasta::pack(const packfa_s& pkStruct, byte threadID) {
  packFP_t packHdr = pkStruct.packHdrFP;  // Function pointer
  std::ifstream in(in_file);
  std::string line, context, seq;
  context.reserve(CHUNK_TARGET_SIZE);
  seq.reserve(CHUNK_TARGET_SIZE);
  std::ofstream pkfile(std::format("{}{}", PK_FNAME, static_cast<unsigned>(threadID)),
                       std::ios_base::app);
  // Lines ignored at the beginning
  for (u64 l = (u64)threadID * BlockLine; l--;) {
    IGNORE_THIS_LINE(in);
  }

  while (in.peek() != EOF) {
    context.clear();
    seq.clear();

    for (u64 l = BlockLine; l-- && getline(in, line).good();) {
      // Header
      if (line.front() == '>') {
        // Previous seq
        if (!seq.empty()) {
          seq.pop_back();  // Remove the last '\n'
          pack_seq(context, seq);
          context += (char)254;
        }
        seq.clear();

        // Header line
        context += (char)253;
        (this->*packHdr)(context, line.substr(1), HdrMap);
        context += (char)254;
      }

      // Empty line. (char) 252 instead of line feed
      else if (line.empty()) {
        seq += (char)252;
      }

      // Sequence
      else {
        // (char) 252 instead of '\n' at the end of each seq line
        seq += line;
        seq += (char)252;
      }
    }
    if (!seq.empty()) {
      seq.pop_back();  // Remove the last '\n'

      // The last seq
      pack_seq(context, seq);
      context += (char)254;
    }

    // Shuffle
    if (!stop_shuffle) {
      mutxFA.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Shuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFA.unlock();  //--------------------------------------------------

      shuffle(context);
    }

    // For unshuffling: insert the size of packed context in the beginning
    std::string contextSize = std::format("{}{}{}", (char)253, context.size(), (char)254);
    context.insert(0, contextSize);

    // Write header containing threadID for each partially packed file
    pkfile << std::format("{}{}\n", THR_ID_HDR, static_cast<unsigned>(threadID));
    pkfile << context << '\n';

    // Ignore to go to the next related chunk
    for (u64 l = (u64)(n_threads - 1) * BlockLine; l--;) {
      IGNORE_THIS_LINE(in);
    }
  }

  pkfile.close();
  in.close();
}

/**
 * @brief Gather chars of all headers & max length of DNA bases lines, excluding '>'
 * @param[out] headers Chars of all headers
 */
void Fasta::gather_h_bs(std::string& headers) {
  u32 maxBLen = 0;  // Max length of each line of bases
  bool hChars[127];
  std::memset(hChars + 32, false, 95);

  std::ifstream in(in_file);
  std::string line;
  while (getline(in, line).good()) {
    if (line[0] == '>') {
      for (char c : line) {
        hChars[c] = true;
      }
    } else if (line.size() > maxBLen) {
      maxBLen = (u32)line.size();
    }
  }
  in.close();

  // Number of lines read from input file while compression
  BlockLine = (u32)(CHUNK_TARGET_SIZE / maxBLen);
  if (!BlockLine) {
    BlockLine = 2;
  }

  // Gather the characters -- Ignore '>'=62 for headers
  for (byte i = 32; i != 62; ++i) {
    if (*(hChars + i)) {
      headers += i;
    }
  }
  for (byte i = 63; i != 127; ++i) {
    if (*(hChars + i)) {
      headers += i;
    }
  }
}

/**
 * @brief Decompress
 */
void Fasta::decompress() {
  if (!verbose) {
    std::cerr << bold("[+]") << " Decompressing ...";
  }
  const auto start = now();  // Start timer

  PlaintextStream plaintext;
  std::exception_ptr decrypt_error;
  std::thread decrypt_thread([&]() {
    try {
      decrypt_stream([&](std::string_view decrypted) { plaintext.push(decrypted); });
      plaintext.close();
    } catch (...) {
      decrypt_error = std::current_exception();
      plaintext.fail(decrypt_error);
    }
  });

  auto join_decrypt = [&]() {
    if (decrypt_thread.joinable()) {
      decrypt_thread.join();
    }
    if (decrypt_error) {
      std::rethrow_exception(decrypt_error);
    }
  };

  std::string headers;
  unpackfa_s upkStruct;  // Collection of inputs to pass to unpack...

  try {
    const auto file_type = plaintext.get();
    if (!file_type || *file_type != (char)127) {
      throw std::runtime_error("corrupted file.");
    }

    const auto shuffle_flag = plaintext.get();
    if (!shuffle_flag || (*shuffle_flag != (char)128 && *shuffle_flag != (char)129)) {
      throw std::runtime_error("corrupted file.");
    }

    shuffled = (*shuffle_flag == (char)128);  // Check if file had been shuffled
    if (verbose) {
      std::cerr << bold("[+]") << " Extracting no. unique characters ...";
    }
    if (!plaintext.read_until((char)254, headers)) {
      throw std::runtime_error("corrupted file.");
    }
    if (verbose) {  // Show number of different chars in headers -- Ignore '>'=62
      std::cerr << "\r" << bold("[+]") << " No. unique characters: headers => " << headers.length()
                << "    \n";
    }

    // Header -- Set unpack table and unpack function
    set_unpackTbl_unpackFn(upkStruct, headers);
    const bool has_small_header = headers.length() <= MAX_C5;

    auto read_chunk = [&]() -> std::optional<std::string> {
      const auto marker = plaintext.get();
      if (!marker || *marker == (char)252) {
        return std::nullopt;
      }
      if (*marker != (char)253) {
        throw std::runtime_error("corrupted file.");
      }

      std::string chunk_size_str;
      if (!plaintext.read_until((char)254, chunk_size_str) || chunk_size_str.empty()) {
        throw std::runtime_error("corrupted file.");
      }

      std::string chunk;
      if (!plaintext.read_bytes(std::stoull(chunk_size_str), chunk)) {
        throw std::runtime_error("corrupted file.");
      }
      return chunk;
    };

    auto unpack_chunk = [this, upkStruct, has_small_header](std::string decText) mutable {
      if (decText.empty()) {
        return std::string{};
      }

      auto i = decText.begin();

      // Unshuffle
      if (shuffled) {
        mutxFA.lock();  //--------------------------------------------------
        if (verbose && shuffInProg) {
          std::cerr << bold("[+]") << " Unshuffling ...";
          shuffle_timer = now();
        }
        shuffInProg = false;
        mutxFA.unlock();  //------------------------------------------------

        unshuffle(i, decText.size());
      }

      std::string upkhdrOut, upkSeqOut;
      std::string content;
      content.reserve(decText.size() * 2);
      do {
        if (*i == (char)253) {  // Hdr
          if (has_small_header) {
            (this->*upkStruct.unpackHdrFP)(upkhdrOut, ++i, upkStruct.hdrUnpack);
          } else {
            unpack_large(upkhdrOut, ++i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
          }
          content += std::format(">{}\n", upkhdrOut);
        } else {  // Seq
          unpack_seq(upkSeqOut, i);
          content += std::format("{}\n", upkSeqOut);
        }
      } while (++i != decText.end());

      return content;
    };

    run_ordered_pipeline<std::string>(n_threads, read_chunk, unpack_chunk,
                                      [](const std::string& output) { std::cout << output; });

    if (verbose && shuffled) {
      std::cerr << "\r" << bold("[+]") << " Unshuffling done in " << hms(now() - shuffle_timer);
      std::cerr << bold("[+]") << " Decompressing ...";
    }
  } catch (...) {
    plaintext.fail(std::current_exception());
    if (decrypt_thread.joinable()) {
      decrypt_thread.join();
    }
    throw;
  }

  join_decrypt();

  const auto finish = now();  // Stop timer
  std::cerr << "\r" << bold("[+]") << " Decompressing done in " << hms(finish - start);
}

/**
 * @brief Set unpack table and unpack function
 * @param[out] upkStruct Unpack structure
 * @param headers Headers
 */
void Fasta::set_unpackTbl_unpackFn(unpackfa_s& upkStruct, const std::string& headers) {
  const size_t headersLen = headers.length();
  u16 keyLen_hdr = 0;

  if (headersLen > MAX_C5) {
    keyLen_hdr = KEYLEN_C5;
  } else if (headersLen > MAX_C4) {  // Cat 5
    upkStruct.unpackHdrFP = &EnDecrypto::unpack_2B;
    keyLen_hdr = KEYLEN_C5;
  } else {
    upkStruct.unpackHdrFP = &EnDecrypto::unpack_1B;

    if (headersLen > MAX_C3) {
      keyLen_hdr = KEYLEN_C4;  // Cat 4
    } else if (headersLen == MAX_C3 || headersLen == MID_C3 || headersLen == MIN_C3) {
      keyLen_hdr = KEYLEN_C3;  // Cat 3
    } else if (headersLen == C2) {
      keyLen_hdr = KEYLEN_C2;  // Cat 2
    } else if (headersLen == C1) {
      keyLen_hdr = KEYLEN_C1;  // Cat 1
    } else {
      keyLen_hdr = 1;  // = 1
    }
  }

  // Build unpacking tables
  if (headersLen <= MAX_C5) {
    build_unpack_tbl(upkStruct.hdrUnpack, headers, keyLen_hdr);
  } else {
    const std::string decHeaders = headers.substr(headersLen - MAX_C5);
    // ASCII char after the last char in headers std::string
    std::string decHeadersX = decHeaders;
    decHeadersX += (upkStruct.XChar_hdr = (char)(decHeaders.back() + 1));

    build_unpack_tbl(upkStruct.hdrUnpack, decHeadersX, keyLen_hdr);
  }
}

/**
 * @brief Unpack: small header
 * @param upkStruct Unpack structure
 * @param threadID Thread ID
 */
void Fasta::unpack_hS(const unpackfa_s& upkStruct, byte threadID) {
  unpackFP_t unpackHdr = upkStruct.unpackHdrFP;  // Function pointer
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(std::format("{}{}", UPK_FNAME, static_cast<unsigned>(threadID)),
                        std::ios_base::app);
  std::string upkhdrOut, upkSeqOut;
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read the file from this position
    // Take a chunk of decrypted file
    std::string decText;
    decText.reserve(chunkSize);
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFA.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFA.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }
    // todo
    content += std::format("{}{}\n", THR_ID_HDR, static_cast<unsigned>(threadID));
    do {
      if (*i == (char)253) {  // Hdr
        (this->*unpackHdr)(upkhdrOut, ++i, upkStruct.hdrUnpack);
        content += std::format(">{}\n", upkhdrOut);
      } else {  // Seq
        unpack_seq(upkSeqOut, i);
        content += std::format("{}\n", upkSeqOut);
      }
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) {
          chunkSizeStr += c;
        }

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= IO_BUFFER_SIZE) {
      write_content();
      content.clear();
      content.reserve(IO_BUFFER_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}

/**
 * @brief Unpack: large header
 * @param upkStruct Unpack structure
 * @param threadID Thread ID
 */
void Fasta::unpack_hL(const unpackfa_s& upkStruct, byte threadID) {
  pos_t begPos = upkStruct.begPos;
  u64 chunkSize = upkStruct.chunkSize;
  std::ifstream in(DEC_FNAME);
  std::ofstream upkfile(std::format("{}{}", UPK_FNAME, static_cast<unsigned>(threadID)),
                        std::ios_base::app);
  std::string upkHdrOut, upkSeqOut;
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { upkfile << content; };

  while (in.peek() != EOF) {
    char c;
    in.seekg(begPos);  // Read the file from this position
    // Take a chunk of decrypted file
    std::string decText;
    decText.reserve(chunkSize);
    for (u64 u = chunkSize; u--;) {
      in.get(c);
      decText += c;
    }
    auto i = decText.begin();
    pos_t endPos = in.tellg();  // Set the end position

    // Unshuffle
    if (shuffled) {
      mutxFA.lock();  //----------------------------------------------------
      if (verbose && shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxFA.unlock();  //--------------------------------------------------

      unshuffle(i, chunkSize);
    }

    content += std::format("{}{}\n", THR_ID_HDR, static_cast<unsigned>(threadID));
    do {
      if (*i == (char)253) {  // Hdr
        unpack_large(upkHdrOut, ++i, upkStruct.XChar_hdr, upkStruct.hdrUnpack);
        content += std::format(">{}\n", upkHdrOut);
      } else {  // Seq
        unpack_seq(upkSeqOut, i);
        content += std::format("{}\n", upkSeqOut);
      }
    } while (++i != decText.end());  // If trouble: change "!=" to "<"

    // Update the chunk size and positions (beg & end)
    for (byte t = n_threads; t--;) {
      in.seekg(endPos);
      in.get(c);
      if (c == (char)253) {
        std::string chunkSizeStr;
        while (in.get(c) && c != (char)254) {
          chunkSizeStr += c;
        }

        chunkSize = stoull(chunkSizeStr);
        begPos = in.tellg();
        endPos = begPos + (pos_t)chunkSize;
      }
    }

    if (content.size() >= IO_BUFFER_SIZE) {
      write_content();
      content.clear();
      content.reserve(IO_BUFFER_SIZE);
    }
  }
  write_content();

  upkfile.close();
  in.close();
}
