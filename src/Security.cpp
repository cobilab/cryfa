/**
 * @file      Security.cpp
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
#include "Security.h"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"
#include "cryptopp/gcm.h"

using std::ifstream;
using std::cerr;
using std::to_string;
using std::chrono::high_resolution_clock;
using std::memset;
using std::setprecision;
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
 * @brief  Get password from a file
 * @return Password (string)
 */
string Security::extractPass () const
{
    ifstream in(KEY_FILE_NAME);
    char     c;
    string   pass;
    
    pass.clear();    while (in.get(c))  pass += c;
    
    in.close();
    return pass;
}

/**
 * @brief   Encrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void Security::encrypt ()
{
    cerr << "Encrypting...\n";
    auto startTime = high_resolution_clock::now();      // Start timer
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
    
    try
    {
        GCM<AES>::Encryption e;
        e.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
    
        FileSource(PCKD_FILENAME.c_str(), true,
                   new AuthenticatedEncryptionFilter(e, new FileSink(cout),
                                                     false, TAG_SIZE));
    }
    catch (CryptoPP::InvalidArgument &e)
    {
        cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
    }
    catch (CryptoPP::Exception &e)
    {
        cerr << "Caught Exception...\n" << e.what() << "\n";
    }

    auto finishTime = high_resolution_clock::now();                 //Stop timer
    std::chrono::duration<double> elapsed = finishTime - startTime; //Dur. (sec)
    cerr << (VERBOSE ? "Encryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";

    // Delete packed file
    const string pkdFileName = PCKD_FILENAME;
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
void Security::decrypt ()
{
    ifstream in(IN_FILE_NAME);
    if (!in.good())
    { cerr << "Error: failed opening \"" << IN_FILE_NAME << "\".\n";  exit(1); }

    cerr << "Decrypting...\n";
    auto startTime = high_resolution_clock::now();      // Start timer
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector

    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // Debug
//    printKey(key);    // Debug

//    string cipherText( (std::istreambuf_iterator<char> (in)),
//                       std::istreambuf_iterator<char> () );

//    if (VERBOSE)
//    {
//        cerr << "cipher size: " << cipherText.size()-1 << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
//    }
    
    try
    {
        const char* outFile = DEC_FILENAME.c_str();
        
        GCM<AES>::Decryption d;
        d.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
        
        AuthenticatedDecryptionFilter df(d, new FileSink(outFile),
                        AuthenticatedDecryptionFilter::DEFAULT_FLAGS, TAG_SIZE);
        FileSource(in, true, new Redirector(df /*, PASS_EVERYTHING */ ));
    }
    catch (CryptoPP::HashVerificationFilter::HashVerificationFailed &e)
    {
        cerr << "Caught HashVerificationFailed...\n" << e.what() << "\n";
    }
    catch (CryptoPP::InvalidArgument &e)
    {
        cerr << "Caught InvalidArgument...\n" << e.what() << "\n";
    }
    catch (CryptoPP::Exception &e)
    {
        cerr << "Caught Exception...\n" << e.what() << "\n";
    }
    
    auto finishTime = high_resolution_clock::now();                 //Stop timer
    std::chrono::duration<double> elapsed = finishTime - startTime; //Dur. (sec)
    cerr << (VERBOSE ? "Decryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    in.close();
}

/**
 * @brief    Random number seed -- Emulate C srand()
 * @param s  Seed
 */
void Security::newSrand (u32 s)
{
    randomEngine().seed(s);
}

/**
 * @brief  Random number generate -- Emulate C rand()
 * @return Random number
 */
int Security::newRand ()
{
    return (int) (randomEngine()() - randomEngine().min());
}

/**
 * @brief  Random number engine
 * @return The classic Minimum Standard rand0
 */
std::minstd_rand0 &Security::randomEngine ()
{
    static std::minstd_rand0 e{};
    return e;
}

/**
 * @brief Shuffle/unshuffle seed generator -- For each chunk
 */
