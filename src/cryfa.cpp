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
#include "fn.hpp"
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

// Instantiation of static variables in Param structure
bool   Param::verbose         = false;
bool   Param::disable_shuffle = false;
byte   Param::n_threads       = DEFAULT_N_THR;
string Param::in_file         = "";
string Param::key_file        = "";
char   Param::format          = 'n';

/**
 * @brief Main function
 */
int main (int argc, char* argv[]) {
  try {
//  std::ios::sync_with_stdio(false); // Turn off synchronizing C++ to C streams

    Param      par;
    EnDecrypto crypt;
    Fasta      fa;
    Fastq      fq;

    par.in_file = argv[argc-1];    // Input file name

    static int h_flag, v_flag, d_flag, s_flag, f_flag;
    bool       k_flag = false;
    int        c;                     // Deal with getopt_long()
    int        option_index;          // Option index stored by getopt_long()
    opterr = 0;  // Force getopt_long() to remain silent when it finds a problem
    
    if (argc < 2)
      Help();
    
    static struct option long_options[] = {
      {"help",            no_argument, &h_flag, (int) 'h'},   // Help
      {"verbose",         no_argument, &v_flag, (int) 'v'},   // Verbose
      {"disable_shuffle", no_argument, &s_flag, (int) 's'},   // Dis (un)shuffle
      {"dec",             no_argument, &d_flag, (int) 'd'},   // Decomp mode
      {"key",       required_argument, nullptr,       'k'},   // Key file
      {"thread",    required_argument, nullptr,       't'},   // #threads >= 1
      {"format",    required_argument, nullptr,       'f'},   // Input format
      {nullptr,                     0, nullptr,         0}
    };

    while (true) {
      option_index = 0;
      if ((c = getopt_long(argc, argv, ":hvsdk:t:f:",
                           long_options, &option_index)) == -1)           break;

      switch (c) {
        case 0:
          // If this option set a flag, do nothing else now.
          if (long_options[option_index].flag != 0)                       break;
          cerr << "option '" << long_options[option_index].name << "'\n";
          if (optarg)    cerr << " with arg " << optarg << '\n';
          break;
        case 'k':    k_flag = true;  par.key_file = string(optarg);       break;
        case 'h':    h_flag=1;       Help();                              break;
        case 'v':    v_flag=1;       par.verbose = true;                  break;
        case 's':    s_flag=1;       par.disable_shuffle = true;          break;
        case 'd':    d_flag=1;                                            break;
        case 't':    par.n_threads = (byte) stoi(string(optarg));         break;
        case 'f':    f_flag=1;
          if      (string(optarg)=="a")  par.format = 'A';
          else if (string(optarg)=="q")  par.format = 'Q';
          else if (string(optarg)=="n")  par.format = 'n';
          break;
        default:     cerr<<"Option '"<<(char) optopt<<"' is invalid.\n";  break;
      }
    }
    
    // Check password file
    if (!h_flag)
      check_pass(par.key_file, k_flag);
    
    // Verbose mode
    if (v_flag)     cerr << "Verbose mode on.\n";
    
    // Decrypt and/or unshuffle + decompress
    if (d_flag) {
      crypt.decrypt();
      ifstream in(DEC_FNAME);
      switch (in.peek()) {
        case (char) 127:  cerr<<"Decompressing...\n";  fa.decompress();   break;
        case (char) 126:  cerr<<"Decompressing...\n";  fq.decompress();   break;
        case (char) 125:  crypt.unshuffle_file();                         break;
        default:                                                          break;
      }
      in.close();
      return 0;
    }
    
    // Compress and/or shuffle + encrypt
    if (!h_flag) {
      if (!f_flag)
        par.format = format(par.in_file);
      switch (par.format) {
        case 'A':    cerr<<"Compacting...\n";    fa.compress();           break;
        case 'Q':    cerr<<"Compacting...\n";    fq.compress();           break;
        case 'n':    crypt.shuffle_file();                                break;
        default :    cerr << "Error: \"" << par.in_file << "\" is not a valid "
                          << "FASTA or FASTQ file.\n";    return 0;       break;
      }
    }
  }
  catch (std::exception& e) { cerr << e.what(); }
  catch (...) { return EXIT_FAILURE; }

  return 0;
}