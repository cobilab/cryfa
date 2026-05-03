// SPDX-FileCopyrightText: 2026 Morteza Hosseini
// SPDX-License-Identifier: GPL-3.0-only

/**
 * @file endecrypto.cpp
 * @brief Encryption/Decryption
 */

#include "endecrypto.hpp"

#include <algorithm>
#include <array>
#include <cmath>  // std::pow
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>  // setw, std::setprecision
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include "assert.hpp"
#include "file.hpp"
#include "time.hpp"
using namespace cryfa;

std::mutex mutxEnDe;

namespace {
constexpr u16 INVALID_RANK = std::numeric_limits<u16>::max();

struct DenseLookup {
  std::string alphabet;
  bool with_extra = false;
  u16 base = 0;
  u16 extra_rank = 0;
  std::array<u16, 256> rank{};
};

auto build_dense_lookup(const std::string& alphabet, bool with_extra) -> DenseLookup {
  DenseLookup lookup;
  lookup.alphabet = alphabet;
  lookup.with_extra = with_extra;
  lookup.base = static_cast<u16>(alphabet.size() + (with_extra ? 1 : 0));
  lookup.extra_rank = static_cast<u16>(alphabet.size());
  lookup.rank.fill(INVALID_RANK);

  for (u16 i = 0; i != alphabet.size(); ++i) {
    lookup.rank[(byte)alphabet[i]] = i;
  }
  if (with_extra) {
    lookup.rank[(byte)(alphabet.back() + 1)] = lookup.extra_rank;
  }

  return lookup;
}

auto dense_lookup(const std::string& alphabet, bool with_extra = false) -> const DenseLookup& {
  thread_local std::vector<DenseLookup> cache;
  for (const DenseLookup& lookup : cache) {
    if (lookup.with_extra == with_extra && lookup.alphabet == alphabet) {
      return lookup;
    }
  }

  cache.push_back(build_dense_lookup(alphabet, with_extra));
  return cache.back();
}

auto checked_rank(const DenseLookup& lookup, char c) -> u16 {
  const u16 rank = lookup.rank[(byte)c];
  if (rank == INVALID_RANK) {
    error(std::format("symbol \"{}\" not found!", c));
  }
  return rank;
}

auto tuple_index(const DenseLookup& lookup, const char* tuple, size_t width) -> u16 {
  u64 index = 0;
  for (size_t i = 0; i != width; ++i) {
    index = index * lookup.base + checked_rank(lookup, tuple[i]);
  }
  return static_cast<u16>(index);
}

auto large_tuple_index(const DenseLookup& lookup, char s0, char s1, char s2, bool& first_not_in,
                       bool& second_not_in, bool& third_not_in) -> u16 {
  auto rank_or_extra = [&](char c, bool& not_in) {
    const u16 rank = lookup.rank[(byte)c];
    not_in = (rank == INVALID_RANK);
    return not_in ? lookup.extra_rank : rank;
  };

  const u16 r0 = rank_or_extra(s0, first_not_in);
  const u16 r1 = rank_or_extra(s1, second_not_in);
  const u16 r2 = rank_or_extra(s2, third_not_in);
  return static_cast<u16>((r0 * lookup.base + r1) * lookup.base + r2);
}

auto dna_rank_or_x(char c, bool& not_in) -> byte {
  not_in = false;
  switch (c) {
    case 'A':
      return 0;
    case 'C':
      return 1;
    case 'G':
      return 2;
    case 'T':
      return 3;
    case 'N':
      return 4;
    default:
      not_in = true;
      return 5;
  }
}

void append_penalty_tail(std::string& packed, const std::string& input, size_t pos) {
  for (; pos != input.size(); ++pos) {
    packed += (char)255;
    packed += input[pos];
  }
}
}  // namespace

/**
 * @brief Build a hash table
 * @param[out] map Hash table
 * @param strIn The string including the keys
 * @param keyLen Length of the keys
 */
