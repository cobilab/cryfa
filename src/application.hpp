/**
 * @file      application.hpp
 * @brief     Application
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_APPLICATION_H
#define CRYFA_APPLICATION_H

#include <chrono>
#include <fstream>
#include <iomanip>  // setw, setprecision
#include <iostream>
#include <memory>

#include "def.hpp"
#include "endecrypto.hpp"
#include "fasta.hpp"
#include "fastq.hpp"
#include "numeric.hpp"
#include "parser.hpp"
#include "security.hpp"
// #define __STDC_FORMAT_MACROS
// #if defined(_MSC_VER)
// #  include <io.h>
// #else
// #  include <unistd.h>
// #endif

namespace cryfa {
// Instantiation of static variables in Param structure
bool Param::verbose = false;
bool Param::stop_shuffle = false;
byte Param::n_threads = DEF_N_THR;
std::string Param::in_file = "";
std::string Param::key_file = "";
char Param::format = 'n';

/**
 * @brief Running the application
 */
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

/**
 * @brief Compress and/or shuffle + encrypt
 */
void application::exe_compress_encrypt() {
  switch (par.format) {
    case 'A':
      fa.compress();
      break;
    case 'Q':
      fq.compress();
      break;
    case 'n':
      crypt.shuffle_file();
      break;
    default:
      error("\"" + par.in_file + "\" is not a valid FASTA or FASTQ file.");
  }
}

/**
 * @brief Decrypt and/or unshuffle + decompress
 */
void application::exe_decrypt_decompress() {
  crypt.decrypt();
  std::ifstream in(DEC_FNAME);
  switch (in.peek()) {
    case (char)127:
      fa.decompress();
      break;
    case (char)126:
      fq.decompress();
      break;
    case (char)125:
      crypt.unshuffle_file();
      break;
    default:
      error("corrupted file.");
  }
  in.close();
}

/**
 * @brief Execute Cryfa
 * @param argc  number of command line arguments
 * @param argv  command line arguments
 */
void application::exe(int argc, char* argv[]) {
  const char action = parse(par, argc, argv);
  if (action == 'd') {
    exe_decrypt_decompress();
  } else if (action == 'c') {
    exe_compress_encrypt();
  }
}
}  // namespace cryfa

#endif  // CRYFA_APPLICATION_H