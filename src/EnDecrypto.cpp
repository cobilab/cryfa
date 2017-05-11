/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Encryption / Decryption
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <fstream>
#include <functional>
#include "EnDecrypto.h"
#include "pack.h"
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
    encrypt FASTA.
    reserved symbols:
    FASTA:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each sequence line
        (char) 253:  instead of '>' in header
        (char) 252:  instead of empty line
*******************************************************************************/
void EnDecrypto::encryptFA (int argc, char **argv, const int v_flag,
                            const string &keyFileName)
{
    ifstream in(argv[argc-1]);
    string line;            // each line of file
    string header, seq;     // header and sequence (FASTA/FASTQ)
    string qs;              // quality scores (FASTQ)
    // FASTA: context = header + seq (+ empty lines)
    // FASTQ: context = header + seq + plus + qs
    string context;
    
    if (!in.good())
    {
        cerr << "Error: failed opening '" << argv[argc-1] << "'.\n";
        return;
    }
    
    // FASTA
    if (findFileType(in) == 'A')
    {
        while (getline(in, line).good())
        {
            // header
            if (line[0] == '>')
            {
                if (!seq.empty())   // previous seq
                    context += packSeq_3to1(seq);
                seq.clear();

                // header line. (char) 253 instead of '>'
                context += (char) 253 + line.substr(1) + "\n";
            }

            // empty line. (char) 252 instead of line feed
            else if (line.empty())  seq += (char) 252;

            // sequence
            else
            {
                if (line.find(' ') != string::npos)
                {
                    cerr << "Invalid sequence -- spaces not allowed.\n";
                    return;
                }
                // (char) 254 instead of '\n' at the end of each seq line
                seq += line + (char) 254;
            }
        }
        if (!seq.empty())   context += packSeq_3to1(seq);  // the last seq
    }
    
    // FASTQ
    else //if (findFileType(in) == 'Q')
    {
        // what quality scores are in the file
        string qsRange;
        string QUALITY_SCORES;              // can have at most 40 values
        
        while(!in.eof())
        {
            in.ignore(LARGE_NUMBER, '\n');  // ignore header
            in.ignore(LARGE_NUMBER, '\n');  // ignore seq
            in.ignore(LARGE_NUMBER, '\n');  // ignore +
            if (getline(in, line).good())   // quality score
            {
                for (string::iterator i = line.begin(); i != line.end(); ++i)
                    if (qsRange.find_first_of(*i) == string::npos)
                        qsRange += *i;
            }
            else;
        }
        in.clear();  in.seekg(0, std::ios::beg);            // beginning of file
        
        std::sort(qsRange.begin(), qsRange.end());          // sort ASCII values
    
        const size_t qsRangeLen = qsRange.length();
        // if len > 40 filter the last 40 ones
        QUALITY_SCORES = (qsRangeLen > 40) ? qsRange.substr(qsRangeLen - 40)
                                           : qsRange;
        
        std::function<string(string)> packQS;               // function wrapper
        if (qsRangeLen > 15)                                // 16 <= #QS <= 40
        {
//            buildQsHashTable(QUALITY_SCORES, 3);
//            packQS = packQS_3to2;
        }
        else if (qsRangeLen > 6)                            // 7 <= #QS <= 15
        {
            buildQsHashTable(QUALITY_SCORES, 2);
            packQS = packQS_2to1;
        }
        else if (qsRangeLen == 6 || qsRangeLen == 5
                                 || qsRangeLen == 4)        // #QS = 4 or 5 or 6
        {
            buildQsHashTable(QUALITY_SCORES, 3);
            packQS = packQS_3to1;
        }
        else if (qsRangeLen == 3)                           // #QS = 3
        {
            buildQsHashTable(QUALITY_SCORES, 5);
            packQS = packQS_5to1;
        }
        else if (qsRangeLen == 2)                           // #QS = 2
        {
            buildQsHashTable(QUALITY_SCORES, 7);
            packQS = packQS_7to1;
        }
    
        // test
//        cerr << qsRange << '\n' << qsRange.length() << '\n';
//        cerr << QUALITY_SCORES<<'\n';
    
        
    
        //todo. nabas havijoori 'context+=' nevesht,
        //todo. chon va3 file 10GB mitereke
        //todo. bas hame kara ro block by block anjam dad
        
        ULL lineNo = 0;
        while(!in.eof())    // process 4 lines by 4 lines
        {
            if (getline(in, line).good())    // header
            {
//                ++lineNo;
//                context += line;
            }
            if (getline(in, line).good())    // sequence
            {
//                ++lineNo;
//                seq += pack3seq(line);
//                context += pack3seq(line);
//                context += line;
            }
            if (getline(in, line).good())    // +
            {
//                ++lineNo;
//                context += line;
            }
            if (getline(in, line).good())    // quality score
            {
//                ++lineNo;
                context += packQS(line);
//                context += line;
            }
        }
        
        // test
        cerr << context;
        
    }
    
    in.close();
    
//    // cryptography
//    // AES encryption uses a secret key of a variable length (128, 196 or
//    // 256 bit). This key is secretly exchanged between two parties before
//    // communication begins. DEFAULT_KEYLENGTH= 16 bytes
//    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
//    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
//    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
//
//    const string pass = getPassFromFile(keyFileName);
//    buildKey(key, pass);
//    buildIV(iv, pass);
////    printIV(iv);      // debug
////    printKey(key);    // debug
//
////     // do random shuffle
////     srand(0);
////     std::random_shuffle(context.begin(),context.end());
////     * need to know the reverse of shuffle, for decryption!
//
//    string cipherText;
//    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
//    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
//    StreamTransformationFilter stfEncryptor(cbcEncryption,
//                                          new CryptoPP::StringSink(cipherText));
//    stfEncryptor.Put(reinterpret_cast<const byte*>
//                     (context.c_str()), context.length() + 1);
//    stfEncryptor.MessageEnd();
//
//    if (v_flag)
//    {
//        cerr << "   sym size: " << context.size() << '\n';
//        cerr << "cipher size: " << cipherText.size() << '\n';
//        cerr << " block size: " << AES::BLOCKSIZE    << '\n';
//    }
//
//    // watermark for encrypted file
//    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
//                       + std::to_string(RELEASE_CRYFA) + "\n";
//   // todo. QUALITY_SCORES should be added to encrypted file, after watermark
//
//    // dump cyphertext for read
//    for (ULL i = 0; i < cipherText.size(); ++i)
//        cout << (char) (0xFF & static_cast<byte> (cipherText[i]));
//    cout << '\n';
}

