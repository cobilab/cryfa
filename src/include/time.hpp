/**
 * @file      time.hpp
 * @brief     time-related functions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_TIME_HPP
#define CRYFA_TIME_HPP

#include <chrono>
#include <string>

/**
 * @brief  Accumulate hop index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @param  h      hop value
 * @return A number
 */

inline static std::chrono::time_point<std::chrono::high_resolution_clock>
now() noexcept {
  return std::chrono::high_resolution_clock::now();
}

template <typename Time>
inline static std::string hms(Time elapsed) {
  const auto durSec =
      std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
  const auto h = durSec / 3600;
  const auto m = (durSec % 3600) / 60;
  const auto s = durSec % 60;

  if (m < 1) {
    return (s == 0 ? "< 1" : std::to_string(s)) + " sec.\n";
  } else if (h < 1) {
    return std::to_string(m) + ":" + std::to_string(s) + " min:sec.\n";
  } else {
    return std::to_string(h) + ":" + std::to_string(m) + ":" +
           std::to_string(s) + " hour:min:sec.\n";
  }
}

#endif  // CRYFA_TIME_HPP
