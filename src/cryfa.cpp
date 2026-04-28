// SPDX-FileCopyrightText: 2026 Morteza Hosseini
// SPDX-License-Identifier: GPL-3.0-only

/**
 * @file cryfa.cpp
 * @brief Main file - Cryfa: a secure encryption tool for genomic data
 */

#include <exception>  // std::exception

#include "application.hpp"
using namespace cryfa;

/**
 * @brief Run Cryfa
 * @param argc Number of command line arguments
 * @param argv Command line arguments
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
