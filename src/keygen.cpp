/**
 * @file      keygen.cpp
 * @brief     Key generator
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <iostream>
#include "def.hpp"
#include "fn.hpp"
using std::cerr;
using std::cin;
using std::string;
using std::uniform_int_distribution;
using std::ofstream;

int main (int argc, char* argv[]) {
  constexpr u32 keyLen = 1024;
  
  cerr << "Enter a password:\n";
  string pass;
  cin >> pass;
  
  cerr << "Enter the output file name (for the generated key):\n";
  string target;
  cin >> target;
  
  uniform_int_distribution<rng_t::result_type> udist(0, 255);
  rng_t rng;
  
  // Using old random to generate the new random seed
  srandom(static_cast<u32>(36721 *
          (94583 * accum_even(pass.begin(), pass.end(), 0ul) +
          279431 * accum_odd(pass.begin(), pass.end(), 0ul)) + 623681));
  u64 seed = 0;
  for (char c : pass)
    seed += c*random() + random();
  
  rng.seed(static_cast<rng_t::result_type>(seed));
  
  string key;
  for (auto i=keyLen; i--;)
    key += static_cast<char>(
      (udist(rng) * accum_hops(pass.begin(), pass.end(), 0u, i+1)) % 128);
  
  ofstream keyFile(target);
  keyFile << key;
}