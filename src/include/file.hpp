/**
 * @file      file.hpp
 * @brief     file handling
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FILE_HPP
#define CRYFA_FILE_HPP

#include <fstream>

/**
 * @brief Check if file can be opened correctly
 * @param name  name of the file
 */
inline static void check_file(std::string name) {  // Must be inline
  std::ifstream f(name);
  if (!f) {
    f.close();
    error("the file \"" + name + "\" cannot be opened or is empty.");
  } else {
    bool foundChar{false};
    for (char c; f.get(c) && !foundChar;)
      if (c != ' ' && c != '\n' && c != '\t') foundChar = true;
    if (!foundChar) {
      f.close();
      error("the file \"" + name + "\" is empty.");
    }
    f.close();
  }
}

/**
 * @brief  Extract file name
 * @param  path  path including the file name
 * @return File name
 */
inline static std::string file_name(std::string path) {
  const auto found = path.find_last_of("/\\");
  return path.substr(found + 1);
}

/**
 * @brief  Find file size
 * @param  name name of the file
 * @return File size
 */
inline static uint64_t file_size(std::string name) {
  check_file(name);
  std::ifstream f(name, std::ifstream::ate | std::ifstream::binary);
  return static_cast<uint64_t>(f.tellg());
}

#endif  // CRYFA_FILE_HPP