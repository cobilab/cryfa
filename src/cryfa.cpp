/*****************************************************
  Cryfa :: A secure encryption tool for genomic data
******************************************************
         Morteza Hosseini    seyedmorteza@ua.pt
         Diogo Pratas        pratas@ua.pt
******************************************************/

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
 * @brief  Run Cryfa
 * @param  argc  number of command line arguments
 * @param  argv  command line arguments
 * @return SUCCESS or FAILURE   
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