/*******************************************************************************
    decrypt FASTA.
    reserved symbols:
    FASTA:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each sequence line
        (char) 253:  instead of '>' in header
        (char) 252:  instead of empty line
*******************************************************************************/
void EnDecrypto::decryptFA (int argc, char **argv, const int v_flag,
                            const string &keyFileName)
{
    // cryptography
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector
    
    const string pass = getPassFromFile(keyFileName);
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug
    
    string line, decText;
    ifstream in(argv[argc-1]);
    
    if (!in.good())
    {
        cerr << "Error: failed opening '" << argv[argc-1] << "'.\n";
        return;
    }
    
    string cipherText( (std::istreambuf_iterator<char> (in)),
                        std::istreambuf_iterator<char> () );
    
    // watermark
    string watermark = "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                                  + std::to_string(RELEASE_CRYFA) + "\n";
    
    string::size_type i = cipherText.find(watermark);
    if (i==string::npos) { cerr << "Error: invalid encrypted file!\n"; return; }
    else  cipherText.erase(i, watermark.length());
    
    if (v_flag)
    {
        cerr << "cipher size: " << cipherText.size() - 1 << '\n';
        cerr << " block size: " << AES::BLOCKSIZE        << '\n';
    }
    
    AES::Decryption aesDecryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    StreamTransformationFilter stfDecryptor(cbcDecryption,
                                            new CryptoPP::StringSink(decText));
    stfDecryptor.Put(reinterpret_cast<const byte*>
                     (cipherText.c_str()), cipherText.size() - 1);
    stfDecryptor.MessageEnd();
    
    // process decrypted text
    bool isHeader = true;
    byte s;
    string trp;     // triplet
    const ULL decTxtSize = decText.size() - 1;
    
    //todo. watermark (QUALITY_SCORES) to detect fa/fq besed on Encrypted file
    //todo. for FA just write 'FASTA', since there is no quality scores
    
    for (ULL j = 0; j < decTxtSize; ++j)
    {
        s = (byte) decText[j];
        
        if      (s == 252 || (s == 254 && !isHeader))
                           { cout << '\n'; }//empty line OR end of each seq line
        else if (s == 255) { cout << penaltySym(decText[++j]); } //seq len != x3
        else if (s == 253) { cout << '>';            isHeader=true;  } // header
        else if (isHeader) { cout << s; if (s=='\n') isHeader=false; } // header
        else //if (!isHeader)     // seq lines
        {
            trp = DNA_UNPACK[s];
            
            if (trp[0] != 'X' && trp[1] != 'X' && trp[2] != 'X')        // ...
            {
                cout << trp;
            }
            else if (trp[0] == 'X' && trp[1] != 'X' && trp[2] != 'X')   // X..
            {
                cout << penaltySym( decText[++j] );
                cout << trp[1];
                cout << trp[2];
            }
            else if (trp[0] != 'X' && trp[1] == 'X' && trp[2] != 'X')   // .X.
            {
                cout << trp[0];
                cout << penaltySym( decText[++j] );
                cout << trp[2];
            }
            else if (trp[0] == 'X' && trp[1] == 'X' && trp[2] != 'X')   // XX.
            {
                cout << penaltySym( decText[++j] );
                cout << penaltySym( decText[++j] );
                cout << trp[2];
            }
            else if (trp[0] != 'X' && trp[1] != 'X' && trp[2] == 'X')   // ..X
            {
                cout << trp[0];
                cout << trp[1];
                cout << penaltySym( decText[++j] );
            }
            else if (trp[0] == 'X' && trp[1] != 'X' && trp[2] == 'X')   // X.X
            {
                cout << penaltySym( decText[++j] );
                cout << trp[1];
                cout << penaltySym( decText[++j] );
            }
            else if (trp[0] != 'X' && trp[1] == 'X' && trp[2] == 'X')   // .XX
            {
                cout << trp[0];
                cout << penaltySym( decText[++j] );
                cout << penaltySym( decText[++j] );
            }
            else                                                        // XXX
            {
                cout << penaltySym( decText[++j] );
                cout << penaltySym( decText[++j] );
                cout << penaltySym( decText[++j] );
            }
        }
    }
}

