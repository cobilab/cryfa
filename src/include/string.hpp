/**
 * @file      string.hpp
 * @brief     String format
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_STRING_HPP
#define CRYFA_STRING_HPP

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

extern void assert_single(bool, const std::string&);

/**
 * @brief  Convert a string to lower case
 * @param  s  the input string
 * @return A string
 */
inline std::string lower_case(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

/**
 * @brief  Make a bold string, supporting different colors
 * @param  text  the input text
 * @param  color the color name
 * @return A string
 */
inline std::string bold(const std::string& text, std::string color = "") {
#ifdef _WIN32
  return text;
#else
  const std::string pre = "\033[1m";
  const std::string post = "\033[0m";

  if (color == "") return pre + text + post;

  std::string code = "\033[38;5;";
  if (lower_case(color) == "black") code += "0";
  if (lower_case(color) == "red") code += "1";
  if (lower_case(color) == "green") code += "2";
  if (lower_case(color) == "yellow") code += "3";
  if (lower_case(color) == "blue") code += "4";
  if (lower_case(color) == "magenta") code += "5";
  if (lower_case(color) == "cyan") code += "6";
  if (lower_case(color) == "white") code += "7";
  code += "m";
  return pre + code + text + post;
#endif
}

/**
 * @brief  Make an italic string
 * @param  text  the input text
 * @return A string
 */
inline std::string italic(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[3m" + text + "\033[0m";
#endif
}

/**
 * @brief  Make an underline string
 * @param  text  the input text
 * @return A string
 */
inline std::string underline(const std::string& text) {
#ifdef _WIN32
  return text;
#else
  return "\033[4m" + text + "\033[0m";
#endif
}

/**
 * @brief  Format a string
 * @param  format  intended format
 * @param  args    string(s)
 * @return A string
 */
template <typename... Args>
inline static std::string string_format(const std::string& format,
                                        Args... args) {
  // Extra space for '\0'
  auto size{size_t(snprintf(nullptr, 0, format.c_str(), args...) + 1)};
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1);  // Doesn't want the '\0' inside
}

/**
 * @brief  Wrap a text
 * @param  text     the text
 * @param  pre_str  the string preceding the text
 * @param  width    width of the text
 * @return A string
 */
inline std::string wrap_text(std::string text, std::string pre_str = "",
                             int width = 57) {
  std::string out = pre_str;
  std::string word;
  char last{'\0'};
  uint64_t pos{0};

  for (auto c : text) {
    if (++pos == width) {
      if (word.empty()) return "";

      auto p = std::end(word);
      while (p != std::begin(word) && *--p != ' ')
        ;
      if (*p == ' ') word = std::string(++p, std::end(word));

      out += "\n" + pre_str + word;
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
 * @brief  Check if a std::string exists in a range
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @param  value  the value to be found in the range
 * @return Yes, if it exists
 */
template <typename Iter, typename T>
bool exist(const Iter first, const Iter last, const T& value) {
  assert_single(first == last, "the range is empty.");
  return std::find(first, last, value) != last;
}

/**
 * @brief  Save the contents of a file into a std::string
 * @param  fname  the password file name
 * @return A string
 */
inline std::string file_to_string(const std::string& fname) {
  std::ifstream in(fname);
  std::string pass;
  for (char c; in.get(c);) pass += c;
  in.close();
  return pass;
}

#endif  // CRYFA_STRING_HPP