void EnDecrypto::build_hash_tbl(htbl_t& map, const std::string& strIn, short keyLen) {
  u64 elementNo = 0;
  std::string element;
  element.reserve((unsigned long)keyLen);
  map.clear();
  map.reserve((u64)std::pow(strIn.size(), keyLen));

  switch (keyLen) {
    case 3:
      LOOP3(i, j, k, strIn) {
        element = i;
        element += j;
        element += k;
        map.insert(make_pair(element, elementNo++));
        ////      map.insert({element, elementNo++});
        ////      map[element] = elementNo++;
      }
      break;

    case 2:
      LOOP2(i, j, strIn) {
        element = i;
        element += j;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 1:
      LOOP(i, strIn) {
        element = i;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 5:
      LOOP5(i, j, k, l, m, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 7:
      LOOP7(i, j, k, l, m, n, o, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        element += o;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 4:
      LOOP4(i, j, k, l, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 6:
      LOOP6(i, j, k, l, m, n, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    case 8:
      LOOP8(i, j, k, l, m, n, o, p, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        element += o;
        element += p;
        map.insert(make_pair(element, elementNo++));
      }
      break;

    default:
      break;
  }
}

/**
 * @brief Build a table for unpacking
 * @param[out] unpack Table (vector of strings)
 * @param strIn The string including the keys
 * @param keyLen Length of the keys
 */
void EnDecrypto::build_unpack_tbl(std::vector<std::string>& unpack, const std::string& strIn,
                                  u16 keyLen) {
  std::string element;
  element.reserve(keyLen);
  unpack.clear();
  unpack.reserve((u64)std::pow(strIn.size(), keyLen));

  switch (keyLen) {
    case 3:
      LOOP3(i, j, k, strIn) {
        element = i;
        element += j;
        element += k;
        unpack.push_back(element);
      }
      break;

    case 2:
      LOOP2(i, j, strIn) {
        element = i;
        element += j;
        unpack.push_back(element);
      }
      break;

    case 1:
      LOOP(i, strIn) {
        element = i;
        unpack.push_back(element);
      }
      break;

    case 5:
      LOOP5(i, j, k, l, m, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        unpack.push_back(element);
      }
      break;

    case 7:
      LOOP7(i, j, k, l, m, n, o, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        element += o;
        unpack.push_back(element);
      }
      break;

    case 4:
      LOOP4(i, j, k, l, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        unpack.push_back(element);
      }
      break;

    case 6:
      LOOP6(i, j, k, l, m, n, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        unpack.push_back(element);
      }
      break;

    case 8:
      LOOP8(i, j, k, l, m, n, o, p, strIn) {
        element = i;
        element += j;
        element += k;
        element += l;
        element += m;
        element += n;
        element += o;
        element += p;
        unpack.push_back(element);
      }
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate each 3 DNA bases in 1 byte. Reduction: ~2/3
 * @param[out] packedSeq Packed sequence
 * @param seq Sequence
 */
void EnDecrypto::pack_seq(std::string& packedSeq, const std::string& seq) {
  size_t pos = 0;
  const size_t tuple_limit = seq.size() - (seq.size() % 3);

  for (; pos != tuple_limit; pos += 3) {
    const char s0 = seq[pos];
    const char s1 = seq[pos + 1];
    const char s2 = seq[pos + 2];
    bool firstNotIn, secondNotIn, thirdNotIn;

    const byte r0 = dna_rank_or_x(s0, firstNotIn);
    const byte r1 = dna_rank_or_x(s1, secondNotIn);
    const byte r2 = dna_rank_or_x(s2, thirdNotIn);
    packedSeq += static_cast<char>((r0 * 6 + r1) * 6 + r2);

    if (firstNotIn) {
      packedSeq += s0;
    }
    if (secondNotIn) {
      packedSeq += s1;
    }
    if (thirdNotIn) {
      packedSeq += s2;
    }
  }

  append_penalty_tail(packedSeq, seq, pos);
}

/**
 * @brief Encapsulate 3 header symbols in 2 bytes, when # >= 40.
 *        -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed Packed header
 * @param strIn Header
 * @param map Hash table
 */
void EnDecrypto::pack_hL_fa_fq(std::string& packed, const std::string& strIn, const htbl_t& map) {
  pack_large(packed, strIn, Hdrs, map);
}

/**
 * @brief Encapsulate 3 quality score symbols in 2 bytes, when # >= 40.
 *        -- FASTQ. Reduction ~1/3
 * @param[out] packed Packed qulity scores
 * @param strIn Quality scores
 * @param map Hash table
 */
void EnDecrypto::pack_qL_fq(std::string& packed, const std::string& strIn, const htbl_t& map) {
  pack_large(packed, strIn, QSs, map);
}

/**
 * @brief Encapsulate 3 header/quality score symbols in 2 bytes, when # >= 40
 *        -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed Packed qulity scores
 * @param strIn Input header/quality score
 * @param hdrQs Collection of headers/quality scores
 * @param map Hash table
 */
inline void EnDecrypto::pack_large(std::string& packed, const std::string& strIn,
                                   const std::string& hdrQs, const htbl_t& map) {
  (void)map;
  const DenseLookup& lookup = dense_lookup(hdrQs, true);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 3);

  for (; pos != tuple_limit; pos += 3) {
    const char s0 = strIn[pos];
    const char s1 = strIn[pos + 1];
    const char s2 = strIn[pos + 2];
    bool firstNotIn, secondNotIn, thirdNotIn;

    const u16 shortTuple =
        large_tuple_index(lookup, s0, s1, s2, firstNotIn, secondNotIn, thirdNotIn);
    packed += (unsigned char)(shortTuple >> 8);    // Left byte
    packed += (unsigned char)(shortTuple & 0xFF);  // Right byte

    if (firstNotIn) {
      packed += s0;
    }
    if (secondNotIn) {
      packed += s1;
    }
    if (thirdNotIn) {
      packed += s2;
    }
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 3 symbols in 2 bytes, when 16 <= # <= 39. Reduction ~1/3
 * @param[out] packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_3to2(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 3);

  for (; pos != tuple_limit; pos += 3) {
    const u16 shortTuple = tuple_index(lookup, strIn.data() + pos, 3);
    packed += (byte)(shortTuple >> 8);    // Left byte
    packed += (byte)(shortTuple & 0xFF);  // Right byte
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 2 symbols in 1 byte, when 7 <= # <= 15. Reduction ~1/2
 * @param[out] packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_2to1(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 2);

  for (; pos != tuple_limit; pos += 2) {
    packed += static_cast<char>(tuple_index(lookup, strIn.data() + pos, 2));
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 3 symbols in 1 byte, when # = 4, 5, 6. Reduction ~2/3
 * @param packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_3to1(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 3);

  for (; pos != tuple_limit; pos += 3) {
    packed += static_cast<char>(tuple_index(lookup, strIn.data() + pos, 3));
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 5 symbols in 1 byte, when # = 3. Reduction ~4/5
 * @param[out] packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_5to1(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 5);

  for (; pos != tuple_limit; pos += 5) {
    packed += static_cast<char>(tuple_index(lookup, strIn.data() + pos, 5));
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 7 symbols in 1 byte, when # = 2. Reduction ~6/7
 * @param[out] packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_7to1(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  size_t pos = 0;
  const size_t tuple_limit = strIn.size() - (strIn.size() % 7);

  for (; pos != tuple_limit; pos += 7) {
    packed += static_cast<char>(tuple_index(lookup, strIn.data() + pos, 7));
  }

  append_penalty_tail(packed, strIn, pos);
}

/**
 * @brief Encapsulate 1 symbol in 1 byte, when # = 1.
 * @param[out] packed Packed string
 * @param strIn Input string
 * @param map Hash table
 */
void EnDecrypto::pack_1to1(std::string& packed, const std::string& strIn, const htbl_t& map) {
  const DenseLookup& lookup = dense_lookup((&map == &QsMap) ? QSs : Hdrs);
  for (char c : strIn) {
    packed += static_cast<char>(checked_rank(lookup, c));
  }
}

/**
 * @brief Penalty symbol
 * @param c Input char
 * @return Input char or (char)10='\\n'
 */
char EnDecrypto::penalty_sym(char c) const {
  const char lookupTable[2] = {c, (char)10};
  return lookupTable[c == (char)254 || c == (char)252];

  //  // More readable; Perhaps slower, because of conditional branch
  //  return (c != (char) 254 && c != (char) 252) ? c : (char) 10;
}

/**
 * @brief Unpack by reading 2 byte by 2 byte, when # > 39
 * @param[out] out Unpacked string
 * @param i Input string iterator
 * @param XChar Extra character for unpacking
 * @param unpack Table for unpacking
 */
void EnDecrypto::unpack_large(std::string& out, std::string::iterator& i, char XChar,
                              const std::vector<std::string>& unpack) {
  out.clear();

  while (*i != (char)254) {
    // Hdr len not multiple of keyLen
    if (*i == (char)255) {
      out += penalty_sym(*(i + 1));
      i += 2;
    } else {
      const auto leftB = (byte)*i;
      const auto rightB = (byte) * (i + 1);
      const u16 doubleB = leftB << 8 | rightB;  // Join two bytes

      const std::string& tpl = unpack[doubleB];

      if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] != XChar) {  // ...
        out += tpl;
        i += 2;
      } else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] != XChar) {  // X..
        out += penalty_sym(*(i + 2));
        out += tpl[1];
        out += tpl[2];
        i += 3;
      } else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] != XChar) {  // .X.
        out += tpl[0];
        out += penalty_sym(*(i + 2));
        out += tpl[2];
        i += 3;
      } else if (tpl[0] == XChar && tpl[1] == XChar && tpl[2] != XChar) {  // XX.
        out += penalty_sym(*(i + 2));
        out += penalty_sym(*(i + 3));
        out += tpl[2];
        i += 4;
      } else if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] == XChar) {  // ..X
        out += tpl[0];
        out += tpl[1];
        out += penalty_sym(*(i + 2));
        i += 3;
      } else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] == XChar) {  // X.X
        out += penalty_sym(*(i + 2));
        out += tpl[1];
        out += penalty_sym(*(i + 3));
        i += 4;
      } else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] == XChar) {  // .XX
        out += tpl[0];
        out += penalty_sym(*(i + 2));
        out += penalty_sym(*(i + 3));
        i += 4;
      } else {
        out += penalty_sym(*(i + 2));
        out += penalty_sym(*(i + 3));  // XXX
        out += penalty_sym(*(i + 4));
        i += 5;
      }
    }
  }
}

/**
 * @brief Unpack by reading 2 byte by 2 byte
 * @param[out] out Unpacked string
 * @param i Input string iterator
 * @param unpack Table for unpacking
 */
void EnDecrypto::unpack_2B(std::string& out, std::string::iterator& i,
                           const std::vector<std::string>& unpack) {
  out.clear();

  for (; *i != (char)254; i += 2) {
    // Hdr len not multiple of keyLen
    if (*i == (char)255) {
      out += penalty_sym(*(i + 1));
    } else {
      const auto leftB = (byte)*i;
      const auto rightB = (byte) * (i + 1);
      const u16 doubleB = leftB << 8 | rightB;  // Join two bytes

      out += unpack[doubleB];
    }
  }
}

/**
 * @brief Unpack by reading 1 byte by 1 byte
 * @param[out] out Unpacked string
 * @param i Input string iterator
 * @param unpack Table for unpacking
 */
void EnDecrypto::unpack_1B(std::string& out, std::string::iterator& i,
                           const std::vector<std::string>& unpack) {
  out.clear();

  for (; *i != (char)254; ++i) {
    // Hdr len not multiple of keyLen
    if (*i == (char)255) {
      out += penalty_sym(*(++i));
    } else {
      out += unpack[(byte)*i];
    }
  }
}

/**
 * @brief Unpack 1 byte to 3 DNA bases
 * @param[out] out DNA bases
 * @param i Input string iterator
 */
void EnDecrypto::unpack_seq(std::string& out, std::string::iterator& i) {
  out.clear();

  for (; *i != (char)254; ++i) {
    if (*i == (char)255) {  // Seq len not multiple of 3
      out += penalty_sym(*(++i));
    } else {
      const std::string& tpl = DNA_UNPACK[(byte)*i];

      if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] != 'X') {  // ...
        out += tpl;
      }
      // Using just one 'out' makes trouble
      else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] != 'X') {  // X..
        out += penalty_sym(*(++i));
        out += tpl[1];
        out += tpl[2];
      } else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] != 'X') {  // .X.
        out += tpl[0];
        out += penalty_sym(*(++i));
        out += tpl[2];
      } else if (tpl[0] == 'X' && tpl[1] == 'X' && tpl[2] != 'X') {  // XX.
        out += penalty_sym(*(++i));
        out += penalty_sym(*(++i));
        out += tpl[2];
      } else if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] == 'X') {  // ..X
        out += tpl[0];
        out += tpl[1];
        out += penalty_sym(*(++i));
      } else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] == 'X') {  // X.X
        out += penalty_sym(*(++i));
        out += tpl[1];
        out += penalty_sym(*(++i));
      } else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] == 'X') {  // .XX
        out += tpl[0];
        out += penalty_sym(*(++i));
        out += penalty_sym(*(++i));
      } else {
        out += penalty_sym(*(++i));
        out += penalty_sym(*(++i));  // XXX
        out += penalty_sym(*(++i));
      }
    }
  }
}

