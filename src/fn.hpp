/**
 * @file      fn.hpp
 * @brief     Global functions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FN_HPP
#define CRYFA_FN_HPP

#include <algorithm>
#include <fstream>
#include <iostream>

#include "assert.hpp"
#include "def.hpp"
#include "string.hpp"

namespace cryfa {
/**
 * @brief  Accumulate hop index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @param  h      hop value
 * @return A number
 */
template <typename T, typename Iter, typename Hop>
T accum_hops(Iter first, Iter last, T init, Hop h) {
  for (; first < last; first += h) init += *first;
  return init;
}

/**
 * @brief  Accumulate even index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_even(Iter first, Iter last, T init) {
  return accum_hops(first, last, init, 2);
}

/**
 * @brief  Accumulate odd index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_odd(Iter first, Iter last, T init) {
  return accum_hops(first + 1, last, init, 2);
}

/**
 * @brief Save the contents of a file into a std::string
 * @param fname  the password file name
 */
inline std::string file_to_string(const std::string& fname) {
  std::ifstream in(fname);
  std::string pass;
  for (char c; in.get(c);) pass += c;
  in.close();
  return pass;
}

/**
 * @brief  Check if a std::string exists in a range
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @param  value  the value to be found in the range
 * @return Yes, if it exists
 */
template <typename Iter, typename T>
bool exist(const Iter first, const Iter last, const T& value) {
  assert(first == last, "Error: the range is empty.\n");
  return std::find(first, last, value) != last;
}

/**
 * @brief  Check if a std::string is a number
 * @param  s  the input std::string
 * @return Yes, if it is a number
 */
inline bool is_number(const std::string& s) {
  assert(s.empty(), "Error: the std::string is empty.\n");
  return std::find_if(s.begin(), s.end(),
                      [](char c) { return !std::isdigit(c); }) == s.end();
}

/**
 * @brief Usage guide
 */
inline void help() {
  std::cerr << bold("NAME") << '\n'
            << "      Cryfa v" << VERSION << " - A secure encryption tool for "
            << "genomic data" << '\n'
            << '\n'
            << bold("SYNOPSIS") << '\n'
            << "      ./cryfa [OPTION]... -k [KEY_FILE] [-d] [IN_FILE] > "
               "[OUT_FILE] \n"
            << '\n'
            << bold("SAMPLE") << '\n'
            << "      Encrypt and Compact:    ./cryfa -k pass.txt in.fq > comp"
            << '\n'
            << "      Decrypt and Unpack:     ./cryfa -k pass.txt -d comp > "
               "orig.fq \n"
            << '\n'
            << "      Encrypt:                ./cryfa -k pass.txt in > enc"
            << '\n'
            << "      Decrypt:                ./cryfa -k pass.txt -d enc > orig"
            << '\n'
            << '\n'
            << bold("DESCRIPTION") << '\n'
            << "      Compact & encrypt FASTA/FASTQ files." << '\n'
            << "      Encrypt any text-based genomic data, e.g., VCF/SAM/BAM."
            << '\n'
            << '\n'
            << "      -h,  --help" << '\n'
            << "           usage guide" << '\n'
            << '\n'
            << "      -k [KEY_FILE],  --key [KEY_FILE]" << '\n'
            << "           key file name -- MANDATORY" << '\n'
            << "           The KEY_FILE would contain a password." << '\n'
            << "           To make a strong password, the \"keygen\" program "
               "can be \n"
            << "           employed via the command \"./keygen\"." << '\n'
            << '\n'
            << "      -d,  --dec" << '\n'
            << "           decrypt & unpack" << '\n'
            << '\n'
            << "      -f,  --force" << '\n'
            << "           force to consider input as non-FASTA/FASTQ" << '\n'
            << "           Forces Cryfa not to compact, but shuffle and "
               "encrypt.    \n"
            << "           If the input is FASTA/FASTQ, it is again considered "
               "as   \n"
            << "           non-FASTA/FASTQ, therefore, compaction will be "
               "ignored,  \n"
            << "           but shuffling and encryption will be performed."
            << '\n'
            << '\n'
            << "      -v,  --verbose" << '\n'
            << "           verbose mode (more information)" << '\n'
            << '\n'
            << "      -s,  --stop_shuffle" << '\n'
            << "           stop shuffling the input" << '\n'
            << '\n'
            << "      -t [NUMBER],  --thread [NUMBER]" << '\n'
            << "           number of threads" << '\n'
            << '\n'
            << bold("AUTHORS") << '\n'
            << "      Morteza Hosseini    seyedmorteza@ua.pt" << '\n'
            << "      Diogo Pratas        pratas@ua.pt" << '\n'
            << '\n'
            << bold("COPYRIGHT") << '\n'
            << "      Copyright (C) " << DEV_YEARS << ", IEETA, University of "
            << "Aveiro." << '\n'
            << "      This is a Free software, under GPLv3. You may "
               "redistribute    \n"
            << "      copies of it under the terms of the GNU - General Public"
            << '\n'
            << "      License v3 <http://www.gnu.org/licenses/gpl.html>. There"
            << '\n'
            << "      is NOT ANY WARRANTY, to the extent permitted by law."
            << '\n';

  throw EXIT_SUCCESS;
}
}  // namespace cryfa

#endif  // CRYFA_FN_HPP