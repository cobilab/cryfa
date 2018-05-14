/**
 * @file      def.hpp
 * @brief     Definitions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_DEF_H
#define CRYFA_DEF_H

#include <iostream>
#include <unordered_map>    // Hash table
#include <random>           // std::mt19937
using std::cout;
using std::string;
using std::unordered_map;
using std::vector;

// Version
static const string MONTH     = "05";
static const string YEAR      = "18";
static const string VERSION   = YEAR + "." + MONTH;
static const string DEV_YEARS = "2017-2018";

// Typedefs
using byte   = unsigned char;
using u16    = unsigned short;
using u32    = unsigned int;
using u64    = unsigned long long;
using i64    = long long;
using rng_t  = std::mt19937;
using htbl_t = std::unordered_map<string, u64>;
using pos_t  = std::char_traits<char>::pos_type; /**< @brief tellg(), tellp() */

// Metaprograms
/**
 * Power (B^E). Usage: "cerr << POWER<3,2>::val;" which yields 9
 * @tparam  B  Base
 * @tparam  E  Exponent
 * @warning Base (B) and exponent (E) MUST be known at compile time.
 */
template<u32 B, u32 E>
struct POWER { static const u64 val = B * POWER<B, E-1>::val; };

/** @cond SHOW_HIDDEN */
template<u32 B>
struct POWER<B, 0> { static const u64 val = 1; };
/** @endcond */

// Macros
#define LOOP(i,S)                 for(char (i) : (S))
#define LOOP2(i,j,S)              LOOP(i,S) LOOP(j,S)
#define LOOP3(i,j,k,S)            LOOP(i,S) LOOP(j,S) LOOP(k,S)
#define LOOP4(i,j,k,l,S)          LOOP(i,S) LOOP(j,S) LOOP2(k,l,S)
#define LOOP5(i,j,k,l,m,S)        LOOP(i,S) LOOP(j,S) LOOP3(k,l,m,S)
#define LOOP6(i,j,k,l,m,n,S)      LOOP(i,S) LOOP(j,S) LOOP4(k,l,m,n,S)
#define LOOP7(i,j,k,l,m,n,o,S)    LOOP(i,S) LOOP(j,S) LOOP5(k,l,m,n,o,S)
#define LOOP8(i,j,k,l,m,n,o,p,S)  LOOP(i,S) LOOP(j,S) LOOP6(k,l,m,n,o,p,S)
#define IGNORE_THIS_LINE(in) \
        (in).ignore(std::numeric_limits<std::streamsize>::max(),'\n')

// Constants
static const string THR_ID_HDR = "THRD=";     /**< @brief Thread ID header */
static const string PK_FNAME   = "CRYFA_PK";  /**< @brief Packed file name */
static const string PCKD_FNAME = "CRYFA_PCKD";/**< @brief Pckd f name - joined*/
static const string SH_FNAME   = "CRYFA_SH";  /**< @brief Shuffed file name */
static const string DEC_FNAME  = "CRYFA_DEC"; /**< @brief Decrypted file name */
static const string UPK_FNAME  = "CRYFA_UPK"; /**< @brief Unpacked file name */
static const string USH_FNAME  = "CRYFA_USH"; /**< @brief Unshuffled file name*/
constexpr byte DEFAULT_N_THR   = 8;   /**< @brief Default number of threads */
constexpr u64  BLOCK_SIZE      = 8 * 1024; /**< @brief To read from input file*/
constexpr byte C1              = 2;   /**< @brief       Cat 1  =  2 */
constexpr byte C2              = 3;   /**< @brief       Cat 2  =  3 */
constexpr byte MIN_C3          = 4;   /**< @brief  4 <= Cat 3 <=  6 */
constexpr byte MID_C3          = 5;
constexpr byte MAX_C3          = 6;
constexpr byte MIN_C4          = 7;   /**< @brief  7 <= Cat 4 <= 15 */
constexpr byte MAX_C4          = 15;
constexpr byte MIN_C5          = 16;  /**< @brief 16 <= Cat 5 <= 39 */
constexpr byte MAX_C5          = 39;
constexpr byte KEYLEN_C1       = 7;   /**< @brief 7 to 1 byte. Build hash table */
constexpr byte KEYLEN_C2       = 5;   /**< @brief 5 to 1 byte */
constexpr byte KEYLEN_C3       = 3;   /**< @brief 3 to 1 byte */
constexpr byte KEYLEN_C4       = 2;   /**< @brief 2 to 1 byte */
constexpr byte KEYLEN_C5       = 3;   /**< @brief 3 to 2 byte */
constexpr int  TAG_SIZE        = 12;  /**< @brief GCC mode auth enc */

