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
using std::getline;
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
void EnDecrypto::encryptFA (int argc, char **argv, const int v_flag,
                            const string &keyFileName)
{
    // AES encryption uses a secret key of a variable length (128, 196 or
    // 256 bit). This key is secretly exchanged between two parties before
    // communication begins. DEFAULT_KEYLENGTH= 16 bytes
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv, 0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string password = getPasswordFromFile(keyFileName);
    buildKey(key, password);
    buildIV(iv, password);
    
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    ifstream input(argv[argc-1]);
    string line, header, dna_seq, header_and_dna_seq;
    
    if (!input.good())
    {
        cerr << "Error opening '" << argv[argc-1] << "'.\n";
        exit(1);
    }

//    while (getline(input, line).good())
//    {
//        if (line[0] == '>' || line.empty())     // FASTA identifier
//        {
//            if (!header.empty())    // 2nd header line onwards
//            {   // print out last entry
//                //cout << ">" header << '\n' << dna_seq << '\n'; //todo. debug
//                header_and_dna_seq +=
//                        ">" + header + "\n" + PackIn3bDNASeq(dna_seq);
//                header.clear();
//            }
//            if (!line.empty())  header = line.substr(1);
//            dna_seq.clear();
//        }
//        else if (!header.empty())
//        {   // invalid sequence--no spaces allowed
//            if (line.find(' ') != string::npos)
//            {
//                header.clear();
//                dna_seq.clear();
//            }
//            else    dna_seq += line + "\n";
//        }
//    }
    
    
    while (getline(input, line).good())
    {
        if (line[0] == '>')
        {
            if (!dna_seq.empty()) header_and_dna_seq += PackIn3bDNASeq(dna_seq);
            header_and_dna_seq += (char) 253 + line.substr(1) + "\n";
            dna_seq.clear();
        }
        else if (line.empty())
        {
            dna_seq += (char) 252;
        }
        else
        {
            if (line.find(' ') != string::npos)
            {
                cerr << "Invalid sequence -- spaces not allowed.\n";
                return;
            }
            dna_seq += line + (char) 254;
        }
    }
    if (!dna_seq.empty())   header_and_dna_seq += PackIn3bDNASeq(dna_seq);
    
    input.close();

    
//    // last entry handling
//    if (!header.empty())
//    {
////        // cout << ">" header << '\n' << dna_seq << std::endl; //todo. debug
////        header_and_dna_seq += (">" + header + "\n" + line);
//        header_and_dna_seq +=
//                ">" + header + "\n" + PackIn3bDNASeq(dna_seq);
//    }
//
    // // do random shuffle
    // srand(0);
    // std::random_shuffle(header_and_dna_seq.begin(),header_and_dna_seq.end());
    // * need to know the reverse of shuffle, for decryption!

    
//    header_and_dna_seq += "<";  // specify the end for decryption
    
    string ciphertext;
    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
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
    
    // watermark for encrypted FASTA file
    cout << "#cryfa v" << VERSION_CRYFA << "." << RELEASE_CRYFA << '\n';
    
    // dump cyphertext for read
    for (ULL i = 0; i < ciphertext.size(); ++i)
        cout << (char) (0xFF & static_cast<byte>( ciphertext[i] ));
    cout << '\n';
    
//    header_and_dna_seq.clear();
//    ciphertext.clear();
//    keyFileName.clear();
}

