/**
 * @file security.cpp
 * @brief Security
 * @author Morteza Hosseini (seyedmorteza.hosseini@manchester.ac.uk)
 * @author Diogo Pratas (pratas@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include "security.hpp"

#include <fstream>
#include <iomanip>  // setw, std::setprecision
#include <mutex>
#include <numeric>  // std::accumulate, std::iota

#include "assert.hpp"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"
#include "cryptopp/gcm.h"
#include "numeric.hpp"
#include "string.hpp"
#include "time.hpp"
using namespace cryfa;

std::mutex mutxSec; /**< @brief Mutex */
std::mutex Security::derived_state_mutex;
std::unordered_map<std::string, std::shared_ptr<const Security::DerivedState>>
    Security::derived_state_cache;

/**
 * @brief Encrypt
 * @details AES encryption uses a secret key of a variable length (128, 196 or 256 bit).
 *          This key is secretly exchanged between two parties before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void Security::encrypt() {
  std::cerr << bold("[+]") << " Encrypting ...";
  const auto start = now();  // Start timer

  const auto state = derived_state();

  try {
    CryptoPP::GCM<CryptoPP::AES>::Encryption e;
    e.SetKeyWithIV(state->key.data(), state->key.size(), state->iv.data(), state->iv.size());

    CryptoPP::FileSource(PCKD_FNAME.c_str(), true,
                         new CryptoPP::AuthenticatedEncryptionFilter(
                             e, new CryptoPP::FileSink(std::cout), false, TAG_SIZE));
  } catch (CryptoPP::InvalidArgument& e) {
    std::cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
  } catch (CryptoPP::Exception& e) {
    std::cerr << "Caught Exception...\n" << e.what() << "\n";
  }

  const auto finish = now();  // Stop timer
  std::cerr << "\r" << bold("[+]") << " Encrypting done in " << hms(finish - start);

  // Delete packed file
  const std::string pkdFileName = PCKD_FNAME;
  std::remove(pkdFileName.c_str());
}

/**
 * @brief Decrypt
 * @details AES encryption uses a secret key of a variable length (128, 196 or 256 bit).
 *          This key is secretly exchanged between two parties before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void Security::decrypt() {
  assert_file_good(in_file);

  std::cerr << bold("[+]") << " Decrypting ...";
  const auto start = now();  // Start timer

  const auto state = derived_state();

  try {
    std::ifstream in(in_file);
    const char* outFile = DEC_FNAME.c_str();

    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(state->key.data(), state->key.size(), state->iv.data(), state->iv.size());

    CryptoPP::AuthenticatedDecryptionFilter df(
        d, new CryptoPP::FileSink(outFile), CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
        TAG_SIZE);
    CryptoPP::FileSource(in, true, new CryptoPP::Redirector(df /*, PASS_EVERYTHING */));
    in.close();
  } catch (CryptoPP::HashVerificationFilter::HashVerificationFailed& e) {
    std::cerr << "Caught HashVerificationFailed...\n" << e.what() << "\n";
  } catch (CryptoPP::InvalidArgument& e) {
    std::cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
  } catch (CryptoPP::Exception& e) {
    std::cerr << "Caught Exception...\n" << e.what() << "\n";
  }

  const auto finish = now();  // Stop timer
  std::cerr << "\r" << bold("[+]") << " Decrypting done in " << hms(finish - start);
}

/**
 * @brief Random number seed -- Emulate C srand()
 * @param s Seed
 */
void Security::srandom(u32 s) { random_engine().seed(s); }

/**
 * @brief Random number generate -- Emulate C rand()
 * @return Random number
 */
int Security::random() { return (int)(random_engine()() - random_engine().min()); }

/**
 * @brief Random number engine
 * @return The classic Minimum Standard rand0
 */
std::minstd_rand0& Security::random_engine() {
  static std::minstd_rand0 e{};
  return e;
}

std::shared_ptr<const Security::DerivedState> Security::derived_state() {
  std::lock_guard<std::mutex> cache_lock(derived_state_mutex);
  const auto found = derived_state_cache.find(key_file);
  if (found != derived_state_cache.end()) {
    return found->second;
  }

  auto state = std::make_shared<DerivedState>();
  const std::string pass = file_to_string(key_file);

  std::lock_guard<std::mutex> random_lock(mutxSec);
  build_key(state->key.data(), pass);
  build_iv(state->iv.data(), pass);
  state->shuffle_seed = build_shuff_seed(pass);

  return derived_state_cache.emplace(key_file, state).first->second;
}

/**
 * @brief Shuffle/unshuffle seed generator
 * @param pass Password
 * @return Shuffle seed
 */