void Security::shuffSeedGen ()
{
    const string pass = extractPass();
    
    u32 passDigitsSum = 0;
    for (char c : pass)    passDigitsSum += c;
    
    // Using old rand to generate the new rand seed
    u64 seed = 0;
    
    mutxSec.lock();//-----------------------------------------------------------
    newSrand(681493*passDigitsSum + 9148693);
    for (char c : pass)    seed += (u64) (c*newRand());
    mutxSec.unlock();//---------------------------------------------------------
    
    seed_shared = seed;
}

/**
 * @brief          Shuffle
 * @param[in, out] str  String to be shuffled
 */
void Security::shuffle (string &str)
{
    shuffSeedGen();    // shuffling seed
    std::shuffle(str.begin(), str.end(), std::mt19937(seed_shared));
}

/**
 * @brief       Unshuffle
 * @param i     Shuffled string iterator
 * @param size  Size of shuffled string
 */
void Security::unshuffle (string::iterator &i, u64 size)
{
    string shuffledStr;     // Copy of shuffled string
    for (u64 j=0; j!=size; ++j, ++i)    shuffledStr += *i;
    string::iterator shIt = shuffledStr.begin();
    i -= size;
    
    // Shuffle vector of positions
    vector<u64> vPos(size);
    std::iota(vPos.begin(), vPos.end(), 0);     // Insert 0 .. N-1
    shuffSeedGen();
    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed_shared));
    
    // Insert unshuffled data
    for (const u64& vI : vPos)  *(i + vI) = *shIt++;       // *shIt, then ++shIt
}

/**
 * @brief Build initialization vector (IV) for cryption
 * @param iv    IV
 * @param pass  Password
 */
void Security::buildIV (byte *iv, const string &pass)
{
    std::uniform_int_distribution<rng_t::result_type> udist(0, 255);
    rng_t rng;
    
    u32 sumEvenPass=0, sumOddPass=0;
    for (auto i = pass.begin(), j = pass.begin()+1;
         i < pass.end(), j < pass.end(); i+=2, j+=2)
    {
        sumEvenPass += *i;
        sumOddPass  += *j;
    }
    
    // Using old rand to generate the new rand seed
    newSrand((u32) (459229*sumEvenPass + 3175661*sumOddPass + 15483683));
    
    u64 seed = 0;
    for (auto i = pass.begin(); i < pass.end(); ++i)
        seed += *i * newRand() + newRand();
    
    const rng_t::result_type seedval = seed;
    rng.seed(seedval);
    
    int i = AES::BLOCKSIZE;
    u64 j = pass.size() - 1;
    while (i--)
    {
        iv[i] = (byte) ((udist(rng)*pass[j]) % 255);
        if (j--==0)    j=pass.size()-1;
    }
}

/**
 * @brief Build key for cryption
 * @param key  Key
 * @param pwd  password
 */
void Security::buildKey (byte *key, const string &pwd)
{
    std::uniform_int_distribution<rng_t::result_type> udist(0, 255);
    rng_t rng;
    
    u32 sumEvenPwd=0, sumOddPwd=0;
    for (auto i=pwd.begin(), j=pwd.begin()+1;
         i<pwd.end(), j<pwd.end(); i+=2, j+=2)
    {
        sumEvenPwd += *i;
        sumOddPwd  += *j;
    }
    
    // Using old rand to generate the new rand seed
    newSrand((u32) 24593 * (9819241*sumEvenPwd + 2597591*sumOddPwd + 648649));
    
    u64 seed = 0;
    for (auto i = pwd.begin(); i < pwd.end(); ++i)
        seed += *i * newRand() + newRand();
    
    const rng_t::result_type seedval = seed;
    rng.seed(seedval);
    
    int i = AES::DEFAULT_KEYLENGTH;
    u64 j = pwd.size()-1;
    while (i--)
    {
        key[i] = (byte) ((udist(rng)*pwd[j]) % 255);
        if (j--==0)    j=pwd.size()-1;
    }
}

/**
 * @brief Print IV
 * @param iv  IV
 */
void Security::printIV (byte *iv) const
{
    cerr << "IV = [" << (int) iv[0];
    for (u32 i = 1; i != AES::BLOCKSIZE; ++i)
        cerr << " " << (int) iv[i];
    cerr << "]\n";
}

/**
 * @brief Print key
 * @param key  Key
 */
void Security::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (u32 i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}