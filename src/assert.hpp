/**
 * @file      assert.hpp
 * @brief     Assertions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ASSERT_H
#define CRYFA_ASSERT_H

#ifdef __APPLE__
#undef assert
#endif

#include <fstream>

namespace cryfa {
/**
 * @brief Assert a condition
 * @param cond  the condition to be checked
 * @param msg   the message shown when the condition is true
 */
inline void assert(bool cond, const std::string& msg) {
  if (cond) throw std::runtime_error(msg);
}

/**
 * @brief Assert a condition
 * @param cond  the condition which will be checked
 * @param msgT  the message shown when the condition is true
 * @param msgF  the message shown when the condition is false
 */
inline void assert_dual(bool cond, const std::string& msgT,
                        const std::string& msgF) {
  throw std::runtime_error(cond ? msgT : msgF);
}

/**
 * @brief Check if file is good
 * @param fname  the file name
 * @param msg    the error message
 */
inline void assert_file_good(const std::string& fname,
                             const std::string& msg = "") {
  std::ifstream in(fname);
  if (!in.good() || in.peek() == EOF) {
    in.close();
    assert_dual(msg.empty(), "Error opening the file \"" + fname + "\".\n",
                msg);
  }
  in.close();
}
}  // namespace cryfa

#endif  // CRYFA_ASSERT_H