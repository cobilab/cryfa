/**
 * @file      endecrypto.hpp
 * @brief     Encryption/Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ENDECRYPTO_H
#define CRYFA_ENDECRYPTO_H

#include <chrono>

#include "security.hpp"

namespace cryfa {
class EnDecrypto;

// Type define
typedef void (EnDecrypto::*packFP_t)(std::string&, const std::string&,
                                     const htbl_t&);
typedef void (EnDecrypto::*unpackFP_t)(std::string&, std::string::iterator&,
                                       const std::vector<std::string>&);

/**
 * @brief Encryption/Decryption
 */
class EnDecrypto : public Security {
 public:
  EnDecrypto() = default;

  void pack_hL_fa_fq(std::string&, const std::string&, const htbl_t&);
  void pack_qL_fq(std::string&, const std::string&, const htbl_t&);
  void pack_3to2(std::string&, const std::string&, const htbl_t&);
  void pack_2to1(std::string&, const std::string&, const htbl_t&);
  void pack_3to1(std::string&, const std::string&, const htbl_t&);
  void pack_5to1(std::string&, const std::string&, const htbl_t&);
  void pack_7to1(std::string&, const std::string&, const htbl_t&);
  void pack_1to1(std::string&, const std::string&, const htbl_t&);
  void unpack_2B(std::string&, std::string::iterator&,
                 const std::vector<std::string>&);
  void unpack_1B(std::string&, std::string::iterator&,
                 const std::vector<std::string>&);
  void shuffle_file();
  void unshuffle_file();

 protected:
  std::string Hdrs;  /**< @brief Max: 39 values */
  std::string QSs;   /**< @brief Max: 39 values */
  std::string HdrsX; /**< @brief Extended Hdrs */
  std::string QSsX;  /**< @brief Extended QSs */
  htbl_t HdrMap;     /**< @brief Hdrs hash table */
  htbl_t QsMap;      /**< @brief QSs hash table */
  u32 BlockLine;     /**< @brief Max block lines */
  std::chrono::time_point<std::chrono::high_resolution_clock> shuffle_timer;

  void build_hash_tbl(htbl_t&, const std::string&, short);
  void build_unpack_tbl(std::vector<std::string>&, const std::string&, u16);
  auto dna_pack_idx(const std::string&) -> byte;
  auto large_pack_idx(const std::string&, const htbl_t&) -> u16;
  void pack_seq(std::string&, const std::string&);
  void unpack_seq(std::string&, std::string::iterator&);
  void unpack_large(std::string&, std::string::iterator&, char,
                    const std::vector<std::string>&);
  void join_packed_files(const std::string&, const std::string&, char,
                         bool) const;
  void join_unpacked_files() const;
  void join_shuffled_files() const;
  void join_unshuffled_files() const;

 private:
  void pack_large(std::string&, const std::string&, const std::string&,
                  const htbl_t&);
  auto penalty_sym(char) const -> char;
  void shuffle_block(byte);
  void unshuffle_block(byte);
};

/**
 * @brief Hash table for packing
 * @hideinitializer
 */
