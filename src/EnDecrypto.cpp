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
    encrypt.
    reserved symbols:
    FASTA:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each sequence line
        (char) 253:  instead of '>' in header
        (char) 252:  instead of empty line
    FASTQ:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each line
        (char) 253:  if third line contains only +
        (char) 252:  end of file
*******************************************************************************/
void EnDecrypto::encrypt (int argc, char **argv, const int v_flag,
                          const string &keyFileName)
{
    ifstream in(argv[argc-1]);
    const bool FASTA = (findFileType(in) == 'A');
    const bool FASTQ = !FASTA;  // const bool FASTQ = (findFileType(in) == 'Q');
    string line;                // each line of file
//    string header;              // header (FASTA/FASTQ)
    string seq;                 // sequence (FASTA/FASTQ)
//    string qs;                  // quality scores (FASTQ)
    // FASTA: context = header + seq (+ empty lines)
    // FASTQ: context = header + seq + plus + qs
    string context;
    string qsRange;             // quality scores presented in FASTQ file
    
    if (!in.good())
    {
        cerr << "Error: failed opening '" << argv[argc-1] << "'.\n";
        return;
    }
    
    // FASTA
    if (FASTA)
    {
        // to tell decryptor this isn't FASTQ
        context += (char) 127;//      context += "\n";
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
    else //if (FASTQ)
    {
        string QUALITY_SCORES_X;    // extended QUALITY_SCORES
        bool justPlus = true;       // if third line is just + or not
        
        // check if the third line contains only +
        in.ignore(LARGE_NUMBER, '\n');  // ignore header
        in.ignore(LARGE_NUMBER, '\n');  // ignore seq
        if (getline(in, line).good()) { if (line.length() > 1) justPlus=false; }
        else { cerr << "Error: file corrupted.\n";    return; }
        
        // gather all quality scores
        while(!in.eof())
        {
            if (getline(in, line).good())   // quality score
            {
                for (string::iterator i = line.begin(); i != line.end(); ++i)
                    if (qsRange.find_first_of(*i) == string::npos)
                        qsRange += *i;
            }
            else { cerr << "Error: file corrupted.\n";    return; }
            in.ignore(LARGE_NUMBER, '\n');  // ignore header
            in.ignore(LARGE_NUMBER, '\n');  // ignore seq
            in.ignore(LARGE_NUMBER, '\n');  // ignore +
        }
        in.clear();  in.seekg(0, std::ios::beg);            // beginning of file
        
        std::sort(qsRange.begin(), qsRange.end());          // sort ASCII values
        
        using packQSPointer = string (*)(string);           // function pointer
        packQSPointer packQS;
//        std::function<string(string)> packQS;     // slower -- more general
        
        const size_t qsRangeLen = qsRange.length();
        if (qsRangeLen > 39)              // if len > 39 filter the last 39 ones
        {
            QUALITY_SCORES   = qsRange.substr(qsRangeLen - 39);
            QUALITY_SCORES_X = QUALITY_SCORES;
            QUALITY_SCORES_X +=  // ASCII char after last char in QUALITY_SCORES
                    (char) (QUALITY_SCORES[QUALITY_SCORES.size()-1] + 1);

            buildQsHashTable(QUALITY_SCORES_X, 3);
            packQS = &packQSLarge_3to2;
        }
        else
        {
            QUALITY_SCORES = qsRange;
            
            if (qsRangeLen > 15)                            // 16 <= #QS <= 39
            { buildQsHashTable(QUALITY_SCORES, 3);    packQS = &packQS_3to2; }
            
            else if (qsRangeLen > 6)                        // 7 <= #QS <= 15
            { buildQsHashTable(QUALITY_SCORES, 2);    packQS = &packQS_2to1; }
                                                            // #QS = 4, 5, 6
            else if (qsRangeLen == 6 || qsRangeLen == 5 || qsRangeLen == 4)
            { buildQsHashTable(QUALITY_SCORES, 3);    packQS = &packQS_3to1; }
            
            else if (qsRangeLen == 3)                       // #QS = 3
            { buildQsHashTable(QUALITY_SCORES, 5);    packQS = &packQS_5to1; }
            
            else if (qsRangeLen == 2)                       // #QS = 2
            { buildQsHashTable(QUALITY_SCORES, 7);    packQS = &packQS_7to1; }
        }
        
        // todo. test
        cerr << qsRange << '\n' << qsRange.length() << '\n';
        
    
        //todo. nabas havijoori 'context+=' nevesht,
        //todo. chon va3 file 10GB mitereke
        //todo. bas hame kara ro block by block anjam dad
        
        context += qsRange;                        // send qsRange to decryptor
        context += (justPlus ? (char) 253 : '\n'); //'+ or not just +' condition
        // (char) 254 instead of '\n' at the end of each line
        while(!in.eof())    // process 4 lines by 4 lines
        {
            // header
            if (getline(in, line).good())
            {
                // header line. //(char) 253 instead of '@'
                // (char) 254 instead of '\n' at the end
//                context += (char) 253 + ..PACK(line) + (char) 254;
                context += line + (char) 254;
            }
            
            // sequence
            if (getline(in, line).good())
            {
                context += packSeq_3to1(line) + (char) 254;
            }
    
            // +. ignore
            in.ignore(LARGE_NUMBER, '\n');
            
            // quality score
            if (getline(in, line).good())
            {
                context += packQS(line) + (char) 254;
            }
        }
        context += (char) 252;  // end of file
    }
    
    in.close();
    
    // cryptography
    // AES encryption uses a secret key of a variable length (128, 196 or
    // 256 bit). This key is secretly exchanged between two parties before
    // communication begins. DEFAULT_KEYLENGTH= 16 bytes
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x00, (size_t) AES::DEFAULT_KEYLENGTH); // AES key
    memset(iv,  0x00, (size_t) AES::BLOCKSIZE);         // Initialization Vector

    const string pass = getPassFromFile(keyFileName);
    buildKey(key, pass);
    buildIV(iv, pass);
//    printIV(iv);      // debug
//    printKey(key);    // debug

//     // do random shuffle
//     srand(0);
//     std::random_shuffle(context.begin(),context.end());
//     * need to know the reverse of shuffle, for decryption!

    string cipherText;
    AES::Encryption aesEncryption(key, (size_t) AES::DEFAULT_KEYLENGTH);
    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
    StreamTransformationFilter stfEncryptor(cbcEncryption,
                                          new CryptoPP::StringSink(cipherText));
    stfEncryptor.Put(reinterpret_cast<const byte*>
                     (context.c_str()), context.length() + 1);
    stfEncryptor.MessageEnd();

    if (v_flag)
    {
        cerr << "   sym size: " << context.size()    << '\n';
        cerr << "cipher size: " << cipherText.size() << '\n';
        cerr << " block size: " << AES::BLOCKSIZE    << '\n';
    }
    
    // watermark for encrypted file
    cout << "#cryfa v" + std::to_string(VERSION_CRYFA) + "."
                       + std::to_string(RELEASE_CRYFA) + "\n";
    
    // dump cyphertext for read
    for (ULL i = 0; i < cipherText.size(); ++i)
        cout << (char) (0xFF & static_cast<byte> (cipherText[i]));
    cout << '\n';
}

