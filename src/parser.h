/**
 * @file      parser.hpp
 * @brief     Parse command line options
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

template <typename Iter, typename T>
string argument (Iter first, Iter last, const T& value) {
  return *++std::find(first, last, value);
}

void parse (Param par, int argc, char** argv) {
  if (argc < 2)
    help();
  else {
    vector<string> vArgs;    vArgs.reserve(static_cast<u64>(argc));
    for (auto a=argv; a!=argv+argc; ++a)
      vArgs.emplace_back(string(*a));
    
    if (exist(vArgs.begin(), vArgs.end(), "-h") ||
        exist(vArgs.begin(), vArgs.end(), "--help"))
      help();
    
    for (auto i=vArgs.begin(); i!=vArgs.end(); ++i) {
      if (*i=="-k" || *i=="--key") {
        if (i+1 != vArgs.end()) {
          
          par.key_file = *++i;
        }
        else
          throw runtime_error("Error: no password file has been set.\n");
      }
//      if (*i=="-t" || *i=="--tar") {
//        if (i+1 != vArgs.end()) {
//          tar = *++i;
//          checkFile(tar);
//        }
//        else throw runtime_error
//               ("Error: target file not specified. Use \"-t fileName\".\n");
//      }
//      else if (*i=="-r" || *i=="--ref") {
//        if (i+1 != vArgs.end()) {
//          ref = *++i;
//          checkFile(ref);
//        }
//        else throw runtime_error
//              ("Error: reference file not specified. Use \"-r fileName\".\n");
//      }
//      else if ((*i=="-l" || *i=="--level") && i+1!=vArgs.end())
//        level = static_cast<u8>(stoi(*++i));
//      else if (*i=="-v"  || *i=="--verbose")
//        verbose = true;
//      else if ((*i=="-n" || *i=="--nthr") && i+1!=vArgs.end())
//        nthr = static_cast<u8>(stoi(*++i));
//      else if ((*i=="-m" || *i=="--models") && i+1!=vArgs.end())
//        modelsPars = *++i;
//      else if (*i=="-R"  || *i=="--report")
//        report = (i+1!=vArgs.end()) ? *++i : "report.txt";
    }
//
//    // Mandatory args
//    const bool has_t   {exist(vArgs.begin(), vArgs.end(), "-t")};
//    const bool has_tar {exist(vArgs.begin(), vArgs.end(), "--tar")};
//    const bool has_r   {exist(vArgs.begin(), vArgs.end(), "-r")};
//    const bool has_ref {exist(vArgs.begin(), vArgs.end(), "--ref")};
//
//    if (!has_t && !has_tar)
//      throw runtime_error
//        ("Error: target file not specified. Use \"-t fileName\".\n");
//    else if (!has_r && !has_ref)
//      throw runtime_error
//        ("Error: reference file not specified. Use \"-r fileName\".\n");
  }
}

#endif //CRYFA_PARSER_H