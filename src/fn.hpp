//
// Created by morteza on 10-05-2018.
//

#ifndef CRYFA_FN_HPP
#define CRYFA_FN_HPP

#include <fstream>
using std::wifstream;
using std::ifstream;
using std::to_string;

/**
 * @brief        Accumulate hop index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 * @param h      hop value
 * @return       A number
 */
template <typename T, typename Iter, typename Hop>
T accum_hops (Iter first, Iter last, T init, Hop h) {
  for (; first < last; first+=h)
    init += *first;
  return init;
};

/**
 * @brief        Accumulate even index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 * @return       A number
 */
template <typename T, typename Iter>
T accum_even (Iter first, Iter last, T init) {
  return accum_hops(first, last, init, 2);
};

/**
 * @brief         Accumulate odd index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return        A number
 */
template <typename T, typename Iter>
T accum_odd (Iter first, Iter last, T init) {
  return accum_hops(first+1, last, init, 2);
};

/**
 * @brief        Save the contents of a file into a string
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
 * @brief  Check password taken from a file
 * @param  keyFileName  Name of the file containing the password
 * @param  k_flag       If '-k' is entered by the user, for running Cryfa
 */
inline void check_pass (const string& keyFileName, const bool k_flag) {
  if (!k_flag)
    std::runtime_error("Error: no password file has been set.\n");
  else {
    ifstream in(keyFileName);
    
    if (in.peek() == EOF) {
      in.close();
      std::runtime_error("Error: password file is empty.\n");
    }
    else if (!in.good()) {
      in.close();
      std::runtime_error("Error opening \"" + keyFileName + "\".\n");
    }
    else {
      // Extract the password
      char   c;
      string pass;    pass.clear();
      while (in.get(c))    pass += c;
      
      if (pass.size() < 8) {
        in.close();
        std::runtime_error("Error: the password size is " +
                           to_string(pass.size()) + ". It must be at least 8.\n");
      }
      
      in.close();
    }
  }
}

#endif //CRYFA_FN_HPP