/*******************************************************************************
    build IV
*******************************************************************************/
inline void EnDecrypto::buildIV (byte *iv, string pass)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evalPassSize(pass);  // pass size must be >= 8
    
    // using old rand to generate the new rand seed
    srand((unsigned int) 7919 * pass[2] * pass[5] + 75653);
    ULL seed = 0;
    for (byte i = 0; i != pass.size(); ++i)
        seed += ((ULL) pass[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);
    
    for (unsigned int i = 0; i != AES::BLOCKSIZE; ++i)
        iv[i] = (byte) (udist(rng) % 255);
}

/*******************************************************************************
    build key
*******************************************************************************/
inline void EnDecrypto::buildKey (byte *key, string pwd)
{
    std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
    rng_type rng;
    
    evalPassSize(pwd);  // pass size must be >= 8
    
    // using old rand to generate the new rand seed
    srand((unsigned int) 24593 * (pwd[0] * pwd[2]) + 49157);
    ULL seed = 0;
    for (byte i = 0; i != pwd.size(); ++i)
        seed += ((ULL) pwd[i] * rand()) + rand();
    seed %= 4294967295;
    
    const rng_type::result_type seedval = seed;
    rng.seed(seedval);
    
    for (unsigned int i = 0; i != AES::DEFAULT_KEYLENGTH; ++i)
        key[i] = (byte) (udist(rng) % 255);
}

/*******************************************************************************
    print IV
*******************************************************************************/
inline void EnDecrypto::printIV (byte *iv) const
{
    cerr << "IV = [" << (int) iv[0];
    for (unsigned int i = 1; i != AES::BLOCKSIZE; ++i)
        cerr << " " << (int) iv[i];
    cerr << "]\n";
}

/*******************************************************************************
    print key
*******************************************************************************/
inline void EnDecrypto::printKey (byte *key) const
{
    cerr << "KEY: [" << (int) key[0];
    for (unsigned int i = 1; i != AES::DEFAULT_KEYLENGTH; ++i)
        cerr << " " << (int) key[i];
    cerr << "]\n";
}

/*******************************************************************************
    find file type: FASTA or FASTQ
*******************************************************************************/
inline char EnDecrypto::findFileType (std::ifstream &in)
{
    string line;

    // FASTQ
    while (getline(in, line).good())
    {
        if (line[0] == '@')
        {
            in.clear();
            in.seekg(0, std::ios::beg); // go to the beginning of file
            return 'Q';
        }
    }

    // FASTA
    in.clear();  in.seekg(0, std::ios::beg);  return 'A';
}

/*******************************************************************************
    get password from a file
*******************************************************************************/
inline string EnDecrypto::getPassFromFile (const string &keyFileName) const
{
    ifstream input(keyFileName);
    string line;
    
    if (keyFileName.empty())
    {
        cerr << "Error: no password file has been set!\n";
        exit(1);
    }
    else if (!input.good())
    {
        cerr << "Error opening '" << keyFileName << "'.\n";
        exit(1);
    }
    
    while (getline(input, line).good())
    {
        if (line.empty()) {cerr<<"Error: empty password line file!\n"; exit(1);}
        return line;
    }

    return "unknown";
}

/*******************************************************************************
    evaluate password size >= 8
*******************************************************************************/
inline void EnDecrypto::evalPassSize (const string &pass) const
{
    if (pass.size() < 8)
    {
        cerr << "Error: password size must be at least 8!\n";
        exit(1);
    }
}

/*******************************************************************************
    penalty symbol
*******************************************************************************/
inline char EnDecrypto::penaltySym (char c) const
{
    return (c != (char) 254 && c != (char) 252) ? c : (char) 10; //(char)10='\n'
}