/**
 * @brief Shuffle a file (not FASTA/FASTQ)
 */
void EnDecrypto::shuffle_file() {
  std::cerr << "\"" << file_name(in_file) << "\" isn't FASTA/FASTQ. We just encrypt it.\n";

  if (!stop_shuffle) {
    const auto start = now();  // Start timer
    std::vector<std::thread> arrThread(n_threads);

    // Distribute file among threads, for shuffling
    for (byte t = 0; t != n_threads; ++t) {
      arrThread[t] = std::thread(&EnDecrypto::shuffle_block, this, t);
    }
    for (auto& thr : arrThread) {
      if (thr.joinable()) thr.join();
    }

    // Join partially shuffled files
    join_shuffled_files();

    const auto finish = now();  // Stop timer
    std::cerr << "\r" << bold("[+]") << " Shuffling done in " << hms(finish - start);
  } else {
    std::ifstream inFile(in_file);
    std::ofstream pckdFile(PCKD_FNAME);

    pckdFile << (char)125 << (!stop_shuffle ? (char)128 : (char)129);
    pckdFile << inFile.rdbuf();

    inFile.close();
    pckdFile.close();
  }

  // Cout encrypted content
  encrypt();
}

/**
 * @brief Shuffle a block of file
 * @param threadID Thread ID
 */
