/**
 * @file      application.hpp
 * @brief     Application
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_APPLICATION_H
#define CRYFA_APPLICATION_H

#include "def.hpp"
#include "endecrypto.hpp"
#include "fasta.hpp"
#include "fastq.hpp"

namespace cryfa {

class application {
  Param par;
  EnDecrypto crypt;
  Fasta fa;
  Fastq fq;

  void exe_compress_encrypt();
  void exe_decrypt_decompress();

 public:
  application() = default;
  void exe(int, char**);
};

}  // namespace cryfa

#endif  // CRYFA_APPLICATION_H