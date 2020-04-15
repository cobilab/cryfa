/**
 * @file      security.hpp
 * @brief     Security
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_SECURITY_H
#define CRYFA_SECURITY_H

#include "def.hpp"

namespace cryfa {
/**
 * @brief Security
 */
class Security : public Param {
 public:
  Security() = default;
  void decrypt();

 protected:
  bool shuffInProg = true; /**< @brief Shuffle in progress @hideinitializer */
  bool shuffled = true;    /**< @hideinitializer */

  void encrypt();
  void shuffle(std::string&);
  void unshuffle(std::string::iterator&, u64);

 private:
  u64 seed_shared; /**< @brief Shared seed */
  // const int TAG_SIZE = 12; /**< @brief Tag size used in GCC mode auth enc */

  void srandom(u32);
  auto random() -> int;
  auto random_engine() -> std::minstd_rand0&;
  void gen_shuff_seed();
  void build_iv(byte*, const std::string&);
  void build_key(byte*, const std::string&);

#ifdef DEBUG
  void print_iv(byte*) const;
  void print_key(byte*) const;
#endif
};
}  // namespace cryfa

#endif  // CRYFA_SECURITY_H