static const htbl_t DNA_MAP{
    {"AAA", 0},   {"AAC", 1},   {"AAG", 2},   {"AAT", 3},   {"AAN", 4},
    {"AAX", 5},   {"ACA", 6},   {"ACC", 7},   {"ACG", 8},   {"ACT", 9},
    {"ACN", 10},  {"ACX", 11},  {"AGA", 12},  {"AGC", 13},  {"AGG", 14},
    {"AGT", 15},  {"AGN", 16},  {"AGX", 17},  {"ATA", 18},  {"ATC", 19},
    {"ATG", 20},  {"ATT", 21},  {"ATN", 22},  {"ATX", 23},  {"ANA", 24},
    {"ANC", 25},  {"ANG", 26},  {"ANT", 27},  {"ANN", 28},  {"ANX", 29},
    {"AXA", 30},  {"AXC", 31},  {"AXG", 32},  {"AXT", 33},  {"AXN", 34},
    {"AXX", 35},  {"CAA", 36},  {"CAC", 37},  {"CAG", 38},  {"CAT", 39},
    {"CAN", 40},  {"CAX", 41},  {"CCA", 42},  {"CCC", 43},  {"CCG", 44},
    {"CCT", 45},  {"CCN", 46},  {"CCX", 47},  {"CGA", 48},  {"CGC", 49},
    {"CGG", 50},  {"CGT", 51},  {"CGN", 52},  {"CGX", 53},  {"CTA", 54},
    {"CTC", 55},  {"CTG", 56},  {"CTT", 57},  {"CTN", 58},  {"CTX", 59},
    {"CNA", 60},  {"CNC", 61},  {"CNG", 62},  {"CNT", 63},  {"CNN", 64},
    {"CNX", 65},  {"CXA", 66},  {"CXC", 67},  {"CXG", 68},  {"CXT", 69},
    {"CXN", 70},  {"CXX", 71},  {"GAA", 72},  {"GAC", 73},  {"GAG", 74},
    {"GAT", 75},  {"GAN", 76},  {"GAX", 77},  {"GCA", 78},  {"GCC", 79},
    {"GCG", 80},  {"GCT", 81},  {"GCN", 82},  {"GCX", 83},  {"GGA", 84},
    {"GGC", 85},  {"GGG", 86},  {"GGT", 87},  {"GGN", 88},  {"GGX", 89},
    {"GTA", 90},  {"GTC", 91},  {"GTG", 92},  {"GTT", 93},  {"GTN", 94},
    {"GTX", 95},  {"GNA", 96},  {"GNC", 97},  {"GNG", 98},  {"GNT", 99},
    {"GNN", 100}, {"GNX", 101}, {"GXA", 102}, {"GXC", 103}, {"GXG", 104},
    {"GXT", 105}, {"GXN", 106}, {"GXX", 107}, {"TAA", 108}, {"TAC", 109},
    {"TAG", 110}, {"TAT", 111}, {"TAN", 112}, {"TAX", 113}, {"TCA", 114},
    {"TCC", 115}, {"TCG", 116}, {"TCT", 117}, {"TCN", 118}, {"TCX", 119},
    {"TGA", 120}, {"TGC", 121}, {"TGG", 122}, {"TGT", 123}, {"TGN", 124},
    {"TGX", 125}, {"TTA", 126}, {"TTC", 127}, {"TTG", 128}, {"TTT", 129},
    {"TTN", 130}, {"TTX", 131}, {"TNA", 132}, {"TNC", 133}, {"TNG", 134},
    {"TNT", 135}, {"TNN", 136}, {"TNX", 137}, {"TXA", 138}, {"TXC", 139},
    {"TXG", 140}, {"TXT", 141}, {"TXN", 142}, {"TXX", 143}, {"NAA", 144},
    {"NAC", 145}, {"NAG", 146}, {"NAT", 147}, {"NAN", 148}, {"NAX", 149},
    {"NCA", 150}, {"NCC", 151}, {"NCG", 152}, {"NCT", 153}, {"NCN", 154},
    {"NCX", 155}, {"NGA", 156}, {"NGC", 157}, {"NGG", 158}, {"NGT", 159},
    {"NGN", 160}, {"NGX", 161}, {"NTA", 162}, {"NTC", 163}, {"NTG", 164},
    {"NTT", 165}, {"NTN", 166}, {"NTX", 167}, {"NNA", 168}, {"NNC", 169},
    {"NNG", 170}, {"NNT", 171}, {"NNN", 172}, {"NNX", 173}, {"NXA", 174},
    {"NXC", 175}, {"NXG", 176}, {"NXT", 177}, {"NXN", 178}, {"NXX", 179},
    {"XAA", 180}, {"XAC", 181}, {"XAG", 182}, {"XAT", 183}, {"XAN", 184},
    {"XAX", 185}, {"XCA", 186}, {"XCC", 187}, {"XCG", 188}, {"XCT", 189},
    {"XCN", 190}, {"XCX", 191}, {"XGA", 192}, {"XGC", 193}, {"XGG", 194},
    {"XGT", 195}, {"XGN", 196}, {"XGX", 197}, {"XTA", 198}, {"XTC", 199},
    {"XTG", 200}, {"XTT", 201}, {"XTN", 202}, {"XTX", 203}, {"XNA", 204},
    {"XNC", 205}, {"XNG", 206}, {"XNT", 207}, {"XNN", 208}, {"XNX", 209},
    {"XXA", 210}, {"XXC", 211}, {"XXG", 212}, {"XXT", 213}, {"XXN", 214},
    {"XXX", 215}};

