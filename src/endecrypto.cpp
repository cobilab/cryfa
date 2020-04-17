/**
 * @file      endecrypto.cpp
 * @brief     Encryption/Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include "endecrypto.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>  // setw, std::setprecision
#include <mutex>
#include <thread>

#include "assert.hpp"
#include "file.hpp"
#include "time.hpp"
using namespace cryfa;

std::mutex mutxEnDe; /**< @brief Mutex */

/**
 * @brief Build a hash table
 * @param[out] map     Hash table
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::build_hash_tbl(htbl_t& map, const std::string& strIn,
                                short keyLen) {
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
 * @param[out] unpack  Table (vector of strings)
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::build_unpack_tbl(std::vector<std::string>& unpack,
                                  const std::string& strIn, u16 keyLen) {
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
 * @brief  Index of each DNA bases pack
 * @param  key  Key
 * @return Value (based on the idea of key-value in a hash table)
 */
byte EnDecrypto::dna_pack_idx(const std::string& key) {
  const auto found = DNA_MAP.find(key);
  if (found == DNA_MAP.end()) error("key \"" + key + "\" not found!");

  return (byte)found->second;
}

/**
 * @brief  Index of each pack, when # > 39
 * @param  key  Key
 * @param  map  Hash table
 * @return Value (based on the idea of key-value in a hash table)
 */
u16 EnDecrypto::large_pack_idx(const std::string& key, const htbl_t& map) {
  const auto found = map.find(key);
  if (found == map.end()) error("key \"" + key + "\" not found!");

  return (u16)found->second;
}

/**
 * @brief Encapsulate each 3 DNA bases in 1 byte. Reduction: ~2/3
 * @param[out] packedSeq  Packed sequence
 * @param[in]  seq        Sequence
 */
