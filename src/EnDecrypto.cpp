/**
 * @file      EnDecrypto.cpp
 * @brief     Encryption/Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include "EnDecrypto.h"
#include "cryptopp/aes.h"
#include "cryptopp/eax.h"
#include "cryptopp/files.h"

using std::vector;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::to_string;
using std::thread;
using std::stoull;
using std::chrono::high_resolution_clock;
using std::setprecision;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::CBC_Mode;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::FileSource;
using CryptoPP::FileSink;

std::mutex mutxEnDe;    /**< @brief Mutex */

/**
 * @brief   Encrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void EnDecrypto::encrypt ()
{
    cerr << "Encrypting...\n";
    
    // Start timer for encryption
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // Debug
//    printKey(key);    // Debug
    
    // Encrypt
    const char* inFile = PCKD_FILENAME;
    CBC_Mode<CryptoPP::AES>::Encryption
            cbcEnc(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(inFile, true,
               new StreamTransformationFilter(cbcEnc, new FileSink(cout)));
    
    // Stop timer for encryption
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Encryption duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    
    cerr << (verbose ? "Encryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    // Delete packed file
    const string pkdFileName = PCKD_FILENAME;
    std::remove(pkdFileName.c_str());
    
    /*
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = extractPass();
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    string cipherText;
    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
    StreamTransformationFilter
        stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipherText));
    stfEncryptor.Put(reinterpret_cast<const byte*>
                     (context.c_str()), context.length() + 1);
    stfEncryptor.MessageEnd();

//    if (verbose)
//    {
//        cerr << "   sym size: " << context.size()    << '\n';
//        cerr << "cipher size: " << cipherText.size() << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE    << '\n';
//    }
    
    string encryptedText;
    for (const char &c : cipherText)
        encryptedText += (char) (c & 0xFF);
////        encryptedText += (char) (0xFF & static_cast<byte> (c));

////    encryptedText+='\n';
    return encryptedText;
    */
}

/**
 * @brief   Decrypt
 * @details AES encryption uses a secret key of a variable length (128, 196
 *          or 256 bit). This key is secretly exchanged between two parties
 *          before communication begins.
 *
 *          DEFAULT_KEYLENGTH = 16 bytes.
 */
void EnDecrypto::decrypt ()
{
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening \"" << inFileName << "\".\n";    exit(1); }
    
    // Watermark
    string watermark = "#cryfa v";
    watermark += to_string(VERSION_CRYFA);    watermark += ".";
    watermark += to_string(RELEASE_CRYFA);    watermark += "\n";
    
    // Invalid encrypted file
    string line;    getline(in, line);
    if ((line + "\n") != watermark)
    {
        cerr << "Error: \"" << inFileName << '"'
             << " is not a valid file encrypted by cryfa.\n";
        exit(1);
    }
    
////    string::size_type watermarkIdx = cipherText.find(watermark);
////    if (watermarkIdx == string::npos)
////    { cerr << "Error: invalid encrypted file!\n";    exit(1); }
////    else  cipherText.erase(watermarkIdx, watermark.length());
    
    cerr << "Decrypting...\n";
    
    // Start timer for decryption
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
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

//    if (verbose)
//    {
//        cerr << "cipher size: " << cipherText.size()-1 << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
//    }
    
    const char* outFile = DEC_FILENAME;
    CBC_Mode<CryptoPP::AES>::Decryption
            cbcDec(key, (size_t) AES::DEFAULT_KEYLENGTH, iv);
    FileSource(in, true,
               new StreamTransformationFilter(cbcDec, new FileSink(outFile)));
    
    // Stop timer for decryption
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // Decryption duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    
    cerr << (verbose ? "Decryption done," : "Done,") << " in "
         << std::fixed << setprecision(4) << elapsed.count() << " seconds.\n";
    
    in.close();
}

/**
 * @brief  Check if the third line of FASTQ file contains only +
 * @return True or false
 */
