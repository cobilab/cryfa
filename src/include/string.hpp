/**
 * @file string.hpp
 * @brief String format
 * @author Morteza Hosseini (seyedmorteza.hosseini@manchester.ac.uk)
 * @author Diogo Pratas (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_STRING_HPP
#define CRYFA_STRING_HPP

#include <algorithm>
#include <cctype>
#include <format>
#include <memory>
#include <string>

extern void assert_single(bool, const std::string&);

/**
 * @brief Convert a string to lower case
 * @param s The input string
 * @return A string
 */
inline std::string lower_case(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

/**
 * @brief Make a bold string, supporting different colors
 * @param text The input text
 * @param color The color name
 * @return A string
 */
inline std::string bold(const std::string& text, std::string color = "") {
#ifdef _WIN32
  return text;
#else
  const std::string pre = "\033[1m";
  const std::string post = "\033[0m";

  if (color.empty()) {
    return std::format("{}{}{}", pre, text, post);
  }

  const std::string color_l = lower_case(color);
  std::string code = "";

  if (color_l == "black") {
    code = "\033[38;5;0m";
  } else if (color_l == "red") {
    code = "\033[38;5;1m";
  } else if (color_l == "green") {
    code = "\033[38;5;2m";
  } else if (color_l == "yellow") {
    code = "\033[38;5;3m";
  } else if (color_l == "blue") {
    code = "\033[38;5;4m";
  } else if (color_l == "magenta") {
    code = "\033[38;5;5m";
  } else if (color_l == "cyan") {
    code = "\033[38;5;6m";
  } else if (color_l == "white") {
    code = "\033[38;5;7m";
  } else {
    return std::format("{}{}{}", pre, text, post);
  }

  return std::format("{}{}{}{}", pre, code, text, post);

#endif
}

/**
 * @brief Make an italic string
 * @param text The input text
 * @return A string
 */
inline std::string italic(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return std::format("\033[3m{}\033[0m", text);
#endif
}

/**
 * @brief Make an underline string
 * @param text The input text
 * @return A string
 */
inline std::string underline(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return std::format("\033[4m{}\033[0m", text);
#endif
}

/**
 * @brief Format a string
 * @param format Intended format
 * @param args String(s)
 * @return A string
 */
template <typename... Args>
inline static std::string string_format(const std::string& format, Args... args) {
  // Extra space for '\0'
  auto size{size_t(snprintf(nullptr, 0, format.c_str(), args...) + 1)};
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), size - 1);  // Exclude the terminating '\0'
}

/**
 * @brief Wrap a text
 * @param text The text
 * @param pre_str The string preceding the text
 * @param width Width of the text
 * @return A string
 */
inline std::string wrap_text(std::string text, std::string pre_str = "", int width = 57) {
  std::string out = pre_str;
  std::string word;
  char last{'\0'};
  uint64_t pos{0};

  for (auto c : text) {
    if (++pos == width) {
      if (word.empty()) {
        return "";
      }

      auto p = std::end(word);
      while (p != std::begin(word) && *--p != ' ');
      if (*p == ' ') {
        word = std::string(++p, std::end(word));
      }

      out += std::format("\n{}{}", pre_str, word);
      pos = word.length();
      word.clear();
    } else if (c == ' ' && last != ' ') {
      out += word;
      word.clear();
    }

    word += c;
    last = c;
  }
  out += word;

  return out;
}

/**
 * @brief Check if a std::string exists in a range
 * @param first Begin iterator of the range
 * @param last End iterator of the range
 * @param value The value to be found in the range
 * @return Yes, if it exists
 */
template <typename Iter, typename T>
bool exist(const Iter first, const Iter last, const T& value) {
  assert_single(first == last, "the range is empty.");
  return std::find(first, last, value) != last;
}

/**
 * @brief Save the contents of a file into a std::string
 * @param fname The password file name
 * @return A string
 */
inline std::string file_to_string(const std::string& fname) {
  std::ifstream in(fname);
  std::string pass;
  for (char c; in.get(c);) {
    pass += c;
  }
  in.close();
  return pass;
}

#endif  // CRYFA_STRING_HPP
