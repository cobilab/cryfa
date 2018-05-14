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
using std::runtime_error;

/**
 * @brief Check if file is good
 * @param fname  the file name
 */
inline void assert_file_good (const string& fname, const string& msg="") {
  ifstream in(fname);
  if (!in.good() || in.peek()==EOF) {
    in.close();
    if (msg.empty())
      throw std::runtime_error("Error opening the file \"" + fname + "\".\n");
    else
      throw std::runtime_error(msg);
  }
  in.close();
}

/**
 * @brief Assert a condition
 * @param cond  the condition to be checked
 * @param msg   the message shown when the condition is true
 */
inline void assert (bool cond, const string& msg) {
  if (cond)  throw runtime_error(msg);
}

/**
 * @brief Assert a condition
 * @param cond  the condition which will be checked
 * @param msgT  the message shown when the condition is true
 * @param msgF  the message shown when the condition is false
 */
inline void assert_dual (bool cond, const string& msgT, const string& msgF) {
  throw runtime_error(cond ? msgT : msgF);
}

#endif //CRYFA_ASSERT_H