bool EnDecrypto::hasFQjustPlus () const
{
    ifstream in(inFileName);
    string   line;
    
    IGNORE_THIS_LINE(in);    // Ignore header
    IGNORE_THIS_LINE(in);    // Ignore seq
    bool justPlus = !(getline(in, line).good() && line.length() > 1);
    
    in.close();
    return justPlus;
    
    /* If input was string, instead of file
    // check if the third line contains only +
    bool justPlus = true;
    string::const_iterator lFFirst = std::find(in.begin(), in.end(), '\n');
    string::const_iterator lFSecond = std::find(lFFirst+1, in.end(), '\n');
    if (*(lFSecond+2) != '\n')  justPlus = false;   // check symbol after +
    */
}

/**
 * @brief      Build a hash table
 * @param[out] map     Hash table
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::buildHashTable (htbl_t &map, const string &strIn, short keyLen)
{
    u64 elementNo = 0;
    string element;    element.reserve(keyLen);
    map.clear();
    map.reserve((u64) std::pow(strIn.size(), keyLen));
    
    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                map.insert(make_pair(element, elementNo++));
////            map.insert({element, elementNo++});
////            map[element] = elementNo++;
            }
            break;
    
        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        default: break;
    }
    
    
    // Test
//    for (htbl_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << "elementNo = " << elementNo << '\n';
}

/**
 * @brief      Build a table for unpacking
 * @param[out] unpack  Table (vector of strings)
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::buildUnpack (vector<string> &unpack, const string &strIn,
                              u16 keyLen)
{
    string element;    element.reserve(keyLen);
    unpack.clear();
    unpack.reserve((u64) std::pow(strIn.size(), keyLen));
    
    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                unpack.push_back(element);
            }
            break;

        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                unpack.push_back(element);
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                unpack.push_back(element);
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                unpack.push_back(element);
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                unpack.push_back(element);
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                unpack.push_back(element);
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                unpack.push_back(element);
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                unpack.push_back(element);
            }
            break;
            
        default: break;
    }
    
    
//    // Test
//    u64 arrSize = (u64) std::pow(strIn.size(), keyLen);
//    for (u64 i = 0; i != arrSize; ++i)
//        cerr << unpack[i] << '\n';
}

/**
 * @brief  Index of each DNA bases pack
 * @param  key  Key
 * @return Value (based on the idea of key-value in a hash table)
 */
byte EnDecrypto::dnaPack (const string &key)   // Maybe byte <-> u16 replacement
{
    htbl_t::const_iterator got = DNA_MAP.find(key);
    if (got == DNA_MAP.end())
    { cerr << "Error: key '" << key << "'not found!\n";    return 0; }
    else  return (byte) got->second;
}

/**
 * @brief  Index of each pack, when # > 39
 * @param  key  Key
 * @param  map  Hash table
 * @return Value (based on the idea of key-value in a hash table)
 */
u16 EnDecrypto::largePack (const string &key, const htbl_t &map)
{
    htbl_t::const_iterator got = map.find(key);
    if (got == map.end())
    { cerr << "Error: key '" << key << "' not found!\n";    return 0; }
    else  return (u16) got->second;
}

/**
 * @brief      Encapsulate each 3 DNA bases in 1 byte. Reduction: ~2/3
 * @param[out] packedSeq  Packed sequence
 * @param[in]  seq        Sequence
 */
