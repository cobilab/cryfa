/**
 * @file      parser.hpp
 * @brief     Parser for command line options
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_PARSER_H
#define CRYFA_PARSER_H

#include <iostream>
#include <algorithm>
#include "def.hpp"
#include "fn.hpp"
using std::runtime_error;
using std::cerr;
using std::wcin;

/**
 * @brief  Argument of a command line option
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @param  value  the value to be found in the range
 * @return An string
 */
template <typename Iter, typename T>
inline string argument (Iter first, Iter last, const T& value) {
  return *++std::find(first, last, value);
}

/**
 * @brief Check password file
 * @param fname  the password file name
 */
inline void check_pass (const string& fname) {
  assert_file_good(fname, "Error opening the password file \"" +fname+ "\".\n");
  const string pass = file_to_string(fname);
  assert(pass.size() < 8, "Error: the password size must be at least 8.\n");
}

/**
 * @brief  Find file type: FASTA (A), FASTQ (Q), not FASTA/FASTQ (n)
 * @param  inFileName  Input file name
 * @return A, Q or n
 */
inline char frmt () {
  wchar_t c;
//  wifstream in(inFileName);
  assert(!wcin.good(), "Error: failed opening the input file.\n");
  
  // Skip leading blank lines or spaces
  while (wcin.peek()=='\n' || wcin.peek()==' ')    wcin.get(c);
  
  // Fastq
  while (wcin.peek() == '@')     IGNORE_THIS_LINE(wcin);
  byte nTabs=0;    while (wcin.get(c) && c!='\n')  if (c=='\t') ++nTabs;
  
  if (wcin.peek() == '+') { /*wcin.close();*/    return 'Q'; }            // Fastq
  
  // Fasta or Not Fasta/Fastq
  wcin.clear();   wcin.seekg(0, std::ios::beg); // Return to beginning of the file
  while (wcin.peek()!='>' && wcin.peek()!=EOF)    IGNORE_THIS_LINE(wcin);
  
  if (wcin.peek() == '>') { /*wcin.close();*/    return 'A'; }      // Fasta
  else                  { /*wcin.close();*/    return 'n'; }      // Not Fasta/Fastq
}

#ifdef DEBUG
inline char frmt_not_stdin (const string& inFileName) {
  wchar_t c;
  wifstream in(inFileName);
  assert(!in.good(), "Error: failed opening '" + inFileName + "'.\n");

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
#endif

/**
 * @brief  Parse the command line options
 * @param  par   An object to hold parameters
 * @param  argc  Number of command line options
 * @param  argv  Array of command line options
 * @return 'c': compress+encrypt or 'd': decrypt+decompress
 */
char parse (Param& par, int argc, char** argv) {
  if (argc < 2)
    help();
  else {
///    par.in_file = *(argv+argc-1);  // Not standard input
    vector<string> vArgs;    vArgs.reserve(static_cast<u64>(argc));
    for (auto a=argv; a!=argv+argc; ++a)
      vArgs.emplace_back(string(*a));
    
    // Help
    if (exist(vArgs.begin(), vArgs.end(), "-h") ||
        exist(vArgs.begin(), vArgs.end(), "--help"))
      help();
  
    // key -- MANDATORY
    assert(!exist(vArgs.begin(), vArgs.end(), "-k") &&
           !exist(vArgs.begin(), vArgs.end(), "--key"),
           "Error: no password file has been set.\n");
    for (auto i=vArgs.begin(); i!=vArgs.end(); ++i) {
      if (*i=="-k" || *i=="--key") {
        if (i+1!=vArgs.end() && (*(i+1))[0]!='-') {
          check_pass(*(i+1));
          par.key_file = *++i;
          break;
        }
        else throw runtime_error("Error: no password file has been set.\n");
      }
    }
    
    // verbose, thread
    for (auto i=vArgs.begin(); i!=vArgs.end(); ++i) {
      if (*i=="-v"  || *i=="--verbose") {
        par.verbose = true;
        cerr << "Verbose mode on.\n";
      }
      else if ((*i=="-t" || *i=="--thread") &&
               i+1!=vArgs.end() && (*(i+1))[0]!='-' && is_number(*(i+1)))
        par.n_threads = static_cast<byte>(stoi(*++i));
    }
    
    // Decrypt+decompress
    if (exist(vArgs.begin(), vArgs.end(), "-d") ||
        exist(vArgs.begin(), vArgs.end(), "--dec"))
      return 'd';
    
    // stop_shuffle, frmt
    for (auto i=vArgs.begin(); i!=vArgs.end(); ++i) {
      if (*i=="-s"  || *i=="--stop_shuffle")
        par.stop_shuffle = true;
      else if ((*i=="-f"  || *i=="--format") &&
               i+1!=vArgs.end() && (*(i+1))[0]!='-') {
        if      (*(i+1)=="a")  par.format='A';
        else if (*(i+1)=="q")  par.format='Q';
        else if (*(i+1)=="n")  par.format='n';
        ++i;
      }
    }
    if (!exist(vArgs.begin(), vArgs.end(), "-f") &&
        !exist(vArgs.begin(), vArgs.end(), "--format"))
      par.format = frmt();
///    par.format = frmt_not_stdin(par.in_file);  // Not standard input file

    // Compress+encrypt
    return 'c';
  }
}

#endif //CRYFA_PARSER_H