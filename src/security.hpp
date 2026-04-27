/**
 * @file security.hpp
 * @brief Security
 * @author Morteza Hosseini (seyedmorteza.hosseini@manchester.ac.uk)
 * @author Diogo Pratas (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_SECURITY_H
#define CRYFA_SECURITY_H

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

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
  using PlaintextSink = std::function<void(std::string_view)>;
  using PlaintextProducer = std::function<void(const PlaintextSink&)>;

  bool shuffInProg = true; /**< @brief Shuffle in progress @hideinitializer */
  bool shuffled = true;    /**< @hideinitializer */

  void encrypt();
  void encrypt_stream(const PlaintextProducer&);
  void shuffle(std::string&);
  void unshuffle(std::string::iterator&, u64);

 private:
  static constexpr size_t AES_KEY_SIZE = 16;
  static constexpr size_t AES_IV_SIZE = 16;

  struct DerivedState {
    std::array<byte, AES_KEY_SIZE> key{};
    std::array<byte, AES_IV_SIZE> iv{};
    u64 shuffle_seed = 0;
  };

  static std::mutex derived_state_mutex;
  static std::unordered_map<std::string, std::shared_ptr<const DerivedState>> derived_state_cache;

  std::mutex unshuffle_cache_mutex;
  std::unordered_map<u64, std::shared_ptr<const std::vector<u64>>> unshuffle_cache;

  void srandom(u32);
  auto random() -> int;
  auto random_engine() -> std::minstd_rand0&;
  auto derived_state() -> std::shared_ptr<const DerivedState>;
  auto build_shuff_seed(const std::string&) -> u64;
  auto unshuffle_positions(u64) -> std::shared_ptr<const std::vector<u64>>;
  void build_iv(byte*, const std::string&);
  void build_key(byte*, const std::string&);

#ifdef DEBUG
  void print_iv(byte*) const;
  void print_key(byte*) const;
#endif
};
}  // namespace cryfa

#endif  // CRYFA_SECURITY_H
