/**
 * @file      endecrypto.hpp
 * @brief     Encryption/Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ENDECRYPTO_H
#define CRYFA_ENDECRYPTO_H

#include "def.hpp"
#include "security.hpp"
using std::string;
using std::vector;

class endecrypto;

// Type define
typedef void (endecrypto::*packFP_t) (string&, const string&, const htbl_t&);
typedef void (endecrypto::*unpackFP_t)
             (string&, string::iterator&, const vector<string>&);

/**
 * @brief Encryption/Decryption
 */
class endecrypto : public Security
{
 public:
  endecrypto () = default;
  
  auto pack_Lhdr_fa_fq (string&, const string&, const htbl_t&) -> void;
  auto pack_Lqs_fq (string&, const string&, const htbl_t&) -> void;
  auto pack_3to2 (string&, const string&, const htbl_t&) -> void;
  auto pack_2to1 (string&, const string&, const htbl_t&) -> void;
  auto pack_3to1 (string&, const string&, const htbl_t&) -> void;
  auto pack_5to1 (string&, const string&, const htbl_t&) -> void;
  auto pack_7to1 (string&, const string&, const htbl_t&) -> void;
  auto pack_1to1 (string&, const string&, const htbl_t&) -> void;
  auto unpack_2B (string&, string::iterator&, const vector<string>&) -> void;
  auto unpack_1B (string&, string::iterator&, const vector<string>&) -> void;
  auto shuffle_file () -> void;
  auto unshuffle_file () -> void;
    
 protected:
  string Hdrs;        /**< @brief Max: 39 values */
  string QSs;         /**< @brief Max: 39 values */
  string HdrsX;       /**< @brief Extended Hdrs */
  string QSsX;        /**< @brief Extended QSs */
  htbl_t HdrMap;      /**< @brief Hdrs hash table */
  htbl_t QsMap;       /**< @brief QSs hash table */
  u32    BlockLine;   /**< @brief Max block lines */
  
  auto build_hash_tbl (htbl_t&, const string&, short) -> void;
  auto build_unpack_tbl (vector<string>&, const string&, u16) -> void;
  auto dna_pack_index (const string&) -> byte;
  auto large_pack_index (const string&, const htbl_t&) -> u16;
  auto pack_seq (string&, const string&) -> void;
  auto unpack_seq (string&, string::iterator&) -> void;
  auto unpack_large (string&, string::iterator&, char, const vector<string>&) -> void;
  auto join_packed_files (const string&, const string&, char, bool) const -> void;
  auto join_unpacked_files () const -> void;
  auto join_shuffled_files () const -> void;
  auto join_unshuffled_files () const -> void;

 private:
  auto pack_large (string&, const string&, const string&, const htbl_t&) -> void;
  auto penalty_sym (char) const -> char;
  auto shuffle_block (byte) -> void;
  auto unshuffle_block (byte) -> void;
};

#endif //CRYFA_ENDECRYPTO_H