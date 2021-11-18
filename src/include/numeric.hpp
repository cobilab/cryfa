/**
 * @file      numeric.hpp
 * @brief     numerical functions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_NUMERIC_HPP
#define CRYFA_NUMERIC_HPP

#include <algorithm>
#include <fstream>
#include <iostream>

#include "assert.hpp"
#include "string.hpp"

/**
 * @brief  Accumulate hop index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @param  h      hop value
 * @return A number
 */
template <typename T, typename Iter, typename Hop>
T accum_hops(Iter first, Iter last, T init, Hop h) {
  for (; first < last; first += h) init += *first;
  return init;
}

/**
 * @brief  Accumulate even index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_even(Iter first, Iter last, T init) {
  return accum_hops(first, last, init, 2);
}

/**
 * @brief  Accumulate odd index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return A number
 */
template <typename T, typename Iter>
T accum_odd(Iter first, Iter last, T init) {
  return accum_hops(first + 1, last, init, 2);
}

/**
 * @brief  Check if a string is a number
 * @param  s  the input string
 * @return Yes, if it is a number
 */
inline bool is_number(const std::string& s) {
  assert_single(s.empty(), "the string is empty.");
  return std::find_if(s.begin(), s.end(),
                      [](char c) { return !std::isdigit(c); }) == s.end();
}

#endif  // CRYFA_NUMERIC_HPP