u64 Security::build_shuff_seed(const std::string& pass) {
  // Using old rand to generate the new random seed
  u64 seed = 0;

  srandom(681493 * std::accumulate(pass.begin(), pass.end(), u32(0)) + 9148693);
  for (char c : pass) {
    seed += (u64)(c * random());
  }

  return seed;
}

std::shared_ptr<const std::vector<u64>> Security::unshuffle_positions(u64 size) {
  {
    std::lock_guard<std::mutex> cache_lock(unshuffle_cache_mutex);
    const auto found = unshuffle_cache.find(size);
    if (found != unshuffle_cache.end()) {
      return found->second;
    }
  }

  auto positions = std::make_shared<std::vector<u64>>(size);
  std::iota(positions->begin(), positions->end(), 0);
  std::shuffle(positions->begin(), positions->end(), rng_t(derived_state()->shuffle_seed));

  std::lock_guard<std::mutex> cache_lock(unshuffle_cache_mutex);
  return unshuffle_cache.emplace(size, positions).first->second;
}

/**
 * @brief Shuffle
 * @param[in,out] str String to be shuffled
 */
void Security::shuffle(std::string& str) {
  std::shuffle(str.begin(), str.end(), rng_t(derived_state()->shuffle_seed));
}

/**
 * @brief Unshuffle
 * @param i Shuffled string iterator
 * @param size Size of shuffled string
 */
void Security::unshuffle(std::string::iterator& i, u64 size) {
  std::string shuffledStr;  // Copy of shuffled std::string
  shuffledStr.reserve(size);
  for (u64 j = 0; j != size; ++j, ++i) {
    shuffledStr += *i;
  }
  auto shIt = shuffledStr.begin();
  i -= size;

  const auto positions = unshuffle_positions(size);

  // Insert unshuffled data
  for (const u64& vI : *positions) {
    *(i + vI) = *shIt++;  // *shIt, then ++shIt
  }
}

/**
 * @brief Build initialization vector (IV) for cryption
 * @param iv IV
 * @param pass Password
 */
void Security::build_iv(byte* iv, const std::string& pass) {
  std::uniform_int_distribution<rng_t::result_type> udist(0, 255);
  rng_t rng;

  // Using old rand to generate the new random seed
  srandom(static_cast<u32>(44701 * (459229 * accum_even(pass.begin(), pass.end(), 0ul) +
                                    3175661 * accum_odd(pass.begin(), pass.end(), 0ul)) +
                           499397));
  u64 seed = 0;
  for (char c : pass) {
    seed += c * random() + random();
  }

  rng.seed(static_cast<rng_t::result_type>(seed));

  for (int i = CryptoPP::AES::BLOCKSIZE; i--;) {
    iv[i] = static_cast<byte>((udist(rng) * accum_hops(pass.begin(), pass.end(), 0u, i + 1)) % 255);
  }
}

/**
 * @brief Build key for cryption
 * @param key Key
 * @param pass Password
 */
void Security::build_key(byte* key, const std::string& pass) {
  std::uniform_int_distribution<rng_t::result_type> udist(0, 255);
  rng_t rng;

  // Using old rand to generate the new random seed
  srandom(static_cast<u32>(24593 * (9819241 * accum_even(pass.begin(), pass.end(), 0ul) +
                                    2597591 * accum_odd(pass.begin(), pass.end(), 0ul)) +
                           648649));
  u64 seed = 0;
  for (char c : pass) {
    seed += c * random() + random();
  }

  rng.seed(static_cast<rng_t::result_type>(seed));

  for (int i = CryptoPP::AES::DEFAULT_KEYLENGTH; i--;) {
    key[i] =
        static_cast<byte>((udist(rng) * accum_hops(pass.begin(), pass.end(), 0u, i + 1)) % 255);
  }
}

#ifdef DEBUG
/**
 * @brief Print IV
 * @param iv IV
 */
void Security::print_iv(byte* iv) const {
  std::cerr << "IV = [" << (int)*iv++;
  for (auto i = CryptoPP::AES::BLOCKSIZE - 1; i--;) {
    std::cerr << " " << (int)*iv++;
  }
  std::cerr << "]\n";
}

/**
 * @brief Print key
 * @param key Key
 */
void Security::print_key(byte* key) const {
  std::cerr << "Key: [" << (int)*key++;
  for (auto i = CryptoPP::AES::DEFAULT_KEYLENGTH - 1; i--;) {
    std::cerr << " " << (int)*key++;
  }
  std::cerr << "]\n";
}
#endif