void EnDecrypto::pack_seq(std::string& packedSeq, const std::string& seq) {
  auto i = seq.begin();

  for (auto iEnd = seq.end() - 2; i < iEnd; i += 3) {
    char s0 = *i, s1 = *(i + 1), s2 = *(i + 2);

    std::string tuple;
    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    tuple += (firstNotIn = (s0 != 'A' && s0 != 'C' && s0 != 'G' && s0 != 'T' &&
                            s0 != 'N'))
                 ? 'X'
                 : s0;
    tuple += (secondNotIn = (s1 != 'A' && s1 != 'C' && s1 != 'G' && s1 != 'T' &&
                             s1 != 'N'))
                 ? 'X'
                 : s1;
    tuple += (thirdNotIn = (s2 != 'A' && s2 != 'C' && s2 != 'G' && s2 != 'T' &&
                            s2 != 'N'))
                 ? 'X'
                 : s2;

    packedSeq += dna_pack_idx(tuple);
    if (firstNotIn) packedSeq += s0;
    if (secondNotIn) packedSeq += s1;
    if (thirdNotIn) packedSeq += s2;
  }

  // If seq len isn't multiple of 3, add (char) 255 before each sym
  switch (seq.length() % 3) {
    case 1:
      packedSeq += (char)255;
      packedSeq += *i;
      break;

    case 2:
      packedSeq += (char)255;
      packedSeq += *i;
      packedSeq += (char)255;
      packedSeq += *(i + 1);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 3 header symbols in 2 bytes, when # >= 40.
 *        -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed  Packed header
 * @param[in]  strIn   Header
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_hL_fa_fq(std::string& packed, const std::string& strIn,
                               const htbl_t& map) {
  pack_large(packed, strIn, Hdrs, map);
}

/**
 * @brief Encapsulate 3 quality score symbols in 2 bytes, when # >= 40.
 *         -- FASTQ. Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Quality scores
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_qL_fq(std::string& packed, const std::string& strIn,
                            const htbl_t& map) {
  pack_large(packed, strIn, QSs, map);
}

/**
 * @brief Encapsulate 3 header/quality score symbols in 2 bytes, when # >= 40
 *        -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Input header/quality score
 * @param[in]  hdrQs   Collection of headers/quality scores
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::pack_large(std::string& packed,
                                   const std::string& strIn,
                                   const std::string& hdrQs,
                                   const htbl_t& map) {
  // ASCII char after the last char in QUALITY_SCORES std::string
  const auto XChar = (char)(hdrQs.back() + 1);
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 2; i < iEnd; i += 3) {
    char s0 = *i, s1 = *(i + 1), s2 = *(i + 2);

    std::string tuple;
    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    tuple = (firstNotIn = (hdrQs.find(s0) == std::string::npos)) ? XChar : s0;
    tuple += (secondNotIn = (hdrQs.find(s1) == std::string::npos)) ? XChar : s1;
    tuple += (thirdNotIn = (hdrQs.find(s2) == std::string::npos)) ? XChar : s2;

    u16 shortTuple = large_pack_idx(tuple, map);
    packed += (unsigned char)(shortTuple >> 8);    // Left byte
    packed += (unsigned char)(shortTuple & 0xFF);  // Right byte

    if (firstNotIn) packed += s0;
    if (secondNotIn) packed += s1;
    if (thirdNotIn) packed += s2;
  }

  // If len isn't multiple of 3, add (char) 255 before each sym
  switch (strIn.length() % 3) {
    case 1:
      packed += (char)255;
      packed += *i;
      break;

    case 2:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 3 symbols in 2 bytes, when 16 <= # <= 39. Reduction ~1/3
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_3to2(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 2; i < iEnd; i += 3) {
    std::string tuple;
    tuple.reserve(3);
    tuple = *i;
    tuple += *(i + 1);
    tuple += *(i + 2);
    u16 shortTuple = (u16)map.find(tuple)->second;
    packed += (byte)(shortTuple >> 8);    // Left byte
    packed += (byte)(shortTuple & 0xFF);  // Right byte
  }

  // If len isn't multiple of 3, add (char) 255 before each sym
  switch (strIn.length() % 3) {
    case 1:
      packed += (char)255;
      packed += *i;
      break;

    case 2:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 2 symbols in 1 byte, when 7 <= # <= 15. Reduction ~1/2
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_2to1(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 1; i < iEnd; i += 2) {
    std::string tuple;
    tuple.reserve(2);
    tuple = *i;
    tuple += *(i + 1);
    packed += (char)map.find(tuple)->second;
  }

  // If len isn't multiple of 2 (it's odd), add (char) 255 before each sym
  if (strIn.length() & 1) {
    packed += (char)255;
    packed += *i;
  }
}

/**
 * @brief Encapsulate 3 symbols in 1 byte, when # = 4, 5, 6. Reduction ~2/3
 * @param packed  Packed string
 * @param strIn   Input string
 * @param map     Hash table
 */
void EnDecrypto::pack_3to1(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 2; i < iEnd; i += 3) {
    std::string tuple;
    tuple.reserve(3);
    tuple = *i;
    tuple += *(i + 1);
    tuple += *(i + 2);
    packed += (char)map.find(tuple)->second;
  }

  // If len isn't multiple of 3, add (char) 255 before each sym
  switch (strIn.length() % 3) {
    case 1:
      packed += (char)255;
      packed += *i;
      break;

    case 2:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 5 symbols in 1 byte, when # = 3. Reduction ~4/5
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_5to1(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 4; i < iEnd; i += 5) {
    std::string tuple;
    tuple.reserve(5);
    tuple = *i;
    tuple += *(i + 1);
    tuple += *(i + 2);
    tuple += *(i + 3);
    tuple += *(i + 4);
    packed += (char)map.find(tuple)->second;
  }

  // If len isn't multiple of 5, add (char) 255 before each sym
  switch (strIn.length() % 5) {
    case 1:
      packed += (char)255;
      packed += *i;
      break;

    case 2:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      break;

    case 3:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      break;

    case 4:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      packed += (char)255;
      packed += *(i + 3);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 7 symbols in 1 byte, when # = 2. Reduction ~6/7
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_7to1(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  auto i = strIn.begin();

  for (auto iEnd = strIn.end() - 6; i < iEnd; i += 7) {
    std::string tuple;
    tuple.reserve(7);
    tuple = *i;
    tuple += *(i + 1);
    tuple += *(i + 2);
    tuple += *(i + 3);
    tuple += *(i + 4);
    tuple += *(i + 5);
    tuple += *(i + 6);
    packed += (char)map.find(tuple)->second;
  }

  // If len isn't multiple of 7, add (char) 255 before each sym
  switch (strIn.length() % 7) {
    case 1:
      packed += (char)255;
      packed += *i;
      break;

    case 2:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      break;

    case 3:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      break;

    case 4:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      packed += (char)255;
      packed += *(i + 3);
      break;

    case 5:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      packed += (char)255;
      packed += *(i + 3);
      packed += (char)255;
      packed += *(i + 4);
      break;

    case 6:
      packed += (char)255;
      packed += *i;
      packed += (char)255;
      packed += *(i + 1);
      packed += (char)255;
      packed += *(i + 2);
      packed += (char)255;
      packed += *(i + 3);
      packed += (char)255;
      packed += *(i + 4);
      packed += (char)255;
      packed += *(i + 5);
      break;

    default:
      break;
  }
}

/**
 * @brief Encapsulate 1 symbol in 1 byte, when # = 1.
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_1to1(std::string& packed, const std::string& strIn,
                           const htbl_t& map) {
  for (auto i = strIn.begin(), iEnd = strIn.end(); i < iEnd; ++i) {
    std::string single;
    single = *i;
    packed += (char)map.find(single)->second;
  }
}

/**
 * @brief  Penalty symbol
 * @param  c  Input char
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
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  XChar   Extra character for unpacking
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_large(std::string& out, std::string::iterator& i,
                              char XChar,
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

      const std::string tpl = unpack[doubleB];

      if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] != XChar)  // ...
      {
        out += tpl;
        i += 2;
      }

      else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] != XChar)  // X..
      {
        out += penalty_sym(*(i + 2));
        out += tpl[1];
        out += tpl[2];
        i += 3;
      }

      else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] != XChar)  // .X.
      {
        out += tpl[0];
        out += penalty_sym(*(i + 2));
        out += tpl[2];
        i += 3;
      }

      else if (tpl[0] == XChar && tpl[1] == XChar && tpl[2] != XChar)  // XX.
      {
        out += penalty_sym(*(i + 2));
        out += penalty_sym(*(i + 3));
        out += tpl[2];
        i += 4;
      }

      else if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] == XChar)  // ..X
      {
        out += tpl[0];
        out += tpl[1];
        out += penalty_sym(*(i + 2));
        i += 3;
      }

      else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] == XChar)  // X.X
      {
        out += penalty_sym(*(i + 2));
        out += tpl[1];
        out += penalty_sym(*(i + 3));
        i += 4;
      }

      else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] == XChar)  // .XX
      {
        out += tpl[0];
        out += penalty_sym(*(i + 2));
        out += penalty_sym(*(i + 3));
        i += 4;
      }

      else {
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
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_2B(std::string& out, std::string::iterator& i,
                           const std::vector<std::string>& unpack) {
  out.clear();

  for (; *i != (char)254; i += 2) {
    // Hdr len not multiple of keyLen
    if (*i == (char)255)
      out += penalty_sym(*(i + 1));
    else {
      const auto leftB = (byte)*i;
      const auto rightB = (byte) * (i + 1);
      const u16 doubleB = leftB << 8 | rightB;  // Join two bytes

      out += unpack[doubleB];
    }
  }
}

/**
 * @brief Unpack by reading 1 byte by 1 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_1B(std::string& out, std::string::iterator& i,
                           const std::vector<std::string>& unpack) {
  out.clear();

  for (; *i != (char)254; ++i) {
    // Hdr len not multiple of keyLen
    if (*i == (char)255)
      out += penalty_sym(*(++i));
    else
      out += unpack[(byte)*i];
  }
}

/**
 * @brief Unpack 1 byte to 3 DNA bases
 * @param[out] out  DNA bases
 * @param[in]  i    Input string iterator
 */
void EnDecrypto::unpack_seq(std::string& out, std::string::iterator& i) {
  out.clear();

  for (; *i != (char)254; ++i) {
    if (*i == (char)255)  // Seq len not multiple of 3
      out += penalty_sym(*(++i));
    else {
      const std::string tpl = DNA_UNPACK[(byte)*i];

      if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] != 'X')  // ...
      {
        out += tpl;
      }
      // Using just one 'out' makes trouble
      else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] != 'X')  // X..
      {
        out += penalty_sym(*(++i));
        out += tpl[1];
        out += tpl[2];
      }

      else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] != 'X')  // .X.
      {
        out += tpl[0];
        out += penalty_sym(*(++i));
        out += tpl[2];
      }

      else if (tpl[0] == 'X' && tpl[1] == 'X' && tpl[2] != 'X')  // XX.
      {
        out += penalty_sym(*(++i));
        out += penalty_sym(*(++i));
        out += tpl[2];
      }

      else if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] == 'X')  // ..X
      {
        out += tpl[0];
        out += tpl[1];
        out += penalty_sym(*(++i));
      }

      else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] == 'X')  // X.X
      {
        out += penalty_sym(*(++i));
        out += tpl[1];
        out += penalty_sym(*(++i));
      }

      else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] == 'X')  // .XX
      {
        out += tpl[0];
        out += penalty_sym(*(++i));
        out += penalty_sym(*(++i));
      }

      else {
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
  std::cerr << "\"" << file_name(in_file)
            << "\" isn't FASTA/FASTQ. We just encrypt it.\n";

  if (!stop_shuffle) {
    const auto start = now();  // Start timer
    std::thread arrThread[n_threads];

    // Distribute file among threads, for shuffling
    for (byte t = 0; t != n_threads; ++t)
      arrThread[t] = std::thread(&EnDecrypto::shuffle_block, this, t);
    for (auto& thr : arrThread)
      if (thr.joinable()) thr.join();

    // Join partially shuffled files
    join_shuffled_files();

    const auto finish = now();  // Stop timer
    std::cerr << "\r" << bold("[+]") << " Shuffling done in "
              << hms(finish - start);
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
 * @param threadID  Thread ID
 */
void EnDecrypto::shuffle_block(byte threadID) {
  std::ifstream in(in_file);
  std::ofstream shfile(SH_FNAME + std::to_string(threadID), std::ios_base::app);
  // Characters ignored at the beginning
  in.ignore((std::streamsize)(threadID * BLOCK_SIZE));

  for (char c; in.peek() != EOF;) {
    std::string context;
    for (u64 bs = BLOCK_SIZE; bs--;)
      if (in.get(c)) context += c;

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
    shfile << THR_ID_HDR << std::to_string(threadID) << '\n';
    shfile << context << '\n';

    // Ignore to go to the next related chunk
    in.ignore((std::streamsize)((n_threads - 1) * BLOCK_SIZE));
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
    std::thread arrThread[n_threads];

    // Distribute file among threads, for unshuffling
    for (byte t = 0; t != n_threads; ++t)
      arrThread[t] = std::thread(&EnDecrypto::unshuffle_block, this, t);
    for (auto& thr : arrThread)
      if (thr.joinable()) thr.join();

    // Delete decrypted file
    std::remove(DEC_FNAME.c_str());

    // Join partially unshuffled files
    join_unshuffled_files();

    const auto finish = now();  // Stop timer
    std::cerr << "\r" << bold("[+]") << " Unshuffling done in "
              << hms(finish - start);
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
 * @param threadID  Thread ID
 */
void EnDecrypto::unshuffle_block(byte threadID) {
  std::ifstream in(DEC_FNAME);
  std::ofstream ushfile(USH_FNAME + std::to_string(threadID),
                        std::ios_base::app);

  // filetype char (125) + shuffed (128) + characters ignored at the beginning
  in.ignore((std::streamsize)(2 + threadID * BLOCK_SIZE));

  for (char c; in.peek() != EOF;) {
    std::string unshText;
    for (u64 bs = BLOCK_SIZE; bs--;)
      if (in.get(c)) unshText += c;

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
    ushfile << THR_ID_HDR + std::to_string(threadID) << '\n';
    ushfile << unshText << '\n';

    // Ignore to go to the next related chunk
    in.ignore((std::streamsize)((n_threads - 1) * BLOCK_SIZE));
  }

  ushfile.close();
  in.close();
}

/**
 * @brief Join partially packed files
 * @param headers   Headers
 * @param qscores   Quality scores
 * @param fT        File type
 * @param justPlus  If the third line of FASTQ contains only the '+' char
 */
void EnDecrypto::join_packed_files(const std::string& headers,
                                   const std::string& qscores, char fT,
                                   bool justPlus) const {
  byte t;  // For threads
  std::ifstream pkFile[n_threads];
  std::ofstream pckdFile(PCKD_FNAME);  // Packed file
  std::string content;
  content.reserve(BLOCK_SIZE);
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
  for (t = n_threads; t--;) pkFile[t].open(PK_FNAME + std::to_string(t));

  std::string line;
  bool prevLineNotThrID;  // If previous line was "THR=" or not
  while (!pkFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      prevLineNotThrID = false;

      while (std::getline(pkFile[t], line).good() &&
             line != THR_ID_HDR + std::to_string(t)) {
        if (prevLineNotThrID) content += '\n';
        content += line;

        if (content.size() >= BLOCK_SIZE) {
          write_content();
          content.clear();
          content.reserve(BLOCK_SIZE);
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
    std::string pkFileName = PK_FNAME;
    pkFileName += std::to_string(t);
    std::remove(pkFileName.c_str());
  }
}

/**
 * @brief Join partially unpacked files
 */
void EnDecrypto::join_unpacked_files() const {
  byte t;  // For threads
  std::ifstream upkdFile[n_threads];
  for (t = n_threads; t--;) upkdFile[t].open(UPK_FNAME + std::to_string(t));
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { std::cout << content; };

  bool prevLineNotThrID;  // If previous line was "THRD=" or not
  while (!upkdFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      prevLineNotThrID = false;

      for (std::string line; std::getline(upkdFile[t], line).good() &&
                             line != THR_ID_HDR + std::to_string(t);) {
        if (prevLineNotThrID) content += '\n';
        content += line;

        if (content.size() >= BLOCK_SIZE) {
          write_content();
          content.clear();
          content.reserve(BLOCK_SIZE);
        }

        prevLineNotThrID = true;
      }

      if (prevLineNotThrID) content += '\n';
    }
  }
  write_content();

  // Close/delete input/output files
  for (t = n_threads; t--;) {
    upkdFile[t].close();
    std::string upkdFileName = UPK_FNAME;
    upkdFileName += std::to_string(t);
    std::remove(upkdFileName.c_str());
  }
}

/**
 * @brief Join partially shuffled files
 */
void EnDecrypto::join_shuffled_files() const {
  std::ifstream shFile[n_threads];
  std::ofstream shdFile(PCKD_FNAME);  // Output Shuffled file
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { shdFile << content; };

  content += (char)125;
  content += (!stop_shuffle ? (char)128 : (char)129);

  // Input files
  for (byte t = n_threads; t--;) shFile[t].open(SH_FNAME + std::to_string(t));

  while (!shFile[0].eof()) {
    for (byte t = 0; t != n_threads; ++t) {
      bool prevLineNotThrID = false;  // If previous line was "THR=" or not

      for (std::string line; std::getline(shFile[t], line).good() &&
                             line != THR_ID_HDR + std::to_string(t);) {
        if (prevLineNotThrID) content += '\n';
        content += line;

        if (content.size() >= BLOCK_SIZE) {
          write_content();
          content.clear();
          content.reserve(BLOCK_SIZE);
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
    std::string shFileName = SH_FNAME;
    shFileName += std::to_string(t);
    std::remove(shFileName.c_str());
  }
}

/**
 * @brief Join partially unshuffled files
 */
void EnDecrypto::join_unshuffled_files() const {
  byte t;  // For threads
  std::ifstream ushdFile[n_threads];
  for (t = n_threads; t--;) ushdFile[t].open(USH_FNAME + std::to_string(t));
  std::string content;
  content.reserve(BLOCK_SIZE);
  auto write_content = [&]() { std::cout << content; };

  while (!ushdFile[0].eof()) {
    for (t = 0; t != n_threads; ++t) {
      bool prevLineNotThrID = false;  // If previous line was "THR=" or not

      for (std::string line; std::getline(ushdFile[t], line).good() &&
                             line != THR_ID_HDR + std::to_string(t);) {
        if (prevLineNotThrID) content += '\n';
        content += line;

        if (content.size() >= BLOCK_SIZE) {
          write_content();
          content.clear();
          content.reserve(BLOCK_SIZE);
        }

        prevLineNotThrID = true;
      }
    }
  }
  write_content();

  // Close/delete input/output files
  for (t = n_threads; t--;) {
    ushdFile[t].close();
    std::string ushdFileName = USH_FNAME;
    ushdFileName += std::to_string(t);
    std::remove(ushdFileName.c_str());
  }
}