void EnDecrypto::packSeq_3to1 (string &packedSeq, const string &seq)
{
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    string tuple;   tuple.reserve(3);
    string::const_iterator i = seq.begin(),   iEnd = seq.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);
        
        tuple.clear();
        tuple +=
           (firstNotIn = (s0!='A' && s0!='C' && s0!='G' && s0!='T' && s0!='N'))
           ? 'X' : s0;
        tuple +=
           (secondNotIn = (s1!='A' && s1!='C' && s1!='G' && s1!='T' && s1!='N'))
           ? 'X' : s1;
        tuple +=
           (thirdNotIn = (s2!='A' && s2!='C' && s2!='G' && s2!='T' && s2!='N'))
           ? 'X' : s2;
        
        packedSeq += dnaPack(tuple);
        if (firstNotIn)  packedSeq += s0;
        if (secondNotIn) packedSeq += s1;
        if (thirdNotIn)  packedSeq += s2;
    }
    
    // If seq len isn't multiple of 3, add (char) 255 before each sym
    switch (seq.length() % 3)
    {
        case 1:
            packedSeq+=(char) 255;    packedSeq+=*i;
            break;

        case 2:
            packedSeq+=(char) 255;    packedSeq+=*i;
            packedSeq+=(char) 255;    packedSeq+=*(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 3 header symbols in 2 bytes, when # >= 40.
 *             Reduction ~1/3
 * @param[out] packed  Packed header
 * @param[in]  strIn   Header
 * @param[in]  map     Hash table
 */
void EnDecrypto::packLargeHdr_3to2 (string &packed, const string &strIn,
                                    const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    u16 shortTuple;
    string hdrs = Hdrs;
    // ASCII char after the last char in HEADERS string
    const char XChar = (char) (hdrs.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (hdrs.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (hdrs.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (hdrs.find(s2)==string::npos)) ? XChar : s2;
    
        shortTuple = largePack(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // Left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // Right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;
        
        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;
        
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 quality score symbols in 2 bytes, when # >= 40.
 *             Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Quality scores
 * @param[in]  map     Hash table
 */
void EnDecrypto::packLargeQs_3to2 (string &packed, const string &strIn,
                                   const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    u16 shortTuple;
    string qss = QSs;
    // ASCII char after the last char in QUALITY_SCORES string
    const char XChar = (char) (qss.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),  s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (qss.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (qss.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (qss.find(s2)==string::npos)) ? XChar : s2;
        
        shortTuple = largePack(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // Left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // Right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;
        
        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;
        
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 symbols in 2 bytes, when 16 <= # <= 39.
 *             Reduction ~1/3
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_3to2 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    u16 shortTuple;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        shortTuple = (u16) map.find(tuple)->second;
        packed += (byte) (shortTuple >> 8);      // Left byte
        packed += (byte) (shortTuple & 0xFF);    // Right byte
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 2 symbols in 1 byte, when 7 <= # <= 15.
 *             Reduction ~1/2
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_2to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(2);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-1;
    
    for (; i < iEnd; i += 2)
    {
        tuple.clear();    tuple = *i;    tuple += *(i+1);
        packed += (char) map.find(tuple)->second;
    }
    
    // If len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (strIn.length() & 1) { packed+=(char) 255;    packed+=*i; }
}

/**
 * @brief         Encapsulate 3 symbols in 1 byte, when # = 4, 5, 6.
 *                Reduction ~2/3
 * @param packed  Packed string
 * @param strIn   Input string
 * @param map     Hash table
 */
void EnDecrypto::pack_3to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;

    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 5 symbols in 1 byte, when # = 3. Reduction ~4/5
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_5to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(5);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-4;
    
    for (; i < iEnd; i += 5)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);
        tuple+=*(i+2);    tuple+=*(i+3);    tuple+=*(i+4);
        packed += (char) map.find(tuple)->second;
    }
    
    // If len isn't multiple of 5, add (char) 255 before each sym
    switch (strIn.length() % 5)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;
            
        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;
            
        case 3:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            break;

        case 4:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 7 symbols in 1 byte, when # = 2. Reduction ~6/7
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_7to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(7);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-6;
    
    for (; i < iEnd; i += 7)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);    tuple+=*(i+2);
        tuple+=*(i+3);    tuple+=*(i+4);    tuple+=*(i+5);    tuple+=*(i+6);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 7, add (char) 255 before each sym
    switch (strIn.length() % 7)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        case 3:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            break;

        case 4:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            break;

        case 5:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            packed+=(char) 255;    packed+=*(i+4);
            break;

        case 6:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            packed+=(char) 255;    packed+=*(i+4);
            packed+=(char) 255;    packed+=*(i+5);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 1 symbol in 1 byte, when # = 1.
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_1to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string single;    single.reserve(1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end();
    
    for (; i < iEnd; ++i)
    {
        single.clear();   single = *i;
        packed += (char) map.find(single)->second;
    }
}

/**
 * @brief  Penalty symbol
 * @param  c  Input char
 * @return Input char or (char)10='\\n'
 */
char EnDecrypto::penaltySym (char c)
{
    const char lookupTable[2] = {c, (char) 10};
    return lookupTable[c==(char) 254 || c==(char) 252];

//    // More readable; Perhaps slower, because of conditional branch
//    return (c != (char) 254 && c != (char) 252) ? c : (char) 10;
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte, when # > 39
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  XChar   Extra character for unpacking
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpackLarge_read2B (string &out, string::iterator &i,
                                     char XChar, const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;                      // Double byte
    string tpl;    tpl.reserve(3);    // Tuplet
    out.clear();

    while (*i != (char) 254)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    i+=2; }
        else
        {
            leftB   = (byte) *i;
            rightB  = (byte) *(i+1);
            doubleB = leftB<<8 | rightB;    // Join two bytes
            
            tpl = unpack[doubleB];
            
            if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]!=XChar)          // ...
            { out+=tpl;                                                  i+=2; }
            
            else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]!=XChar)     // X..
            { out+=penaltySym(*(i+2));    out+=tpl[1];    out+=tpl[2];   i+=3; }
            
            else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]!=XChar)     // .X.
            { out+=tpl[0];    out+=penaltySym(*(i+2));    out+=tpl[2];   i+=3; }
            
            else if (tpl[0]==XChar && tpl[1]==XChar && tpl[2]!=XChar)     // XX.
            { out+=penaltySym(*(i+2));  out+=penaltySym(*(i+3));  out+=tpl[2];
                                                                         i+=4; }
            
            else if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]==XChar)     // ..X
            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(i+2));   i+=3; }
            
            else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]==XChar)     // X.X
            { out+=penaltySym(*(i+2));  out+=tpl[1];  out+=penaltySym(*(i+3));
                                                                         i+=4; }
            
            else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]==XChar)     // .XX
            { out+=tpl[0];  out+=penaltySym(*(i+2));  out+=penaltySym(*(i+3));
                                                                         i+=4; }
            
            else { out+=penaltySym(*(i+2));    out+=penaltySym(*(i+3));   // XXX
                   out+=penaltySym(*(i+4));                              i+=5; }
        }
    }
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_read2B (string &out, string::iterator &i,
                                const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;     // Double byte
    out.clear();
    
    for (; *i != (char) 254; i += 2)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    continue; }
        
        leftB   = (byte) *i;
        rightB  = (byte) *(i+1);
        doubleB = leftB<<8 | rightB;    // Join two bytes
        
        out += unpack[doubleB];
    }
}

