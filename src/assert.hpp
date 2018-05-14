/**
 * @file      assert.hpp
 * @brief     Assertions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ASSERT_H
#define CRYFA_ASSERT_H

#include <fstream>
using std::ifstream;
using std::string;

void assert_file_not_empty (const string& fileName) {
  ifstream in(fileName);
  if (in.peek() == EOF) {
    in.close();
    throw std::runtime_error("Error: \"" + fileName + "\" is empty.\n");
  }
}

#endif //CRYFA_ASSERT_H