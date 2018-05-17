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
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include <memory>
#include "def.hpp"
#include "security.hpp"
#include "endecrypto.hpp"
#include "fasta.hpp"
#include "fastq.hpp"
#include "fn.hpp"
#include "parser.hpp"
#define __STDC_FORMAT_MACROS
#if defined(_MSC_VER)
#  include <io.h>
#else
#  include <unistd.h>
#endif
using std::string;
using std::cout;
using std::cerr;
using std::ifstream;
using std::wifstream;
using std::setprecision;
using std::chrono::high_resolution_clock;
using std::to_string;
using std::make_shared;

// Instantiation of static variables in Param structure
bool   Param::verbose      = false;
bool   Param::stop_shuffle = false;
byte   Param::n_threads    = DEF_N_THR;
string Param::in_file      = "";//todo remove
string Param::key_file     = "";
char   Param::format       = 'n';

//todo
    void f(){
      string s;
      for(char c;cin.get(c);)
        s+=c;
      cerr<<s;
    }
    
/**
 * @brief Main function
 */
//todo
#include <thread>
#include <future>

bool is_prime (int x) {
  for (int i=(int)std::round(std::sqrt(x)); i>2; --i) if (x%i==0) return false;
  return true;
}
int main (int argc, char* argv[]) {
  try {
    //todo
//    f();
//    std::thread t1(&f);
//    std::thread t2(&f);
//    if(t1.joinable())
//      t1.join();
//    if(t2.joinable())
//      t2.join();
  
//    // call function asynchronously:
//    std::future<bool> fut = std::async (is_prime,13768146);
//
//    // do something while waiting for function to set future:
//    std::cout << "checking, please wait";
////    std::chrono::milliseconds span (100);
////    while (fut.wait_for(span)==std::future_status::timeout)
////      std::cout << '.' << std::flush;
//
//    bool x = fut.get();     // retrieve return value
//
//    std::cout << "\n444444443 " << (x?"is":"is not") << " prime.\n";



    Param par;
    auto  crypt = make_shared<EnDecrypto>();
    auto  fa    = make_shared<Fasta>();
    auto  fq    = make_shared<Fastq>();

    const char action = parse(par, argc, argv);

    // Decrypt and/or unshuffle + decompress
    if (action == 'd') {
      crypt->decrypt();
      ifstream in(DEC_FNAME);
      switch (in.peek()) {
        case (char) 127:  cerr<<"Decompressing...\n";  fa->decompress();  break;
        case (char) 126:  cerr<<"Decompressing...\n";  fq->decompress();  break;
        case (char) 125:  crypt->unshuffle_file();                        break;
        default:          throw runtime_error("Error: corrupted file.");
      }
      in.close();
      return 0;
    }
    // Compress and/or shuffle + encrypt
    else if (action == 'c') {
      switch (par.format) {
        case 'A':    cerr<<"Compacting...\n";    fa->compress();          break;
        case 'Q':    cerr<<"Compacting...\n";    fq->compress();          break;
        case 'n':    crypt->shuffle_file();                               break;
        default :    throw runtime_error("Error: the input file is not valid.\n");
//        default :    throw runtime_error("Error: \"" +par.in_file+ "\" is not"
//                                         " a valid FASTA or FASTQ file.\n");todo
      }
    }
  }
  catch (std::exception& e) { cerr << e.what(); }
  catch (...) { return EXIT_FAILURE; }

  return 0;
}