/**
 * @brief      Unpack by reading 1 byte by 1 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_read1B (string &out, string::iterator &i,
                                const vector<string> &unpack)
{
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(++i));    continue; }
        out += unpack[(byte) *i];
    }
}

/**
 * @brief      Unpack 1 byte to 3 DNA bases
 * @param[out] out  DNA bases
 * @param[in]  i    Input string iterator
 */
void EnDecrypto::unpackSeq (string &out, string::iterator &i)
{
    string tpl;    tpl.reserve(3);     // Tuplet
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        if (*i == (char) 255)                       // Seq len not multiple of 3
            out += penaltySym(*(++i));
        else
        {
            tpl = DNA_UNPACK[(byte) *i];
            
            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')                // ...
            { out+=tpl;                                                        }
                // Using just one 'out' makes trouble
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')           // X..
            { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2];         }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')           // .X.
            { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2];         }
            
            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')           // XX.
            { out+=penaltySym(*(++i));  out+=penaltySym(*(++i));  out+=tpl[2]; }
            
            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')           // ..X
            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i));         }
            
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')           // X.X
            { out+=penaltySym(*(++i));  out+=tpl[1];  out+=penaltySym(*(++i)); }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')           // .XX
            { out+=tpl[0];  out+=penaltySym(*(++i));  out+=penaltySym(*(++i)); }
            
            else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));   // XXX
                   out+=penaltySym(*(++i));                                    }
        }
    }
}

/**
 * @brief  Random number engine
 * @return The classic Minimum Standard rand0
 */
std::minstd_rand0 &EnDecrypto::randomEngine ()
{
    static std::minstd_rand0 e{};
    return e;
}

/**
 * @brief    Random number seed -- Emulate C srand()
 * @param s  Seed
 */
void EnDecrypto::my_srand (u32 s)
{
    randomEngine().seed(s);
}

/**
 * @brief  Random number generate -- Emulate C rand()
 * @return Random number
 */
int EnDecrypto::my_rand ()
{
    return (int) (randomEngine()() - randomEngine().min());
}

/**
 * @brief Shuffle/unshuffle seed generator -- For each chunk
 */
