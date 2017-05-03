/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Encryption / Decryption
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <fstream>
#include "EnDecrypto.h"
#include "pack.h"
// cryptopp modules
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"
#include "cryptopp/eax.h"
using std::cout;
using std::cerr;
using std::ifstream;
using CryptoPP::AES;
using CryptoPP::CBC_Mode_ExternalCipher;
using CryptoPP::StreamTransformationFilter;

/*******************************************************************************
    constructor
*******************************************************************************/
EnDecrypto::EnDecrypto () {}

/*******************************************************************************
    encrypt fasta
*******************************************************************************/
void EnDecrypto::encryptFA (int argc, char **argv, int v_flag,
                            string keyFileName)
{
    // AES encryption uses a secret key of a variable length (128, 196 or 256
    // bit). This key is secretly exchanged between two parties before
    // communication begins. DEFAULT_KEYLENGTH= 16 bytes
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, AES::DEFAULT_KEYLENGTH);  // AES key
    memset(iv, 0x00, AES::BLOCKSIZE);           // Initialization Vector
    
    string password = getPasswordFromFile(keyFileName);
    buildKey(key, password);
    buildIV(iv, password);
    
    printIV(iv);
    printKey(key);
    
    ifstream input( argv[argc-1] );
    string line, header, dna_seq, header_and_dna_seq;
    
    if (!input.good())
    {
        cerr << "Error opening '" << argv[ argc - 1 ] << "'.\n";
        exit(1);
    }
    
    while (std::getline(input, line).good())
    {
        if (line.empty() || line[ 0 ] == '>')
        {   // FASTA identifier
            if (!header.empty())
            {   // print out last entry
                //cout << ">" header << '\n' << dna_seq << '\n'; //todo. debug
                header_and_dna_seq +=
                        (">" + header + "\n" + PackIn3bDNASeq(dna_seq));
                header.clear();
            }
            if (!line.empty())  header = line.substr(1);
            dna_seq.clear();
        }
        else if (!header.empty())
        {
            if (line.find(' ') != string::npos)
            {   // invalid sequence--no spaces allowed
                header.clear();
                dna_seq.clear();
            }
            else    dna_seq += (line + "\n");
        }
    }
    
    input.close();
    
    header_and_dna_seq += '<';  // the rest is as it is
    
    // last entry handling
    if (!header.empty())
    {
        // cout << ">" header << '\n' << dna_seq << std::endl; //todo. debug
        header_and_dna_seq += (">" + header + "\n" + line);
    }
    
    // // DO RANDOM SHUFFLE:
    // srand(0);
    // std::random_shuffle(header_and_dna_seq.begin(),header_and_dna_seq.end());
    // * NEED TO KNOW THE REVERSE OF SHUFFLE, FOR DECRYPT!
    
    header_and_dna_seq += '<'; // KNOW WHERE IS END ON DECRYPTION
    
    string ciphertext;
    AES::Encryption aesEncryption(key, AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
    StreamTransformationFilter stfEncryptor(cbcEncryption,
                                            new CryptoPP::StringSink(ciphertext));
    stfEncryptor.Put(reinterpret_cast<const unsigned char *>
                     (header_and_dna_seq.c_str()), header_and_dna_seq.length() + 1);
    stfEncryptor.MessageEnd();
    
    if (v_flag)
    {
        cerr << "   sym size: " << header_and_dna_seq.size() << '\n';
        cerr << "cipher size: " << ciphertext.size() << '\n';
        cerr << " block size: " << AES::BLOCKSIZE << '\n';
    }
    
    // WATERMAK FOR ENCRYPTED FASTA FILE
    cout << "#cryfa v" << VERSION_CRYFA << "." << RELEASE_CRYFA << '\n';
    
    // DUMP CYPHERTEXT FOR READ
    for (ULL i = 0; i < ciphertext.size(); ++i)
        cout << (char) (0xFF & static_cast<byte>(ciphertext[ i ]));
    cout << '\n';
    
    header_and_dna_seq.clear();
    ciphertext.clear();
    keyFileName.clear();
}

