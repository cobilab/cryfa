/**
 * @file      fn.hpp
 * @brief     Global functions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FN_HPP
#define CRYFA_FN_HPP

#include <iostream>
#include <fstream>
#include <algorithm>
#include "assert.hpp"
#include "def.hpp"
using std::wifstream;
using std::ifstream;
using std::to_string;
using std::cerr;
using std::runtime_error;

/**
 * @brief  Accumulate hop index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @param  h      hop value
 * @return A number
 */
template <typename T, typename Iter, typename Hop>
T accum_hops (Iter first, Iter last, T init, Hop h) {
  for (; first < last; first+=h)
    init += *first;
  return init;
};

/**
 * @brief  Accumulate even index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_even (Iter first, Iter last, T init) {
  return accum_hops(first, last, init, 2);
};

/**
 * @brief  Accumulate odd index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_odd (Iter first, Iter last, T init) {
  return accum_hops(first+1, last, init, 2);
};

/**
 * @brief Save the contents of a file into a string
 * @param fname  the password file name
 */
inline string file_to_string (const string& fname) {
  ifstream in(fname);
  string pass;
  for (char c; in.get(c);)
    pass += c;
  in.close();
  return pass;
}

/**
 * @brief  Check if a string exists in a range
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @param  value  the value to be found in the range
 * @return Yes, if it exists
 */
template <typename Iter, typename T>
bool exist (Iter first, Iter last, const T& value) {
  assert(first==last, "Error: the range is empty.\n");
  return std::find(first, last, value) != last;
}

/**
 * @brief  Check if a string is a number
 * @param  s  the input string
 * @return Yes, if it is a number
 */
inline bool is_number (const string& s) {
  assert(s.empty(), "Error: the string is empty.\n");
  return std::find_if(s.begin(), s.end(),
                      [](char c) { return !std::isdigit(c); }) == s.end();
}

/**
 * @brief Usage guide
 */
inline void help () {
  cerr                                                                  << '\n'
       << "NAME"                                                        << '\n'
       << "      Cryfa v" << VERSION << " - "
       <<                    "A secure encryption tool for genomic data"<< '\n'
                                                                        << '\n'
       << "AUTHORS"                                                     << '\n'
       << "      Morteza Hosseini    seyedmorteza@ua.pt"                << '\n'
       << "      Diogo Pratas        pratas@ua.pt"                      << '\n'
       << "      Armando J. Pinho    ap@ua.pt"                          << '\n'
                                                                        << '\n'
       << "SYNOPSIS"                                                    << '\n'
       << "      ./cryfa [OPTION]... -k [key_file] [-d] [in_file] "
       <<                                                "> [OUT_FILE]" << '\n'
                                                                        << '\n'
       << "SAMPLE"                                                      << '\n'
       << "      Encrypt and Compact:    ./cryfa -k pass.txt in.fq "
       <<                                                  "> comp"     << '\n'
       << "      Decrypt and Unpack:     ./cryfa -k pass.txt -d comp "
       <<                                                  "> orig.fq"  << '\n'
                                                                        << '\n'
       << "      Encrypt:                ./cryfa -k pass.txt in > enc"  << '\n'
       << "      Decrypt:                ./cryfa -k pass.txt -d enc > "
       <<                                                        "orig" << '\n'
                                                                        << '\n'
       << "DESCRIPTION"                                                 << '\n'
       << "      Compact & encrypt FASTA/FASTQ files."                  << '\n'
       << "      Encrypt any text-based genomic data."                  << '\n'
                                                                        << '\n'
       << "      The key_file specifies a file including the password." << '\n'
                                                                        << '\n'
       << "      -h,  --help"                                           << '\n'
       << "           usage guide"                                      << '\n'
                                                                        << '\n'
       << "      -k [key_file],  --key [key_file]"                      << '\n'
       << "           key file name -- MANDATORY"                       << '\n'
                                                                        << '\n'
       << "      -d,  --dec"                                            << '\n'
       << "           decrypt & unpack"                                 << '\n'
                                                                        << '\n'
       << "      -f,  --format"                                         << '\n'
       << "           force specified format "
                                   "('a':FASTA, 'q':FASTQ, 'n':others)" << '\n'
                                                                        << '\n'
       << "      -v,  --verbose"                                        << '\n'
       << "           verbose mode (more information)"                  << '\n'
                                                                        << '\n'
       << "      -s,  --disable_shuffle"                                << '\n'
       << "           disable input shuffling"                          << '\n'
                                                                        << '\n'
       << "      -t [NUMBER],  --thread [NUMBER]"                       << '\n'
       << "           number of threads"                                << '\n'
                                                                        << '\n'
       << "COPYRIGHT"                                                   << '\n'
       << "      Copyright (C) " << DEV_YEARS << ", IEETA, University "
       <<                                                  "of Aveiro." << '\n'
       << "      This is a Free software, under GPLv3. You may redistribute \n"
       << "      copies of it under the terms of the GNU - General Public   \n"
       << "      License v3 <http://www.gnu.org/licenses/gpl.html>. There   \n"
       << "      is NOT ANY WARRANTY, to the extent permitted by law."  << '\n';

    throw EXIT_SUCCESS;
}

#endif //CRYFA_FN_HPP