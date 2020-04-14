/*******************************************************
   Cryfa :: A secure encryption tool for genomic data
********************************************************
         Morteza Hosseini    seyedmorteza@ua.pt
         Diogo Pratas        pratas@ua.pt
         Armando J. Pinho    ap@ua.pt
********************************************************
  Copyright (C) 2017-2020, IEETA, University of Aveiro
********************************************************/

/**
 * @file      cryfa.cpp
 * @brief     Main
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include "application.hpp"
using namespace cryfa;

/**
 * @brief Main function
 */
int main(int argc, char* argv[]) {
  try {
    application{}.exe(argc, argv);
  } catch (std::exception& e) {
    std::cerr << e.what();
  } catch (...) {
    return EXIT_FAILURE;
  }

  return 0;
}