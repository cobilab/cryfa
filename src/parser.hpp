/**
 * @file      parser.hpp
 * @brief     Parser for command line options
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_PARSER_H
#define CRYFA_PARSER_H

#include <algorithm>
#include <iostream>

#include "def.hpp"
#include "file.hpp"
#include "string.hpp"

namespace cryfa {
/**
 * @brief  Argument of a command line option
 * @param  first  begin iterator of the range
 * @param  last   end iterator of the range
 * @param  value  the value to be found in the range
 * @return A string
 */
template <typename Iter, typename T>
inline std::string argument(Iter first, Iter last, const T& value) {
  return *++std::find(first, last, value);
}

/**
 * @brief Check password file
 * @param fname  the password file name
 */
inline void check_pass(const std::string& fname) {
  assert_file_good(fname,
                   "Error opening the password file \"" + fname + "\".\n");
  const std::string pass = file_to_string(fname);
  assert_single(pass.size() < 8, "the password size must be at least 8.");
}

/**
 * @brief  Check input file format (FASTA/FASTQ/other)
 * @param  inFileName  the file name
 * @return A character
 */
inline char frmt(const std::string& inFileName) {
  wchar_t c;
  std::wifstream in(inFileName);
  assert_single(!in.good(), "failed opening \"" + inFileName + "\".");

  // Skip leading blank lines or spaces
  while (in.peek() == '\n' || in.peek() == ' ') in.get(c);

  // Fastq
  while (in.peek() == '@') IGNORE_THIS_LINE(in);
  byte nTabs = 0;
  while (in.get(c) && c != '\n')
    if (c == '\t') ++nTabs;

  if (in.peek() == '+') {
    in.close();
    return 'Q';
  }  // Fastq

  // Fasta or Not Fasta/Fastq
  in.clear();
  in.seekg(0, std::ios::beg);  // Return to beginning of the file
  while (in.peek() != '>' && in.peek() != EOF) IGNORE_THIS_LINE(in);

  if (in.peek() == '>') {
    in.close();
    return 'A';
  }  // Fasta
  else {
    in.close();
    return 'n';
  }  // Not Fasta/Fastq
}

/**
 * @brief Usage guide
 */
inline void show_help() {
  const std::string init_space = "      ";
  const std::string opt_space = init_space + "     ";
  std::cerr
      << bold("NAME") << '\n'
      << init_space << "Cryfa - a secure encryption tool for genomic data \n"
      << '\n'
      << bold("SYNOPSIS") << '\n'
      << init_space << "./cryfa [" << underline("OPTION") << "]... -k ["
      << underline("KEY_FILE") << "] [-d] [" << underline("IN_FILE") << "] > ["
      << underline("OUT_FILE") << "] \n"
      << '\n'
      << bold("SAMPLE") << '\n'
      << init_space << italic("Encrypt and Compact")
      << ":  ./cryfa -k pass.txt in.fq > comp \n"
      << init_space << italic("Decrypt and Unpack")
      << ":   ./cryfa -k pass.txt -d comp > orig.fq \n"
      << '\n'
      << init_space << italic("Encrypt")
      << ":              ./cryfa -k pass.txt in > enc \n"
      << init_space << italic("Decrypt")
      << ":              ./cryfa -k pass.txt -d enc > orig \n"
      << '\n'
      << bold("OPTIONS") << '\n'
      << init_space << "Compact & encrypt FASTA/FASTQ files. \n"
      << init_space
      << "Encrypt any text-based genomic data, e.g., VCF/SAM/BAM. \n"
      << '\n'
      << init_space << bold("-k") << " [" << underline("KEY_FILE") << "],  "
      << bold("--key") << " [" << underline("KEY_FILE") << "] \n"
      << opt_space << "key file name -- " << italic("MANDATORY") << '\n'
      << opt_space << "The KEY_FILE should contain a password. \n"
      << wrap_text(
             "To make a strong password, the \"keygen\" program can be used "
             "via the command \"./keygen\".",
             opt_space)
      << '\n'
      << '\n'
      << init_space << bold("-d") << ",  " << bold("--dec") << '\n'
      << opt_space << "decrypt & unpack \n"
      << '\n'
      << init_space << bold("-f") << ",  " << bold("--force") << '\n'
      << opt_space << "force to consider input as non-FASTA/FASTQ \n"
      << wrap_text(
             "Forces Cryfa not to compact, but shuffle and encrypt. If the "
             "input is FASTA/FASTQ, it is considered as non-FASTA/FASTQ; so, "
             "compaction will be ignored, but shuffling and encryption will be "
             "performed.",
             opt_space)
      << '\n'
      << '\n'
      << init_space << bold("-s") << ",  " << bold("--stop_shuffle") << '\n'
      << opt_space << "stop shuffling the input \n"
      << '\n'
      << init_space << bold("-t") << " [" << underline("NUMBER") << "],  "
      << bold("--thread") << " [" << underline("NUMBER") << "] \n"
      << opt_space << "number of threads \n"
      << '\n'
      << init_space << bold("-v") << ",  " << bold("--verbose") << '\n'
      << opt_space << "verbose mode (more information) \n"
      << '\n'
      << init_space << bold("-h") << ",  " << bold("--help") << '\n'
      << opt_space << "usage guide \n"
      << '\n'
      << init_space << bold("--version") << '\n'
      << opt_space << "version information \n"
      << '\n'
      << bold("AUTHORS") << '\n'
      << "      Morteza Hosseini   seyedmorteza@ua.pt \n"
      << "      Diogo Pratas       pratas@ua.pt \n"
      << '\n'
      << bold("Warning:") << ' '
      << wrap_text(
             "the maximum file size supported is 64 GB. For larger files, you "
             "can split them, e.g. by \"split\" command, and encrypt each "
             "chunk. After the decryption, you can concatenate the chunks, "
             "e.g. by \"cat\" command.", "", 62) << std::endl;

  throw EXIT_SUCCESS;
}

