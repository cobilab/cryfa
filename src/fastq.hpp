/**
 * @file      fastq.hpp
 * @brief     Compression/Decompression of FASTQ
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FASTQ_H
#define CRYFA_FASTQ_H

#include "endecrypto.hpp"
#include "security.hpp"

namespace cryfa {
/** @brief Packing FASTQ */
struct packfq_s {
  packFP_t packHdrFPtr; /**< @brief Points to a hdr packing function */
  packFP_t packQSFPtr;  /**< @brief Points to a qs packing function */
};

/** @brief Unpakcing FASTQ */
struct unpackfq_s {
  char XChar_hdr; /**< @brief Extra char if header's length > 39 */
  char XChar_qs;  /**< @brief Extra char if q scores length > 39 */
  pos_t begPos;   /**< @brief Begining position for each thread */
  u64 chunkSize;  /**< @brief Chunk size */
  std::vector<std::string>
      hdrUnpack; /**< @brief Lookup table for unpacking headers */
  std::vector<std::string>
      qsUnpack;             /**< @brief Lookup table for unpacking q scores */
  unpackFP_t unpackHdrFPtr; /**< @brief Points to a hdr unpacking function */
  unpackFP_t unpackQSFPtr;  /**< @brief Points to a qs unpacking function */
};

/**
 * @brief Compression/Decompression of FASTQ
 */
class Fastq : public EnDecrypto {
 public:
  void compress();
  void decompress();

 private:
  bool justPlus = true; /**< @brief If line 3 is just +  @hideinitializer */

  auto has_just_plus() const -> bool;
  void gather_h_q(std::string&, std::string&);
  void set_hashTbl_packFn(packfq_s&, const std::string&, const std::string&);
  void pack(const packfq_s&, byte);
  void set_unpackTbl_unpackFn(unpackfq_s&, const std::string&,
                              const std::string&);
  void unpack_hS_qS(const unpackfq_s&, byte);
  void unpack_hS_qL(const unpackfq_s&, byte);
  void unpack_hL_qS(const unpackfq_s&, byte);
  void unpack_hL_qL(const unpackfq_s&, byte);
};
}  // namespace cryfa

#endif  // CRYFA_FASTQ_H