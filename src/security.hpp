/**
 * @file      security.hpp
 * @brief     Security
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_SECURITY_H
#define CRYFA_SECURITY_H

#include "def.hpp"

/**
 * @brief Security
 */
class Security : public InArgs
{
 public:
  auto decrypt () -> void;
  
 protected:
  bool shuffInProg = true;  /**< @brief Shuffle in progress @hideinitializer */
  bool shuffled    = true;  /**< @hideinitializer */
  
  auto encrypt () -> void;
  auto shuffle (string&) -> void;
  auto unshuffle (string::iterator&, u64) -> void;
  
 private:
  u64  seed_shared;         /**< @brief Shared seed */
//    const int TAG_SIZE = 12;  /**< @brief Tag size used in GCC mode auth enc */
  
  auto read_pass () const -> string;
  auto srand (u32) -> void;
  auto rand () -> int;
  auto rand_engine () -> std::minstd_rand0&;
  auto gen_shuff_seed () -> void;
  auto build_iv (byte*, const string&) -> void;
  auto build_key (byte*, const string&) -> void;

#ifdef DEBUG
  auto print_iv (byte*) const -> void;
  auto print_key (byte*) const -> void;
#endif
};

#endif //CRYFA_SECURITY_H