void EnDecrypto::shuffle_block(byte threadID) {
  std::ifstream in(in_file);
  std::ofstream shfile(std::format("{}{}", SH_FNAME, static_cast<unsigned>(threadID)),
                       std::ios_base::app);
  // Characters ignored at the beginning
  in.ignore((std::streamsize)(threadID * CHUNK_TARGET_SIZE));

  for (char c; in.peek() != EOF;) {
    std::string context;
    context.reserve(CHUNK_TARGET_SIZE);
    for (u64 bs = CHUNK_TARGET_SIZE; bs--;) {
      if (in.get(c)) {
        context += c;
      }
    }

    // Shuffle
    if (!stop_shuffle) {
      mutxEnDe.lock();  //--------------------------------------------------
      if (shuffInProg) {
        std::cerr << bold("[+]") << " Shuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxEnDe.unlock();  //------------------------------------------------

      shuffle(context);
    }

    // Write header containing threadID for each partially shuffled file
    shfile << std::format("{}{}\n", THR_ID_HDR, static_cast<unsigned>(threadID));
    shfile << context << '\n';

    // Ignore to go to the next related chunk
    in.ignore((std::streamsize)((n_threads - 1) * CHUNK_TARGET_SIZE));
  }
  shfile.close();
}

/**
 * @brief Unshuffle a file (not FASTA/FASTQ)
 */
void EnDecrypto::unshuffle_file() {
  std::ifstream in(DEC_FNAME);
  in.ignore(1);
  char c;
  in.get(c);
  if (c == (char)128) {
    in.close();

    const auto start = now();  // Start timer
    std::vector<std::thread> arrThread(n_threads);

    // Distribute file among threads, for unshuffling
    for (byte t = 0; t != n_threads; ++t) {
      arrThread[t] = std::thread(&EnDecrypto::unshuffle_block, this, t);
    }
    for (auto& thr : arrThread) {
      if (thr.joinable()) {
        thr.join();
      }
    }

    // Delete decrypted file
    std::remove(DEC_FNAME.c_str());

    // Join partially unshuffled files
    join_unshuffled_files();

    const auto finish = now();  // Stop timer
    std::cerr << "\r" << bold("[+]") << " Unshuffling done in " << hms(finish - start);
  } else if (c == (char)129) {
    std::cout << in.rdbuf();

    in.close();
    std::remove(DEC_FNAME.c_str());
  } else {
    std::cerr << bold("Error:", "red") << " file corrupted.";
    in.close();
  }
}

/**
 * @brief Unshuffle a block of file
 * @param threadID Thread ID
 */
void EnDecrypto::unshuffle_block(byte threadID) {
  std::ifstream in(DEC_FNAME);
  std::ofstream ushfile(std::format("{}{}", USH_FNAME, static_cast<unsigned>(threadID)),
                        std::ios_base::app);

  // filetype char (125) + shuffed (128) + characters ignored at the beginning
  in.ignore((std::streamsize)(2 + threadID * CHUNK_TARGET_SIZE));

  for (char c; in.peek() != EOF;) {
    std::string unshText;
    unshText.reserve(CHUNK_TARGET_SIZE);
    for (u64 bs = CHUNK_TARGET_SIZE; bs--;) {
      if (in.get(c)) {
        unshText += c;
      }
    }

    auto i = unshText.begin();

    // Unshuffle
    if (shuffled) {
      mutxEnDe.lock();  //--------------------------------------------------
      if (shuffInProg) {
        std::cerr << bold("[+]") << " Unshuffling ...";
        shuffle_timer = now();
      }
      shuffInProg = false;
      mutxEnDe.unlock();  //------------------------------------------------

      unshuffle(i, unshText.size());
    }

    // Write header containing threadID for each partially unshuffled file
    ushfile << std::format("{}{}\n", THR_ID_HDR, static_cast<unsigned>(threadID));
    ushfile << unshText << '\n';

    // Ignore to go to the next related chunk
    in.ignore((std::streamsize)((n_threads - 1) * CHUNK_TARGET_SIZE));
  }

  ushfile.close();
  in.close();
}

/**
 * @brief Join partially packed files
 * @param headers Headers
 * @param qscores Quality scores
 * @param fT File type
 * @param justPlus If the third line of FASTQ contains only the '+' char
 */
void EnDecrypto::join_packed_files(const std::string& headers, const std::string& qscores, char fT,
                                   bool justPlus) const {
  byte t;  // For threads
  std::vector<std::ifstream> pkFile(n_threads);
  std::ofstream pckdFile(PCKD_FNAME);  // Packed file
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { pckdFile << content; };

  switch (fT) {
    case 'A':
      content += (char)127;
      break;  // Fasta
    case 'Q':
      content += (char)126;
      break;  // Fastq
    default:
      break;
  }
  content += (!stop_shuffle ? (char)128 : (char)129);
  content += headers;
  content += (char)254;  // To detect headers in decryptor
  if (fT == 'Q') {
    content += qscores;
    content += (justPlus ? (char)253 : '\n');
  }

  // Input files
  for (t = n_threads; t--;) {
    pkFile[t].open(std::format("{}{}", PK_FNAME, static_cast<unsigned>(t)));
  }

  std::string line;
  bool prevLineNotThrID;  // If previous line was "THR=" or not
  while (!pkFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      prevLineNotThrID = false;

      while (std::getline(pkFile[t], line).good() &&
             line != std::format("{}{}", THR_ID_HDR, static_cast<unsigned>(t))) {
        if (prevLineNotThrID) {
          content += '\n';
        }
        content += line;

        if (content.size() >= IO_BUFFER_SIZE) {
          write_content();
          content.clear();
          content.reserve(IO_BUFFER_SIZE);
        }

        prevLineNotThrID = true;
      }
    }
  }
  content += (char)252;
  write_content();

  // Close/delete input/output files
  pckdFile.close();
  for (t = n_threads; t--;) {
    pkFile[t].close();
    std::string pkFileName = std::format("{}{}", PK_FNAME, static_cast<unsigned>(t));
    std::remove(pkFileName.c_str());
  }
}

