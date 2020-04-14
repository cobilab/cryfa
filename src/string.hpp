/**
 * @file      string.hpp
 * @brief     String format
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_STRING_HPP
#define CRYFA_STRING_HPP

#include <string>
#include <memory>

inline std::string bold(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[1m" + text + "\033[0m";
#endif
}

inline std::string italic(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[3m" + text + "\033[0m";
#endif
}

inline std::string underline(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[4m" + text + "\033[0m";
#endif
}

inline std::string bold_red(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[1m\033[38;5;1m" + text + "\033[0m";
#endif
}

template <typename... Args>
inline static std::string string_format(const std::string& format,
                                        Args... args) {
  // Extra space for '\0'
  auto size{size_t(snprintf(nullptr, 0, format.c_str(), args...) + 1)};
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1);  // Don't want the '\0' inside
}

#endif  // CRYFA_STRING_HPP