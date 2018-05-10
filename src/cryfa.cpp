/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <    Cryfa :: A secure encryption tool for genomic data   >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <          Morteza Hosseini    seyedmorteza@ua.pt         >
          <          Diogo Pratas        pratas@ua.pt               >
          <          Armando J. Pinho    ap@ua.pt                   >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <   Copyright (C) 2017-2018, IEETA, University of Aveiro  >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @file      cryfa.cpp
 * @brief     Main
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include "def.hpp"
#include "security.hpp"
#include "endecrypto.hpp"
#include "fasta.hpp"
#include "fastq.hpp"
#define __STDC_FORMAT_MACROS
#if defined(_MSC_VER)
#    include <io.h>
#else
#    include <unistd.h>
#endif
using std::string;
using std::cout;
using std::cerr;
using std::ifstream;
using std::wifstream;
using std::setprecision;
using std::chrono::high_resolution_clock;
using std::to_string;

/**
 * @brief  Find file type: FASTA (A), FASTQ (Q), not FASTA/FASTQ (n)
 * @param  inFileName  Input file name
 * @return A, Q or n
 */
inline char fileType (const string& inFileName) {
  wchar_t c;
  wifstream in(inFileName);
  
  if (!in.good())
    std::runtime_error("Error: failed opening '" + inFileName + "'.\n");
  
  // Skip leading blank lines or spaces
  while (in.peek()=='\n' || in.peek()==' ')    in.get(c);
  
  // FASTQ
  while (in.peek() == '@')     IGNORE_THIS_LINE(in);
  byte nTabs=0;    while (in.get(c) && c!='\n')  if (c=='\t') ++nTabs;

  if (in.peek() == '+') { in.close();    return 'Q'; }            // FASTQ

  // FASTA or Not FASTA/FASTQ
  in.clear();   in.seekg(0, std::ios::beg); // Return to beginning of the file
  while (in.peek()!='>' && in.peek()!=EOF)    IGNORE_THIS_LINE(in);
  
  if (in.peek() == '>') { in.close();    return 'A'; }      // FASTA
  else                  { in.close();    return 'n'; }      // Not FASTA/FASTQ
}

/**
 * @brief  Check password taken from a file
 * @param  keyFileName  Name of the file containing the password
 * @param  k_flag       If '-k' is entered by the user, for running Cryfa
 */
inline void checkPass (const string& keyFileName, const bool k_flag) {
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

// Instantiation of static variables in InArgs structure
bool   InArgs::VERBOSE         = false;
bool   InArgs::DISABLE_SHUFFLE = false;
byte   InArgs::N_THREADS       = DEFAULT_N_THR;
string InArgs::IN_FILE_NAME    = "";
string InArgs::KEY_FILE_NAME   = "";

/**
 * @brief Main function
 */
int main (int argc, char* argv[]) {
  try {
    std::ios::sync_with_stdio(false); // Turn off synchronizing C++ to C streams

    InArgs     inArgsObj;
    Security   secObj;
    endecrypto cryptObj;
    FASTA      fastaObj;
    FASTQ      fastqObj;

    inArgsObj.IN_FILE_NAME = argv[argc-1];    // Input file name

    static int h_flag, v_flag, d_flag, s_flag;
    bool       k_flag = false;
    int        c;                     // Deal with getopt_long()
    int        option_index;          // Option index stored by getopt_long()
    opterr = 0;  // Force getopt_long() to remain silent when it finds a problem

    static struct option long_options[] = {
      {"help",            no_argument, &h_flag, (int) 'h'},   // Help
      {"verbose",         no_argument, &v_flag, (int) 'v'},   // Verbose
      {"disable_shuffle", no_argument, &s_flag, (int) 's'},   // D (un)shuffle
      {"dec",             no_argument, &d_flag, (int) 'd'},   // Decomp mode
      {"key",       required_argument,       0,       'k'},   // Key file
      {"thread",    required_argument,       0,       't'},   // #threads >= 1
      {0,                           0,       0,         0}
    };

    while (true) {
      option_index = 0;
      if ((c = getopt_long(argc, argv, ":hvsdk:t:",
                           long_options, &option_index)) == -1)         break;

      switch (c) {
        case 0:
          // If this option set a flag, do nothing else now.
          if (long_options[option_index].flag != 0)                 break;
          cerr << "option '" << long_options[option_index].name << "'\n";
          if (optarg)    cerr << " with arg " << optarg << '\n';
          break;

        case 'k':
          k_flag = true;
          inArgsObj.KEY_FILE_NAME = string(optarg);
          break;

        case 'h':  h_flag=1;    Help();                               break;
        case 'v':  v_flag=1;    inArgsObj.VERBOSE = true;             break;
        case 's':  s_flag=1;    inArgsObj.DISABLE_SHUFFLE = true;     break;
        case 'd':  d_flag=1;                                          break;
        case 't':  inArgsObj.N_THREADS = (byte) stoi(string(optarg)); break;

        default:
          cerr << "Option '" << (char) optopt << "' is invalid.\n"; break;
      }
    }

    // Check password file
    if (!h_flag)
      checkPass(inArgsObj.KEY_FILE_NAME, k_flag);

    // Verbose mode
    if (v_flag)     cerr << "Verbose mode on.\n";
    
    // Decrypt and/or unshuffle + decompress
    if (d_flag) {
      cryptObj.decrypt();                                         // Decrypt

      ifstream in(DEC_FNAME);
      switch (in.peek()) {
        case (char) 127:
            cerr<<"Decompressing...\n";    fastaObj.decompress();     break;
        case (char) 126:
            cerr<<"Decompressing...\n";    fastqObj.decompress();     break;
        case (char) 125:
          cryptObj.unshuffle_file();  break;
        default:                                                      break;
      }
      in.close();

      return 0;
    }
    
    // Compress and/or shuffle + encrypt
    if (!h_flag) {
      switch (fileType(inArgsObj.IN_FILE_NAME)) {
        case 'A':  cerr<<"Compacting...\n";    fastaObj.compress();    break;
        case 'Q':  cerr<<"Compacting...\n";    fastqObj.compress();    break;
        case 'n':                          cryptObj.shuffle_file();    break;
        default :  cerr<<"Error: \"" << inArgsObj.IN_FILE_NAME << "\" "
                       <<"is not a valid FASTA or FASTQ file.\n";
                   return 0;                                           break;
      }
    }
  }
  catch (std::exception& e) { cerr << e.what(); }
  catch (...) { return EXIT_FAILURE; }

  return 0;
}