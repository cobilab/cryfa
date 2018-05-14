/**
 * @file      security.cpp
 * @brief     Security
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <mutex>
#include <cstring>
#include <iomanip>      // setw, setprecision
#include "security.hpp"
#include "fn.hpp"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"
#include "cryptopp/gcm.h"
using std::ifstream;
using std::wifstream;
using std::cerr;
using std::to_string;
using std::chrono::high_resolution_clock;
using std::memset;
using std::setprecision;
using std::uniform_int_distribution;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::CBC_Mode;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::FileSource;
using CryptoPP::FileSink;
using CryptoPP::Redirector;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;
using CryptoPP::GCM;

std::mutex mutxSec;    /**< @brief Mutex */

/**
 * @brief   Encrypt
 * @details AES encryption uses a secret key of a variable length (128, 196 or
 *          256 bit). This key is secretly exchanged between two parties before
 *          communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void Security::encrypt () {
  cerr << "Encrypting...\n";
  const auto start = high_resolution_clock::now();  // Start timer
  
  byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
  memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH);   // AES key
  memset(iv,  0x00, (size_t) AES::BLOCKSIZE);           // Initialization Vector
  
  const string pass = file_to_string(key_file);
  build_key(key, pass);
  build_iv(iv, pass);
  
  try {
    GCM<AES>::Encryption e;
    e.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
  
    FileSource(PCKD_FNAME.c_str(), true,
               new AuthenticatedEncryptionFilter(e, new FileSink(cout),
                                                 false, TAG_SIZE));
  }
  catch (CryptoPP::InvalidArgument& e) {
    cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
  }
  catch (CryptoPP::Exception& e) {
    cerr << "Caught Exception...\n" << e.what() << "\n";
  }

  const auto finish = high_resolution_clock::now();        // Stop timer
  std::chrono::duration<double> elapsed = finish - start;  // Dur. (sec)
  cerr << (verbose ? "Encryption done," : "Done,") << " in "
       << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

  // Delete packed file
  const string pkdFileName = PCKD_FNAME;
  std::remove(pkdFileName.c_str());
}

/**
 * @brief   Decrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void Security::decrypt () {
  assert_file_good(in_file, "Error: failed opening \"" + in_file + "\".\n");

  cerr << "Decrypting...\n";
  const auto start = high_resolution_clock::now();// Start timer
  
  byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
  memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
  memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector

  const string pass = file_to_string(key_file);
  build_key(key, pass);
  build_iv(iv, pass);
  
  try {
    ifstream in(in_file);
    const char* outFile = DEC_FNAME.c_str();
    
    GCM<AES>::Decryption d;
    d.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
    
    AuthenticatedDecryptionFilter df(d, new FileSink(outFile),
                        AuthenticatedDecryptionFilter::DEFAULT_FLAGS, TAG_SIZE);
    FileSource(in, true, new Redirector(df /*, PASS_EVERYTHING */ ));
    in.close();
  }
  catch (CryptoPP::HashVerificationFilter::HashVerificationFailed& e) {
    cerr << "Caught HashVerificationFailed...\n" << e.what() << "\n";
  }
  catch (CryptoPP::InvalidArgument& e) {
    cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
  }
  catch (CryptoPP::Exception& e) {
    cerr << "Caught Exception...\n" << e.what() << "\n";
  }
  
  const auto finish = high_resolution_clock::now();        // Stop timer
  std::chrono::duration<double> elapsed = finish - start;  // Dur. (sec)
  cerr << (verbose ? "Decryption done," : "Done,") << " in "
       << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
}

/**
 * @brief Random number seed -- Emulate C srand()
 * @param s  Seed
 */
void Security::srandom (u32 s) {
  random_engine().seed(s);
}

/**
 * @brief  Random number generate -- Emulate C rand()
 * @return Random number
 */
int Security::random () {
  return (int) (random_engine()() - random_engine().min());
}

/**
 * @brief  Random number engine
 * @return The classic Minimum Standard rand0
 */
std::minstd_rand0 &Security::random_engine () {
  static std::minstd_rand0 e{};
  return e;
}