/**
 * @brief Join partially unpacked files
 */
void EnDecrypto::join_unpacked_files() const {
  byte t;  // For threads
  std::vector<std::ifstream> upkdFile(n_threads);
  for (t = n_threads; t--;) {
    upkdFile[t].open(std::format("{}{}", UPK_FNAME, static_cast<unsigned>(t)));
  }
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { std::cout << content; };

  bool prevLineNotThrID;  // If previous line was "THRD=" or not
  while (!upkdFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      prevLineNotThrID = false;

      for (std::string line; std::getline(upkdFile[t], line).good() &&
                             line != std::format("{}{}", THR_ID_HDR, static_cast<unsigned>(t));) {
        if (prevLineNotThrID) {
          content += '\n';
        }
        content += line;

        if (content.size() >= IO_BUFFER_SIZE) {
          write_content();
          content.clear();
          content.reserve(IO_BUFFER_SIZE);
        }

        prevLineNotThrID = true;
      }

      if (prevLineNotThrID) {
        content += '\n';
      }
    }
  }
  write_content();

  // Close/delete input/output files
  for (t = n_threads; t--;) {
    upkdFile[t].close();
    std::string upkdFileName = std::format("{}{}", UPK_FNAME, static_cast<unsigned>(t));
    std::remove(upkdFileName.c_str());
  }
}

