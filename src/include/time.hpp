// SPDX-FileCopyrightText: 2026 Morteza Hosseini
// SPDX-License-Identifier: GPL-3.0-only

/**
 * @file time.hpp
 * @brief Time-related functions
 */

#ifndef CRYFA_TIME_HPP
#define CRYFA_TIME_HPP

#include <chrono>
#include <format>
#include <string>

/**
 * @brief Get the current high-resolution time point
 * @return Current time point
 */
inline static std::chrono::time_point<std::chrono::high_resolution_clock> now() noexcept {
  return std::chrono::high_resolution_clock::now();
}

/**
 * @brief Format an elapsed duration as a human-readable string
 * @tparam Time Duration type accepted by std::chrono::duration_cast
 * @param elapsed Elapsed duration
 * @return Formatted elapsed time
 */
template <typename Time>
inline static std::string hms(Time elapsed) {
  const auto durSec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
  const auto h = durSec / 3600;
  const auto m = (durSec % 3600) / 60;
  const auto s = durSec % 60;

  if (m < 1) {
    return (s == 0) ? "< 1 sec.\n" : std::format("{} sec.\n", s);
  } else if (h < 1) {
    return std::format("{}:{} min:sec.\n", m, s);
  } else {
    return std::format("{}:{}:{} hour:min:sec.\n", h, m, s);
  }
}

#endif  // CRYFA_TIME_HPP