/**
 * @brief Version information
 */
inline void show_version() {
  std::cerr << "Cryfa " << VERSION << std::endl;
  throw EXIT_SUCCESS;
}

/**
 * @brief  Parse the command line options
 * @param  par   An object to hold parameters
 * @param  argc  Number of command line options
 * @param  argv  Array of command line options
 * @return 'c': compress+encrypt or 'd': decrypt+decompress
 */
char parse(Param& par, int argc, char** argv) {
  if (argc < 2) show_help();

  par.in_file = *(argv + argc - 1);  // Not standard input
  std::vector<std::string> vArgs;
  vArgs.reserve(static_cast<u64>(argc));
  for (auto a = argv; a != argv + argc; ++a)
    vArgs.emplace_back(std::string(*a));

  // Help
  if (exist(vArgs.begin(), vArgs.end(), "-h") ||
      exist(vArgs.begin(), vArgs.end(), "--help"))
    show_help();

  // Version
  if (exist(vArgs.begin(), vArgs.end(), "--version")) show_version();

  // Check file size for > 64 GB
  if (file_size(par.in_file) > (1ull << 36)) {
    const std::string message =
        "Size of \"" + file_name(par.in_file) +
        "\" is larger than 64 GB. You can split it, e.g. by \"split\" command, "
        "and encrypt each chunk. "
        "After the decryption, you can concatenate the chunks, e.g. by \"cat\" "
        "command.";
    error(message);
  }

  // key -- MANDATORY
  assert_single(!exist(vArgs.begin(), vArgs.end(), "-k") &&
             !exist(vArgs.begin(), vArgs.end(), "--key"),
         "no password file has been set.");
  for (auto i = vArgs.begin(); i != vArgs.end(); ++i) {
    if (*i == "-k" || *i == "--key") {
      if (i + 1 != vArgs.end() && (*(i + 1))[0] != '-') {
        check_pass(*(i + 1));
        par.key_file = *++i;
        break;
      } else
        error("no password file has been set.");
    }
  }

  // verbose, thread
  for (auto i = vArgs.begin(); i != vArgs.end(); ++i) {
    if (*i == "-v" || *i == "--verbose") {
      par.verbose = true;
    } else if ((*i == "-t" || *i == "--thread") && i + 1 != vArgs.end() &&
               (*(i + 1))[0] != '-' && is_number(*(i + 1)))
      par.n_threads = static_cast<byte>(stoi(*++i));
  }

  // Decrypt+decompress
  if (exist(vArgs.begin(), vArgs.end(), "-d") ||
      exist(vArgs.begin(), vArgs.end(), "--dec"))
    return 'd';

  // stop_shuffle, frmt
  for (auto i = vArgs.begin(); i != vArgs.end(); ++i) {
    if (*i == "-s" || *i == "--stop_shuffle")
      par.stop_shuffle = true;
    else if (*i == "-f" || *i == "--force")
      par.format = 'n';
  }
  if (!exist(vArgs.begin(), vArgs.end(), "-f") &&
      !exist(vArgs.begin(), vArgs.end(), "--force"))
    par.format = frmt(par.in_file);  // Not standard input file

  // Compress+encrypt
  return 'c';
}
}  // namespace cryfa

#endif  // CRYFA_PARSER_H