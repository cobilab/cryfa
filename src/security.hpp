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

#include <memory>//todo
#include "def.hpp"
#include "parser.hpp"//todo

/**
 * @brief Security
 */
class Security //todo: public Param
{
 public:
  Security () = default;//todo
  explicit Security (std::shared_ptr<Param>);//todo
  auto decrypt () -> void;
  
 protected:
  bool shuffInProg = true;  /**< @brief Shuffle in progress @hideinitializer */
  bool shuffled    = true;  /**< @hideinitializer */
  
  auto encrypt () -> void;
  auto shuffle (string&) -> void;
  auto unshuffle (string::iterator&, u64) -> void;
  
 private:
  u64  seed_shared;         /**< @brief Shared seed */
//    const int TAG_SIZE = 12; /**< @brief Tag size used in GCC mode auth enc */
  std::shared_ptr<Param> par;//todo
  
  auto srandom (u32) -> void;
  auto random () -> int;
  auto random_engine () -> std::minstd_rand0&;
  auto gen_shuff_seed () -> void;
  auto build_iv (byte*, const string&) -> void;
  auto build_key (byte*, const string&) -> void;

#ifdef DEBUG
  auto print_iv (byte*) const -> void;
  auto print_key (byte*) const -> void;
#endif
};

#endif //CRYFA_SECURITY_H