/**
 * @brief Lookup table for unpacking -- 216 elements
 * @hideinitializer
 */
static const std::vector<std::string> DNA_UNPACK{
    "AAA", "AAC", "AAG", "AAT", "AAN", "AAX", "ACA", "ACC", "ACG", "ACT", "ACN",
    "ACX", "AGA", "AGC", "AGG", "AGT", "AGN", "AGX", "ATA", "ATC", "ATG", "ATT",
    "ATN", "ATX", "ANA", "ANC", "ANG", "ANT", "ANN", "ANX", "AXA", "AXC", "AXG",
    "AXT", "AXN", "AXX", "CAA", "CAC", "CAG", "CAT", "CAN", "CAX", "CCA", "CCC",
    "CCG", "CCT", "CCN", "CCX", "CGA", "CGC", "CGG", "CGT", "CGN", "CGX", "CTA",
    "CTC", "CTG", "CTT", "CTN", "CTX", "CNA", "CNC", "CNG", "CNT", "CNN", "CNX",
    "CXA", "CXC", "CXG", "CXT", "CXN", "CXX", "GAA", "GAC", "GAG", "GAT", "GAN",
    "GAX", "GCA", "GCC", "GCG", "GCT", "GCN", "GCX", "GGA", "GGC", "GGG", "GGT",
    "GGN", "GGX", "GTA", "GTC", "GTG", "GTT", "GTN", "GTX", "GNA", "GNC", "GNG",
    "GNT", "GNN", "GNX", "GXA", "GXC", "GXG", "GXT", "GXN", "GXX", "TAA", "TAC",
    "TAG", "TAT", "TAN", "TAX", "TCA", "TCC", "TCG", "TCT", "TCN", "TCX", "TGA",
    "TGC", "TGG", "TGT", "TGN", "TGX", "TTA", "TTC", "TTG", "TTT", "TTN", "TTX",
    "TNA", "TNC", "TNG", "TNT", "TNN", "TNX", "TXA", "TXC", "TXG", "TXT", "TXN",
    "TXX", "NAA", "NAC", "NAG", "NAT", "NAN", "NAX", "NCA", "NCC", "NCG", "NCT",
    "NCN", "NCX", "NGA", "NGC", "NGG", "NGT", "NGN", "NGX", "NTA", "NTC", "NTG",
    "NTT", "NTN", "NTX", "NNA", "NNC", "NNG", "NNT", "NNN", "NNX", "NXA", "NXC",
    "NXG", "NXT", "NXN", "NXX", "XAA", "XAC", "XAG", "XAT", "XAN", "XAX", "XCA",
    "XCC", "XCG", "XCT", "XCN", "XCX", "XGA", "XGC", "XGG", "XGT", "XGN", "XGX",
    "XTA", "XTC", "XTG", "XTT", "XTN", "XTX", "XNA", "XNC", "XNG", "XNT", "XNN",
    "XNX", "XXA", "XXC", "XXG", "XXT", "XXN", "XXX"};
}  // namespace cryfa

#endif  // CRYFA_ENDECRYPTO_H