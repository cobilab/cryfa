/**
 * @file assert.hpp
 * @brief Assertion and diagnostic helpers
 * @author Morteza Hosseini (seyedmorteza.hosseini@manchester.ac.uk)
 * @author Diogo Pratas (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ASSERT_H
#define CRYFA_ASSERT_H

#include <format>
#include <fstream>
#include <stdexcept>  // std::runtime_error

#include "string.hpp"

/**
 * @brief Throw a formatted runtime error
 * @param message Message shown after the error prefix
 * @param width Maximum terminal width used when wrapping the message
 * @throws std::runtime_error Always throws with the formatted error message
 */
inline void error(std::string const& message, int width = 65) {
  std::string msg = wrap_text(std::format("Error: {}", message), "", width);
  msg = std::format("{}{}\n", bold(msg.substr(0, 6), "red"), msg.substr(6));
  throw std::runtime_error(msg);
}

/**
 * @brief Print a formatted warning message
 * @param message Message shown after the warning prefix
 * @param width Maximum terminal width used when wrapping the message
 */
inline void warning(std::string const& message, int width = 65) {
  std::string msg = wrap_text(std::format("Warning: {}", message), "", width);
  msg = std::format("{}{}\n", bold(msg.substr(0, 8), "magenta"), msg.substr(8));
  std::cerr << msg;
}

/**
 * @brief Throw an error when a condition is true
 * @param cond Condition that triggers the error
 * @param msg Message shown when the condition is true
 */
inline void assert_single(bool cond, const std::string& msg) {
  if (cond) error(msg);
}

/**
 * @brief Throw one of two errors based on a condition
 * @param cond Condition used to select the error message
 * @param msgT Message shown when the condition is true
 * @param msgF Message shown when the condition is false
 */
inline void assert_dual(bool cond, const std::string& msgT, const std::string& msgF) {
  error(cond ? msgT : msgF);
}

/**
 * @brief Check that a file can be opened and is not empty
 * @param fname File name to check
 * @param msg Optional custom error message
 */
inline void assert_file_good(const std::string& fname, const std::string& msg = "") {
  std::ifstream in(fname);
  if (!in.good() || in.peek() == EOF) {
    in.close();
    assert_dual(msg.empty(), std::format("failed opening \"{}\".", fname), msg);
  }
  in.close();
}

#endif  // CRYFA_ASSERT_H