/**
 * @brief Join partially shuffled files
 */
void EnDecrypto::join_shuffled_files() const {
  std::vector<std::ifstream> shFile(n_threads);
  std::ofstream shdFile(PCKD_FNAME);  // Output Shuffled file
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { shdFile << content; };

  content += (char)125;
  content += (!stop_shuffle ? (char)128 : (char)129);

  // Input files
  for (byte t = n_threads; t--;) {
    shFile[t].open(std::format("{}{}", SH_FNAME, static_cast<unsigned>(t)));
  }

  while (!shFile[0].eof()) {
    for (byte t = 0; t != n_threads; ++t) {
      bool prevLineNotThrID = false;  // If previous line was "THR=" or not

      for (std::string line; std::getline(shFile[t], line).good() &&
                             line != std::format("{}{}", THR_ID_HDR, static_cast<unsigned>(t));) {
        if (prevLineNotThrID) {
          content += '\n';
        }
        content += line;

        if (content.size() >= IO_BUFFER_SIZE) {
          write_content();
          content.clear();
          content.reserve(IO_BUFFER_SIZE);
        }

        prevLineNotThrID = true;
      }
    }
  }
  write_content();

  // Close/delete input/output files
  shdFile.close();
  for (byte t = n_threads; t--;) {
    shFile[t].close();
    std::string shFileName = std::format("{}{}", SH_FNAME, static_cast<unsigned>(t));
    std::remove(shFileName.c_str());
  }
}

