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

#include <fstream>
#include <algorithm>
using std::wifstream;
using std::ifstream;
using std::to_string;

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
 * @brief  Find file type: FASTA (A), FASTQ (Q), not FASTA/FASTQ (n)
 * @param  inFileName  Input file name
 * @return A, Q or n
 */
inline char format (const string& inFileName) {
  wchar_t c;
  wifstream in(inFileName);
  
  if (!in.good())
    std::runtime_error("Error: failed opening '" + inFileName + "'.\n");
  
  // Skip leading blank lines or spaces
  while (in.peek()=='\n' || in.peek()==' ')    in.get(c);
  
  // Fastq
  while (in.peek() == '@')     IGNORE_THIS_LINE(in);
  byte nTabs=0;    while (in.get(c) && c!='\n')  if (c=='\t') ++nTabs;
  
  if (in.peek() == '+') { in.close();    return 'Q'; }            // Fastq
  
  // Fasta or Not Fasta/Fastq
  in.clear();   in.seekg(0, std::ios::beg); // Return to beginning of the file
  while (in.peek()!='>' && in.peek()!=EOF)    IGNORE_THIS_LINE(in);
  
  if (in.peek() == '>') { in.close();    return 'A'; }      // Fasta
  else                  { in.close();    return 'n'; }      // Not Fasta/Fastq
}

/**
 * @brief Save the contents of a file into a string
 * @param fname  file name
 * @param out    the output string
 */
template <typename Iter>
void file_to_string (const string& fname, Iter out) {
  std::ifstream in(fname);
  for (char c; in.get(c);)
    *out++ += c;
  in.close();
}

/**
 * @brief  Get password from a file
 * @return Password (string)
 */
inline string read_pass (const string& keyFile) {
  string pass;
  file_to_string(keyFile, pass.begin());
  return pass;
}

/**
 * @brief Check password taken from a file
 * @param keyFile  Name of the file containing the password
 * @param k_flag   If '-k' is entered by the user, for running Cryfa
 */
inline void check_pass (const string& keyFile, const bool k_flag) {
  if (!k_flag)
    std::runtime_error("Error: no password file has been set.\n");
  else {
    ifstream in(keyFile);
    if (in.peek() == EOF) {
      in.close();
      std::runtime_error("Error: password file is empty.\n");
    }
    else if (!in.good()) {
      in.close();
      std::runtime_error("Error opening \"" + keyFile + "\".\n");
    }
    else {
      const string pass = read_pass(keyFile);
      if (pass.size() < 8) {
        in.close();
        std::runtime_error("Error: the password size is "+to_string(pass.size())
                           + ". It must be at least 8.\n");
      }
      in.close();
    }
  }
}

/**
 * @brief  Check if a string exists in a range
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @return Yes, if it exists
 */
template <typename Iter, typename T>
bool exist (Iter first, Iter last, const T& value) {
  return std::find(first, last, value) != last;
}

/**
 * @brief Usage guide
 */
inline void help () {
  cout                                                                  << '\n'
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