/**
 * @file      assert.hpp
 * @brief     Assertions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ASSERT_H
#define CRYFA_ASSERT_H

#include <fstream>

#include "string.hpp"

/**
 * @brief Show error
 * @param message  the message to be shown
 * @param width    width of the message shown on terminal
 */
inline void error(std::string const& message, int width = 65) {
  std::string msg = wrap_text("Error: " + message, "", width);
  msg = bold(msg.substr(0, 6), "red") + msg.substr(6) + "\n";
  throw std::runtime_error(msg);
}

/**
 * @brief Show warning
 * @param message  the message to be shown
 * @param width    width of the message shown on terminal
 */
inline void warning(std::string const& message, int width = 65) {
  std::string msg = wrap_text("Warning: " + message, "", width);
  msg = bold(msg.substr(0, 8), "magenta") + msg.substr(8) + "\n";
  std::cerr << msg;
}

/**
 * @brief Assert a condition
 * @param cond  the condition to be checked
 * @param msg   the message shown when the condition is true
 */
inline void assert_single(bool cond, const std::string& msg) {
  if (cond) error(msg);
}

/**
 * @brief Assert a condition
 * @param cond  the condition which will be checked
 * @param msgT  the message shown when the condition is true
 * @param msgF  the message shown when the condition is false
 */
inline void assert_dual(bool cond, const std::string& msgT,
                        const std::string& msgF) {
  error(cond ? msgT : msgF);
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
    assert_dual(msg.empty(), "failed opening \"" + fname + "\".", msg);
  }
  in.close();
}

#endif  // CRYFA_ASSERT_H