/**
 * @brief Shuffle/unshuffle seed generator -- For each chunk
 */
void Security::gen_shuff_seed () {
  const string pass = file_to_string(key_file);
  
  // Using old rand to generate the new random seed
  u64 seed = 0;
  
  mutxSec.lock();//-----------------------------------------------------------
  srandom(681493*std::accumulate(pass.begin(), pass.end(), u32(0))+9148693);
  for (char c : pass)    seed += (u64) (c*random());
  mutxSec.unlock();//---------------------------------------------------------
  
  seed_shared = seed;
}

/**
 * @brief Shuffle
 * @param[in, out] str  String to be shuffled
 */
void Security::shuffle (string& str) {
  gen_shuff_seed();    // shuffling seed
  std::shuffle(str.begin(), str.end(), rng_t(seed_shared));
}

/**
 * @brief Unshuffle
 * @param i     Shuffled string iterator
 * @param size  Size of shuffled string
 */
void Security::unshuffle (string::iterator& i, u64 size) {
  string shuffledStr;     // Copy of shuffled string
  for (u64 j=0; j!=size; ++j, ++i)    shuffledStr += *i;
  auto shIt = shuffledStr.begin();
  i -= size;
  
  // Shuffle vector of positions
  vector<u64> vPos(size);
  std::iota(vPos.begin(), vPos.end(), 0);     // Insert 0 .. N-1
  gen_shuff_seed();
  std::shuffle(vPos.begin(), vPos.end(), rng_t(seed_shared));
  
  // Insert unshuffled data
  for (const u64& vI : vPos)  *(i + vI) = *shIt++;       // *shIt, then ++shIt
}

/**
 * @brief Build initialization vector (IV) for cryption
 * @param iv    IV
 * @param pass  Password
 */
void Security::build_iv (byte* iv, const string& pass) {
  uniform_int_distribution<rng_t::result_type> udist(0, 255);
  rng_t rng;
  
  // Using old rand to generate the new random seed
  srandom(static_cast<u32>(44701*
          (459229*accum_even(pass.begin(), pass.end(), 0ul)+
           3175661*accum_odd(pass.begin(), pass.end(), 0ul)) + 499397));
  u64 seed = 0;
  for (char c : pass)
    seed += c*random() + random();
  
  rng.seed(static_cast<rng_t::result_type>(seed));
  
  for (int i=AES::BLOCKSIZE; i--;)
    iv[i] = static_cast<byte>(
      (udist(rng) * accum_hops(pass.begin(), pass.end(), 0u, i+1)) % 255);
}

/**
 * @brief Build key for cryption
 * @param key   Key
 * @param pass  password
 */
void Security::build_key (byte*key, const string& pass) {
  uniform_int_distribution<rng_t::result_type> udist(0, 255);
  rng_t rng;
  
  // Using old rand to generate the new random seed
  srandom(static_cast<u32>(24593*
          (9819241*accum_even(pass.begin(), pass.end(), 0ul) +
           2597591*accum_odd(pass.begin(), pass.end(), 0ul)) + 648649));
  u64 seed = 0;
  for (char c : pass)
    seed += c*random() + random();
  
  rng.seed(static_cast<rng_t::result_type>(seed));
  
  for (int i=AES::DEFAULT_KEYLENGTH; i--;)
    key[i] = static_cast<byte>(
      (udist(rng) * accum_hops(pass.begin(), pass.end(), 0u, i+1)) % 255);
}

#ifdef DEBUG
/**
 * @brief Print IV
 * @param iv  IV
 */
void Security::print_iv (byte* iv) const {
  cerr << "IV = [" << (int) *iv++;
  for (auto i=AES::BLOCKSIZE-1; i--;)
    cerr << " " << (int) *iv++;
  cerr << "]\n";
}

/**
 * @brief Print key
 * @param key  Key
 */
void Security::print_key (byte* key) const {
  cerr << "Key: [" << (int) *key++;
  for (auto i=AES::DEFAULT_KEYLENGTH-1; i--;)
    cerr << " " << (int) *key++;
  cerr << "]\n";
}
#endif