/*******************************************************************************
    decrypt.
    reserved symbols:
    FASTA:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each sequence line
        (char) 253:  instead of '>' in header
        (char) 252:  instead of empty line
    FASTQ:
        (char) 255:  penalty if sequence length isn't multiple of 3
        (char) 254:  end of each line
        (char) 253:  if third line contains only +
        (char) 252:  end of file
*******************************************************************************/
void EnDecrypto::decrypt (int argc, char **argv, const int v_flag,
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
    
    string::size_type watermarkIdx = cipherText.find(watermark);
    if (watermarkIdx == string::npos)
    { cerr << "Error: invalid encrypted file!\n";    return; }
    else  cipherText.erase(watermarkIdx, watermark.length());
    
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
    string tpl;     // tuplet
    const ULL decTxtSize = decText.size() - 1;
    const bool FASTA = (decText[0] == (char) 127);
    const bool FASTQ = !FASTA; // const bool FASTQ = (decText[0] != (char) 127);
    string::iterator i = decText.begin();
    
    // FASTA
    if (FASTA)
    {
        bool isHeader = true;
        byte s;
        
        ++i;    // jump over decText[0]
        for (; i != decText.end(); ++i)
        {
            s = (byte) *i;
            //empty line OR end of each seq line
            if (s == 252 || (s == 254 && !isHeader)) { cout << '\n'; }
            //seq len not multiple of 3
            else if (s == 255) { cout << penaltySym(*(++i)); }
            // header
            else if (s == 253) { cout << '>';  isHeader = true; }
            else if (isHeader) { cout << s; if (s == '\n') isHeader = false; }
            // sequence
            else //if (!isHeader)
            {
                tpl = DNA_UNPACK[s];
                
                if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] != 'X')      // ...
                { cout << tpl; }
                
                else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] != 'X') // X..
                { cout << penaltySym(*(++i)) << tpl[1] << tpl[2]; }
                
                else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] != 'X') // .X.
                { cout << tpl[0] << penaltySym(*(++i)) << tpl[2]; }
                
                else if (tpl[0] == 'X' && tpl[1] == 'X' && tpl[2] != 'X') // XX.
                { cout << penaltySym(*(++i)) << penaltySym(*(++i)) << tpl[2]; }
                
                else if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] == 'X') // ..X
                { cout << tpl[0] << tpl[1] << penaltySym(*(++i)); }
                
                else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] == 'X') // X.X
                { cout << penaltySym(*(++i)) << tpl[1] << penaltySym(*(++i)); }
                
                else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] == 'X') // .XX
                { cout << tpl[0] << penaltySym(*(++i)) << penaltySym(*(++i)); }
                
                else { cout << penaltySym(*(++i)) << penaltySym(*(++i))   // XXX
                            << penaltySym(*(++i)); }
            }
        }
    }
    
    // FASTQ
    else // if (FASTQ)
    {
        string qsRange;
        bool justPlus = true;
        string plusMore;
        
        for (; *i != '\n' && *i != (char) 253; ++i)     qsRange += *i; // all qs
        if (*i == '\n')  justPlus = false;              // if 3rd line is just +
        ++i;   // jump over '\n' or (char) 253
        
        const size_t qsRangeLen = qsRange.length();
        short keyLen = 0;
        
        using unpackQSPointer = void (*)(string::iterator&); // function pointer
        unpackQSPointer unpackQS;
        // 40 <= #QS
        if (qsRangeLen > 39)    keyLen = 3;
        // 16 <= #QS <= 39
        else if (qsRangeLen > 15) { keyLen = 3;   unpackQS = &unpackQS_read2B; }
        // 7 <= #QS <= 15
        else if (qsRangeLen > 6)  { keyLen = 2;   unpackQS = &unpackQS_read1B; }
        // #QS = 6, 5, 4
        else if (qsRangeLen==6 || qsRangeLen==5 || qsRangeLen==4)
        { keyLen = 3;    unpackQS = &unpackQS_read1B; }
        // #QS = 3
        else if (qsRangeLen == 3) { keyLen = 5;   unpackQS = &unpackQS_read1B; }
        // #QS = 2
        else if (qsRangeLen == 2) { keyLen = 7;   unpackQS = &unpackQS_read1B; }
        
        if (qsRangeLen > 39)
        {
            const string quality_scores = qsRange.substr(qsRangeLen - 39);
            // ASCII char after the last char in quality_scores string
            const char XChar =
                    (char) (quality_scores[quality_scores.size()-1] + 1);
            string quality_scores_X = quality_scores;   quality_scores_X+=XChar;
            
            buildQsUnpack(quality_scores_X, keyLen); //build table for unpacking
            
            while (i != decText.end())
            {
                unpackHdrFQ(i, plusMore);    ++i;   // header
                unpackSeqFQ_3to1(i);                // sequence
                // +
                cout << (justPlus ? "+" : "+"+plusMore.substr(1)) << '\n';  ++i;
                unpackQSLarge_read2B(i, XChar);     // quality scores
                //end of file
                if (*(++i) == (char) 252) break;    else cout << '\n';
            }
        }
        else
        {
            buildQsUnpack(qsRange, keyLen);     // build table for unpacking
            
            while (i != decText.end())
            {
                unpackHdrFQ(i, plusMore);    ++i;   // header
                unpackSeqFQ_3to1(i);                // sequence
                // +
                cout<<(justPlus ? "+" : "+"+plusMore.substr(1)) <<'\n'; ++i;
                unpackQS(i);                        // quality scores
                // end of file
                if (*(++i) == (char) 252) break;    else cout << '\n';
            }
        }
    }   // end--FASTQ
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