/** @brief Command line input arguments */
struct Param {
  static bool   verbose;          /**< @brief Verbose mode */
  static bool   disable_shuffle;  /**< @brief Disable shuffle */
  static byte   n_threads;        /**< @brief Number of threads */
  static string in_file;          /**< @brief Input file name */
  static string key_file;         /**< @brief Password file name */
  static char   format;           /**< @brief Format of the input file */
};

/**
 * @brief Usage guide
 */
inline void Help () {
  cout                                                                  << '\n'
       << "NAME"                                                        << '\n'
       << "      Cryfa v" << VERSION << " - "
       <<                    "A secure encryption tool for genomic data"<< '\n'
                                                                        << '\n'
       << "AUTHORS"                                                     << '\n'
       << "      Morteza Hosseini    seyedmorteza@ua.pt"                << '\n'
       << "      Diogo Pratas        pratas@ua.pt"                      << '\n'
       << "      Armando J. Pinho    ap@ua.pt"                          << '\n'
                                                                        << '\n'
       << "SYNOPSIS"                                                    << '\n'
       << "      ./cryfa [OPTION]... -k [key_file] [-d] [in_file] "
       <<                                                "> [OUT_FILE]" << '\n'
                                                                        << '\n'
       << "SAMPLE"                                                      << '\n'
       << "      Encrypt and Compact:    ./cryfa -k pass.txt in.fq "
       <<                                                  "> comp"     << '\n'
       << "      Decrypt and Unpack:     ./cryfa -k pass.txt -d comp "
       <<                                                  "> orig.fq"  << '\n'
                                                                        << '\n'
       << "      Encrypt:                ./cryfa -k pass.txt in > enc"  << '\n'
       << "      Decrypt:                ./cryfa -k pass.txt -d enc > "
       <<                                                        "orig" << '\n'
                                                                        << '\n'
       << "DESCRIPTION"                                                 << '\n'
       << "      Compact & encrypt FASTA/FASTQ files."                  << '\n'
       << "      Encrypt any text-based genomic data."                  << '\n'
                                                                        << '\n'
       << "      The key_file specifies a file including the password." << '\n'
                                                                        << '\n'
       << "      -h,  --help"                                           << '\n'
       << "           usage guide"                                      << '\n'
                                                                        << '\n'
       << "      -k [key_file],  --key [key_file]"                      << '\n'
       << "           key file name -- MANDATORY"                       << '\n'
                                                                        << '\n'
       << "      -d,  --dec"                                            << '\n'
       << "           decrypt & unpack"                                 << '\n'
                                                                        << '\n'
       << "      -f,  --format"                                         << '\n'
       << "           force specified format "
                                   "('a':FASTA, 'q':FASTQ, 'n':others)" << '\n'
                                                                        << '\n'
       << "      -v,  --verbose"                                        << '\n'
       << "           verbose mode (more information)"                  << '\n'
                                                                        << '\n'
       << "      -s,  --disable_shuffle"                                << '\n'
       << "           disable input shuffling"                          << '\n'
                                                                        << '\n'
       << "      -t [NUMBER],  --thread [NUMBER]"                       << '\n'
       << "           number of threads"                                << '\n'
                                                                        << '\n'
       << "COPYRIGHT"                                                   << '\n'
       << "      Copyright (C) " << DEV_YEARS << ", IEETA, University "
       <<                                                  "of Aveiro." << '\n'
       << "      This is a Free software, under GPLv3. You may redistribute \n"
       << "      copies of it under the terms of the GNU - General Public   \n"
       << "      License v3 <http://www.gnu.org/licenses/gpl.html>. There   \n"
       << "      is NOT ANY WARRANTY, to the extent permitted by law."  << '\n';
  
//  exit(1);//todo
  throw EXIT_SUCCESS;
}

#endif //CRYFA_DEF_H