/*******************************************************************************
    decrypt FASTA
*******************************************************************************/
void EnDecrypto::decryptFA (int argc, char **argv, const int v_flag,
                            const string &keyFileName)
{
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv, 0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string password = getPasswordFromFile(keyFileName);
    buildKey(key, password);
    buildIV(iv, password);

//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    string line, decText;
    ifstream input(argv[argc - 1]);
    if (!input.good())
    {
        cerr << "Error opening '" << argv[argc - 1] << "'.\n";
        exit(1);
    }
    
    string ciphertext( (std::istreambuf_iterator<char> (input)),
                        std::istreambuf_iterator<char> () );
    
    // string watermark = "#cryfa v1.1\n";
    string watermark = "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                                  + std::to_string(RELEASE_CRYFA) + "\n";
    
    string::size_type i = ciphertext.find(watermark);
    if (i == string::npos)
    {
        cerr << "Error: invalid encrypted file!\n";
        exit(1);
    }
    else  ciphertext.erase(i, watermark.length());
    
    if (v_flag)
    {
        cerr << "cipher size: " << ciphertext.size() - 1 << '\n';
        cerr << " block size: " << AES::BLOCKSIZE << '\n';
    }
    
    AES::Decryption aesDecryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    StreamTransformationFilter stfDecryptor(cbcDecryption,
                                       new CryptoPP::StringSink(decText));
    stfDecryptor.Put(reinterpret_cast<const unsigned char *>
                     (ciphertext.c_str()), ciphertext.size() - 1);
    stfDecryptor.MessageEnd();
    
//    // dump decrypted text
//    cerr << "Decrypting... \n";
    
    bool isHeader = true, firstIsX, secondIsX, thirdIsX;
    unsigned char s;
    string triplet;
    char trp0, trp1, trp2;
    const ULL decTxtSize = decText.size() - 1;    //todo. check '-1'

//    for (ULL j = 0; j < ciphertext.size(); ++j)
    for (ULL j = 0; j < decTxtSize; ++j)
    {
        s = (unsigned char) decText[j];
        
//        if (s == '<')
//        {   // reached the end
//            while ((s = decText[++j]) != '<')   cout << s;
//            cout << '\n';
//            return;
//        }
    
        if (s == 252)   // empty line
        {
            cout << '\n';
            continue;
        }
        
        if (s == 255)   // length of seq line not multiplicant of 3
        {
            cout << penaltySym( decText, ++j );
            continue;
        }
        
        if (s == 254 && !isHeader) // end of each sequence line
        {
            cout << '\n';
            continue;
        }

        if (s == 253)   // header line
        {
            cout << '>';
            isHeader = true;
            continue;
        }
        if (isHeader)   // header line
        {
            cout << s;
            if (s == '\n')  isHeader = false;
            continue;
        }
        
        if (!isHeader)
        {
            //cerr << (int) s << ":" << DNA_UNPACK[(int) s];

//            if (s == 244)
//            {   // extra chars % size
//                while ((s = decText[j]) != '>' && s != '<')
//                {
//                    if (s != 244)   cout << s;
//                    ++j;
//                }
//                --j;
//                continue;
//            }

            triplet = DNA_UNPACK[(int) s];

            firstIsX = false, secondIsX = false, thirdIsX = false;
            trp0 = triplet[0], trp1 = triplet[1], trp2 = triplet[2];

            if (trp0 == 'X')    firstIsX  = true;
            if (trp1 == 'X')    secondIsX = true;
            if (trp2 == 'X')    thirdIsX  = true;

            if ( !(firstIsX || secondIsX || thirdIsX) )         // ...
            {
                cout << triplet;
            }
            else if ( !(!firstIsX || secondIsX || thirdIsX) )   // X..
            {
                cout << penaltySym( decText, ++j );
                cout << trp1;
                cout << trp2;
            }
            else if ( !(firstIsX || !secondIsX || thirdIsX) )   // .X.
            {
                cout << trp0;
                cout << penaltySym( decText, ++j );
                cout << trp2;
            }
            else if ( !(!firstIsX || !secondIsX || thirdIsX) )  // XX.
            {
                cout << penaltySym( decText, ++j );
                cout << penaltySym( decText, ++j );
                cout << trp2;
            }
            else if ( !(firstIsX || secondIsX || !thirdIsX) )   // ..X
            {
                cout << trp0;
                cout << trp1;
                cout << penaltySym( decText, ++j );
            }
            else if ( !(!firstIsX || secondIsX || !thirdIsX) )  // X.X
            {
                cout << penaltySym( decText, ++j );
                cout << trp1;
                cout << penaltySym( decText, ++j );
            }
            else if ( !(firstIsX || !secondIsX || !thirdIsX) )  // .XX
            {
                cout << trp0;
                cout << penaltySym( decText, ++j );
                cout << penaltySym( decText, ++j );
            }
            else                                                // XXX
            {
                cout << penaltySym( decText, ++j );
                cout << penaltySym( decText, ++j );
                cout << penaltySym( decText, ++j );
            }
        }
    }
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::buildIV (byte *iv, string pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evaluatePasswordSize(pwd);
    
    // using old rand to generate the new rand seed
    srand((unsigned int) 7919 * (pwd[2] * pwd[5]) + 75653);
    ULL seed = 0;
    for (int i = 0; i < pwd.size(); ++i)
        seed += ((ULL) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);
    
    for (int i = 0; i < AES::BLOCKSIZE; ++i)
        iv[i] = udist(rng) % 255;
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::buildKey (byte *key, string pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evaluatePasswordSize(pwd);
    
    // USING OLD RAND TO GENERATE THE NEW RAND SEED
    srand((unsigned int) 24593 * (pwd[0] * pwd[2]) + 49157);
    ULL seed = 0;
    for (int i = 0; i < pwd.size(); ++i)
        seed += ((ULL) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);
    
    for (int i = 0; i < AES::DEFAULT_KEYLENGTH; ++i)
        key[i] = udist(rng) % 255;
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::printIV (byte *iv) const
{
    cerr << "IV : [";
    for (int i = 0; i < AES::BLOCKSIZE; ++i)
        cerr << (int) iv[i] << " ";
    cerr << "]\n";
}

/*******************************************************************************

*******************************************************************************/
inline void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [";
    for (int i = 0; i < AES::DEFAULT_KEYLENGTH; ++i)
        cerr << (int) key[i] << " ";
    cerr << "]\n";
}

/*******************************************************************************
    get password from a file
*******************************************************************************/
inline string EnDecrypto::getPasswordFromFile (const string &keyFileName) const
{
    ifstream input(keyFileName);
    string line;
    
    if (keyFileName == "")
    {
        cerr << "Error: no password file has been set!\n";
        exit(1);
    }
    else if (!input.good())
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
inline void EnDecrypto::evaluatePasswordSize (const string &pwd) const
{
    if (pwd.size() < 8)
    {
        cerr << "Error: password is too short!\n";
        exit(1);
    }
}

/*******************************************************************************
    end of each sequence line's character
*******************************************************************************/
inline char EnDecrypto::penaltySym (const string &str,
                                    const ULL idx)    const
{
    char c = str[idx];
    if (c == (char) 254 || c == (char) 252)     return '\n';
    else                                        return c;
}