//inline u64 EnDecrypto::un_shuffleSeedGen (const u32 seedInit)
void EnDecrypto::un_shuffleSeedGen ()
{
    const string pass = extractPass();
    
    u64 passDigitsMult = 1;    // Multiplication of all pass digits
    for (u32 i = (u32) pass.size(); i--;)    passDigitsMult *= pass[i];
    
    // Using old rand to generate the new rand seed
    u64 seed = 0;
    
    mutxEnDe.lock();//----------------------------------------------------------
//    my_srand(20543 * seedInit * (u32) passDigitsMult + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += ((u64) pass[i] * my_rand()) + my_rand();

//    my_srand(20543 * seedInit + 81647);
//    for (byte i = (byte) pass.size(); i--;)
//        seed += (u64) pass[i] * my_rand();
    my_srand(20543 * (u32) passDigitsMult + 81647);
    for (byte i = (byte) pass.size(); i--;)
        seed += (u64) pass[i] * my_rand();
    mutxEnDe.unlock();//--------------------------------------------------------
    
//    seed %= 2106945901;
 
    seed_shared = seed;
//    return seed;
}

/**
 * @brief          Shuffle
 * @param[in, out] str  String to be shuffled
 */
void EnDecrypto::shufflePkd (string &str)
{
//    const u64 seed = un_shuffleSeedGen((u32) in.size());    // Shuffling seed
//    std::shuffle(in.begin(), in.end(), std::mt19937(seed));
    un_shuffleSeedGen();    // shuffling seed
    std::shuffle(str.begin(), str.end(), std::mt19937(seed_shared));
}

/**
 * @brief       Unshuffle
 * @param i     Shuffled string iterator
 * @param size  Size of shuffled string
 */
void EnDecrypto::unshufflePkd (string::iterator &i, u64 size)
{
    string shuffledStr;     // Copy of shuffled string
    for (u64 j = 0; j != size; ++j, ++i)    shuffledStr += *i;
    string::iterator shIt = shuffledStr.begin();
    i -= size;
    
    // Shuffle vector of positions
    vector<u64> vPos(size);
    std::iota(vPos.begin(), vPos.end(), 0);     // Insert 0 .. N-1
//    const u64 seed = un_shuffleSeedGen((u32) size);
//    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed));
    un_shuffleSeedGen();
    std::shuffle(vPos.begin(), vPos.end(), std::mt19937(seed_shared));

    // Insert unshuffled data
    for (const u64& vI : vPos)  *(i + vI) = *shIt++;       // *shIt, then ++shIt
}

/**
 * @brief Build initialization vector (IV) for cryption
 * @param iv    IV
 * @param pass  Password
 */
void EnDecrypto::buildIV (byte *iv, const string &pass)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    // Using old rand to generate the new rand seed
    my_srand((u32) 7919 * pass[2] * pass[5] + 75653);
//    srand((u32) 7919 * pass[2] * pass[5] + 75653);
    u64 seed = 0;
    for (byte i = (byte) pass.size(); i--;)
        seed += ((u64) pass[i] * my_rand()) + my_rand();
//    seed += ((u64) pass[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (u32 i = (u32) AES::BLOCKSIZE; i--;)
        iv[i] = (byte) (udist(rng) % 255);
}

/**
 * @brief Build key for cryption
 * @param key  Key
 * @param pwd  password
 */
void EnDecrypto::buildKey (byte *key, const string &pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    // Using old rand to generate the new rand seed
    my_srand((u32) 24593 * (pwd[0] * pwd[2]) + 49157);
//    srand((u32) 24593 * (pwd[0] * pwd[2]) + 49157);
    u64 seed = 0;
    for (byte i = (byte) pwd.size(); i--;)
        seed += ((u64) pwd[i] * my_rand()) + my_rand();
//    seed += ((u64) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);

    for (u32 i = (u32) AES::DEFAULT_KEYLENGTH; i--;)
        key[i] = (byte) (udist(rng) % 255);
}

/**
 * @brief Print IV
 * @param iv  IV
 */
void EnDecrypto::printIV (byte *iv) const
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
void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (u32 i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}

/**
 * @brief  Get password from a file
 * @return Password (string)
 */
string EnDecrypto::extractPass () const
{
    ifstream in(keyFileName);
    char     c;
    string   pass;
    pass.clear();
    
    while (in.get(c))    pass += c;
    
    in.close();
    return pass;
}