/**
 * @brief Join partially unshuffled files
 */
void EnDecrypto::join_unshuffled_files() const {
  byte t;  // For threads
  std::vector<std::ifstream> ushdFile(n_threads);
  for (t = n_threads; t--;) {
    ushdFile[t].open(std::format("{}{}", USH_FNAME, static_cast<unsigned>(t)));
  }
  std::string content;
  content.reserve(IO_BUFFER_SIZE);
  auto write_content = [&]() { std::cout << content; };

  while (!ushdFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      bool prevLineNotThrID = false;  // If previous line was "THR=" or not

      for (std::string line; std::getline(ushdFile[t], line).good() &&
                             line != std::format("{}{}", THR_ID_HDR, static_cast<unsigned>(t));) {
        if (prevLineNotThrID) {
          content += '\n';
        }
        content += line;

        if (content.size() >= IO_BUFFER_SIZE) {
          write_content();
          content.clear();
          content.reserve(IO_BUFFER_SIZE);
        }

        prevLineNotThrID = true;
      }
    }
  }
  write_content();

  // Close/delete input/output files
  for (t = n_threads; t--;) {
    ushdFile[t].close();
    std::string ushdFileName = std::format("{}{}", USH_FNAME, static_cast<unsigned>(t));
    std::remove(ushdFileName.c_str());
  }
}
