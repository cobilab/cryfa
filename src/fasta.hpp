/**
 * @file      fasta.hpp
 * @brief     Compression/Decompression of FASTA
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FASTA_H
#define CRYFA_FASTA_H

#include "endecrypto.hpp"
#include "security.hpp"

namespace cryfa {
/**
 * @brief Packing FASTA
 */
struct packfa_s {
  packFP_t packHdrFP; /**< @brief Points to a header packing function */
};

/**
 * @brief Unpakcing FASTA
 */
struct unpackfa_s {
  char XChar_hdr; /**< @brief Extra char if header's length > 39 */
  pos_t begPos;   /**< @brief Begining position for each thread */
  u64 chunkSize;  /**< @brief Chunk size */
  std::vector<std::string>
      hdrUnpack;          /**< @brief Lookup table for unpacking headers */
  unpackFP_t unpackHdrFP; /**< @brief Points to a header unpacking fn */
};

/**
 * @brief Compression/Decompression of FASTA
 */
class Fasta : public EnDecrypto {
 public:
  void compress();
  void decompress();

 private:
  void gather_h_bs(std::string&);
  void set_hashTbl_packFn(packfa_s&, const std::string&);
  void pack(const packfa_s&, byte);
  void set_unpackTbl_unpackFn(unpackfa_s&, const std::string&);
  void unpack_hS(const unpackfa_s&, byte);
  void unpack_hL(const unpackfa_s&, byte);
};
}  // namespace cryfa

#endif  // CRYFA_FASTA_H