/*******************************************************************************
    decrypt FASTA
*******************************************************************************/
void EnDecrypto::decryptFA (int argc, char **argv, int v_flag, string keyFileName)
{
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, AES::DEFAULT_KEYLENGTH); // AES KEY
    memset(iv, 0x00, AES::BLOCKSIZE);     // INITIALLIZATION VECTOR
    
    string password = getPasswordFromFile(keyFileName);
    buildKey(key, password);
    buildIV(iv, password);
    
    printIV(iv);
    printKey(key);
    
    string line, decryptedtext;
    ifstream input(argv[ argc - 1 ]);
    
    if (!input.good())
    {
        cerr << "Error opening '" << argv[ argc - 1 ] << "'.\n";
        exit(1);
    }
    
    string ciphertext((std::istreambuf_iterator< char >(input)),
                      std::istreambuf_iterator< char >());
    
    // string watermark = "#cryfa v1.1\n";
    string watermark = "#cryfa v" + std::to_string(VERSION_CRYFA)
                       + "." + std::to_string(RELEASE_CRYFA) + "\n";
    
    string::size_type i = ciphertext.find(watermark);
    
    if (i == string::npos)
    {
        cerr << "Error: invalid encrypted file!\n";
        exit(1);
    }
    else
        ciphertext.erase(i, watermark.length());
    
    if (v_flag)
    {
        cerr << "cipher size: " << ciphertext.size() - 1 << '\n';
        cerr << " block size: " << AES::BLOCKSIZE << '\n';
    }
    
    AES::Decryption aesDecryption(key, AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    StreamTransformationFilter stfDecryptor(cbcDecryption,
                                            new CryptoPP::StringSink(decryptedtext));
    stfDecryptor.Put(reinterpret_cast<const unsigned char *>
                     (ciphertext.c_str()), ciphertext.size() - 1);
    stfDecryptor.MessageEnd();
    
    // Dump Decrypted Text
    cerr << "Decrypting... \n";
    
    bool header = true, first, second, third;
    unsigned char s;
    string triplet;
    char trp0, trp1, trp2;
    
    for (ULL i = 0; i < ciphertext.size(); ++i)
    {
        s = decryptedtext[ i ];
        
        if (s == '<')
        { // REACHED END
            while ((s = decryptedtext[ ++i ]) != '<')
                cout << s;
            cout << '\n';
            return;
        }
        
        if (header)
        {
            cout << s;
            if (s == '\n')
                header = false;
            continue;
        }
        
        if (s == '>')
        {
            header = true;
            cout << s;
        }
        
        if (!header)
        {
            //cerr << (int) s << ":" << DNA_UNPACK[(int) s];
            
            if (s == 244)
            { // EXTRA CHARS % SIZE
                while ((s = decryptedtext[ i ]) != '>' && s != '<')
                {
                    if (s != 244)
                        cout << s;
                    ++i;
                }
                --i;
                continue;
            }
            
            triplet = DNA_UNPACK[ (int) s ];
            //cout << triplet;
            
            first = false, second = false, third = false;
            trp0 = triplet[ 0 ], trp1 = triplet[ 1 ], trp2 = triplet[ 2 ];
            
            if (trp0 == 'X')
                first = true;
            if (trp1 == 'X')
                second = true;
            if (trp2 == 'X')
                third = true;
            
            if (!first && !second && !third)
                cout << triplet;
            
            else if (first && !second && !third)
                cout << decryptedtext[ ++i ] << trp1 << trp2;
            
            else if (!first && second && !third)
                cout << trp0 << decryptedtext[ ++i ] << trp2;
            
            else if (!first && !second && third)
                cout << trp0 << trp1 << decryptedtext[ ++i ];
            
            else if (first && second && !third)
                cout << decryptedtext[ ++i ] << decryptedtext[ ++i ] << trp2;
            
            else if (first && !second && third)
                cout << decryptedtext[ ++i ] << trp1 << decryptedtext[ ++i ];
            
            else if (!first && second && third)
                cout << trp0 << decryptedtext[ ++i ] << decryptedtext[ ++i ];
            
            else
                cout << decryptedtext[ ++i ] << decryptedtext[ ++i ]
                     << decryptedtext[ ++i ];
        }
    }
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::buildIV (byte *iv, string pwd)
{
    std::uniform_int_distribution< rng_type::result_type > udist(0, 255);
    rng_type rng;
    
    evaluatePasswordSize(pwd);
    
    // USING OLD RAND TO GENERATE THE NEW RAND SEED
    srand(7919 * (pwd[ 2 ] * pwd[ 5 ]) + 75653);
    ULL seed = 0;
    for (int i = 0; i < pwd.size(); ++i)
        seed += ((ULL) pwd[ i ] * rand()) + rand();
    seed %= 4294967295;
    
    rng_type::result_type const seedval = seed;
    rng.seed(seedval);
    
    for (int i = 0; i < AES::BLOCKSIZE; ++i)
        iv[ i ] = udist(rng) % 255;
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::buildKey (byte *key, string pwd)
{
    std::uniform_int_distribution< rng_type::result_type > udist(0, 255);
    rng_type rng;
    
    evaluatePasswordSize(pwd);
    
    // USING OLD RAND TO GENERATE THE NEW RAND SEED
    srand(24593 * (pwd[ 0 ] * pwd[ 2 ]) + 49157);
    ULL seed = 0;
    for (int i = 0; i < pwd.size(); ++i)
        seed += ((ULL) pwd[ i ] * rand()) + rand();
    seed %= 4294967295;
    
    rng_type::result_type const seedval = seed;
    rng.seed(seedval);
    
    for (int i = 0; i < AES::DEFAULT_KEYLENGTH; ++i)
        key[ i ] = udist(rng) % 255;
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::printIV (byte *iv)
{
    cerr << "IV : [";
    for (int i = 0; i < AES::BLOCKSIZE; ++i)
        cerr << (int) iv[ i ] << " ";
    cerr << "]\n";
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::printKey (byte *key)
{
    cerr << "KEY: [";
    for (int i = 0; i < AES::DEFAULT_KEYLENGTH; ++i)
        cerr << (int) key[ i ] << " ";
    cerr << "]\n";
}

/*******************************************************************************
    get password from a file
*******************************************************************************/
inline string EnDecrypto::getPasswordFromFile (string keyFileName)
{
    ifstream input(keyFileName);
    string line;
    
    if (keyFileName == "")
    {
        cerr << "Error: no password file has been set!\n";
        exit(1);
    }
    
    if (!input.good())
    {
        cerr << "Error opening '" << keyFileName << "'." << std::endl;
        exit(1);
    }
    
    while (std::getline(input, line).good())
    {
        if (line.empty())
        {
            cerr << "Error: empty password line file!\n";
            exit(1);
        }
        return line;
    }
    
    return "unknown";
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::evaluatePasswordSize (string pwd)
{
    if (pwd.size() < 8)
    {
        cerr << "Error: password is too short!\n";
        exit(1);
    }
}