//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//            ===================================================
//            | CRYFA :: A FASTA encryption and decryption tool |
//            ===================================================
//
// COMPILE:  g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
//
// DEPENDENCIES: https://github.com/weidai11/cryptopp
// sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <streambuf>
#include <unordered_map>

#include "defs.h"

// CRYPTOAPP MODULES:
#include "modes.h"
#include "aes.h"
#include "filters.h"

#include "eax.h"

typedef unsigned char byte;
typedef unsigned long long ULL;
typedef std::mt19937 rng_type;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// 216
const std::string DNA_UNPACK[] = 
                     { "AAA", "AAC", "AAG", "AAT", "AAN", "AAX", "ACA", "ACC",
                       "ACG", "ACT", "ACN", "ACX", "AGA", "AGC", "AGG", "AGT",
                       "AGN", "AGX", "ATA", "ATC", "ATG", "ATT", "ATN", "ATX",
                       "ANA", "ANC", "ANG", "ANT", "ANN", "ANX", "AXA", "AXC",
                       "AXG", "AXT", "AXN", "AXX", "CAA", "CAC", "CAG", "CAT",
                       "CAN", "CAX", "CCA", "CCC", "CCG", "CCT", "CCN", "CCX",
                       "CGA", "CGC", "CGG", "CGT", "CGN", "CGX", "CTA", "CTC",
                       "CTG", "CTT", "CTN", "CTX", "CNA", "CNC", "CNG", "CNT",
                       "CNN", "CNX", "CXA", "CXC", "CXG", "CXT", "CXN", "CXX",
                       "GAA", "GAC", "GAG", "GAT", "GAN", "GAX", "GCA", "GCC",
                       "GCG", "GCT", "GCN", "GCX", "GGA", "GGC", "GGG", "GGT",
                       "GGN", "GGX", "GTA", "GTC", "GTG", "GTT", "GTN", "GTX",
                       "GNA", "GNC", "GNG", "GNT", "GNN", "GNX", "GXA", "GXC",
                       "GXG", "GXT", "GXN", "GXX", "TAA", "TAC", "TAG", "TAT",
                       "TAN", "TAX", "TCA", "TCC", "TCG", "TCT", "TCN", "TCX",
                       "TGA", "TGC", "TGG", "TGT", "TGN", "TGX", "TTA", "TTC",
                       "TTG", "TTT", "TTN", "TTX", "TNA", "TNC", "TNG", "TNT",
                       "TNN", "TNX", "TXA", "TXC", "TXG", "TXT", "TXN", "TXX",
                       "NAA", "NAC", "NAG", "NAT", "NAN", "NAX", "NCA", "NCC",
                       "NCG", "NCT", "NCN", "NCX", "NGA", "NGC", "NGG", "NGT",
                       "NGN", "NGX", "NTA", "NTC", "NTG", "NTT", "NTN", "NTX",
                       "NNA", "NNC", "NNG", "NNT", "NNN", "NNX", "NXA", "NXC",
                       "NXG", "NXT", "NXN", "NXX", "XAA", "XAC", "XAG", "XAT",
                       "XAN", "XAX", "XCA", "XCC", "XCG", "XCT", "XCN", "XCX",
                       "XGA", "XGC", "XGG", "XGT", "XGN", "XGX", "XTA", "XTC",
                       "XTG", "XTT", "XTN", "XTX", "XNA", "XNC", "XNG", "XNT",
                       "XNN", "XNX", "XXA", "XXC", "XXG", "XXT", "XXN", "XXX" };

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::unordered_map<std::string,int> mymap = {
  {"AAA", 0}, {"AAC", 1}, {"AAG", 2}, {"AAT", 3}, {"AAN",  4}, {"AAX",  5}, 
  {"ACA", 6}, {"ACC", 7}, {"ACG", 8}, {"ACT", 9}, {"ACN", 10}, {"ACX", 11}, 
  {"AGA", 12}, {"AGC", 13}, {"AGG", 14}, {"AGT", 15}, {"AGN", 16}, {"AGX", 17}, 
  {"ATA", 18}, {"ATC", 19}, {"ATG", 20}, {"ATT", 21}, {"ATN", 22}, {"ATX", 23}, 
  {"ANA", 24}, {"ANC", 25}, {"ANG", 26}, {"ANT", 27}, {"ANN", 28}, {"ANX", 29}, 
  {"AXA", 30}, {"AXC", 31}, {"AXG", 32}, {"AXT", 33}, {"AXN", 34}, {"AXX", 35}, 
  {"CAA", 36}, {"CAC", 37}, {"CAG", 38}, {"CAT", 39}, {"CAN", 40}, {"CAX", 41}, 
  {"CCA", 42}, {"CCC", 43}, {"CCG", 44}, {"CCT", 45}, {"CCN", 46}, {"CCX", 47}, 
  {"CGA", 48}, {"CGC", 49}, {"CGG", 50}, {"CGT", 51}, {"CGN", 52}, {"CGX", 53}, 
  {"CTA", 54}, {"CTC", 55}, {"CTG", 56}, {"CTT", 57}, {"CTN", 58}, {"CTX", 59}, 
  {"CNA", 60}, {"CNC", 61}, {"CNG", 62}, {"CNT", 63}, {"CNN", 64}, {"CNX", 65}, 
  {"CXA", 66}, {"CXC", 67}, {"CXG", 68}, {"CXT", 69}, {"CXN", 70}, {"CXX", 71}, 
  {"GAA", 72}, {"GAC", 73}, {"GAG", 74}, {"GAT", 75}, {"GAN", 76}, {"GAX", 77}, 
  {"GCA", 78}, {"GCC", 79}, {"GCG", 80}, {"GCT", 81}, {"GCN", 82}, {"GCX", 83}, 
  {"GGA", 84}, {"GGC", 85}, {"GGG", 86}, {"GGT", 87}, {"GGN", 88}, {"GGX", 89}, 
  {"GTA", 90}, {"GTC", 91}, {"GTG", 92}, {"GTT", 93}, {"GTN", 94}, {"GTX", 95}, 
  {"GNA", 96}, {"GNC", 97}, {"GNG", 98}, {"GNT", 99}, {"GNN", 100}, 
  {"GNX", 101}, {"GXA", 102}, {"GXC", 103}, {"GXG", 104}, {"GXT", 105}, 
  {"GXN", 106}, {"GXX", 107}, {"TAA", 108}, {"TAC", 109}, {"TAG", 110}, 
  {"TAT", 111}, {"TAN", 112}, {"TAX", 113}, {"TCA", 114}, {"TCC", 115}, 
  {"TCG", 116}, {"TCT", 117}, {"TCN", 118}, {"TCX", 119}, {"TGA", 120}, 
  {"TGC", 121}, {"TGG", 122}, {"TGT", 123}, {"TGN", 124}, {"TGX", 125}, 
  {"TTA", 126}, {"TTC", 127}, {"TTG", 128}, {"TTT", 129}, {"TTN", 130}, 
  {"TTX", 131}, {"TNA", 132}, {"TNC", 133}, {"TNG", 134}, {"TNT", 135}, 
  {"TNN", 136}, {"TNX", 137}, {"TXA", 138}, {"TXC", 139}, {"TXG", 140}, 
  {"TXT", 141}, {"TXN", 142}, {"TXX", 143}, {"NAA", 144}, {"NAC", 145}, 
  {"NAG", 146}, {"NAT", 147}, {"NAN", 148}, {"NAX", 149}, {"NCA", 150}, 
  {"NCC", 151}, {"NCG", 152}, {"NCT", 153}, {"NCN", 154}, {"NCX", 155}, 
  {"NGA", 156}, {"NGC", 157}, {"NGG", 158}, {"NGT", 159}, {"NGN", 160}, 
  {"NGX", 161}, {"NTA", 162}, {"NTC", 163}, {"NTG", 164}, {"NTT", 165}, 
  {"NTN", 166}, {"NTX", 167}, {"NNA", 168}, {"NNC", 169}, {"NNG", 170}, 
  {"NNT", 171}, {"NNN", 172}, {"NNX", 173}, {"NXA", 174}, {"NXC", 175}, 
  {"NXG", 176}, {"NXT", 177}, {"NXN", 178}, {"NXX", 179}, {"XAA", 180}, 
  {"XAC", 181}, {"XAG", 182}, {"XAT", 183}, {"XAN", 184}, {"XAX", 185}, 
  {"XCA", 186}, {"XCC", 187}, {"XCG", 188}, {"XCT", 189}, {"XCN", 190}, 
  {"XCX", 191}, {"XGA", 192}, {"XGC", 193}, {"XGG", 194}, {"XGT", 195}, 
  {"XGN", 196}, {"XGX", 197}, {"XTA", 198}, {"XTC", 199}, {"XTG", 200}, 
  {"XTT", 201}, {"XTN", 202}, {"XTX", 203}, {"XNA", 204}, {"XNC", 205}, 
  {"XNG", 206}, {"XNT", 207}, {"XNN", 208}, {"XNX", 209}, {"XXA", 210}, 
  {"XXC", 211}, {"XXG", 212}, {"XXT", 213}, {"XXN", 214}, {"XXX", 215}};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int DNA_PACK(std::string DNA){

   std::unordered_map<std::string,int>::const_iterator got = mymap.find(DNA);

  if(got == mymap.end()){
    std::cerr << "Error: key not found!\n";
    exit(1);
    }
  else
    return got->second;

  return -1;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void About (void){
  std::cout                                                       << "\n"
  << "cryfa v" << VERSION << "." << RELEASE                       << "\n"
  << "================"                                           << "\n"
  << "A FASTA encryption and decryption tool"                     << "\n"
                                                                  << "\n"
  << "Diogo Pratas & Armando J. Pinho"                            << "\n"
  << "Copyright (C) 2017 University of Aveiro"                    << "\n"
                                                                  << "\n"
  << "This is a Free software, under GPLv3. You may redistribute" << "\n"
  << "copies of it under the terms of the GNU - General Public"   << "\n"
  << "License v3 <http://www.gnu.org/licenses/gpl.html>. There"   << "\n"
  << "is NOT ANY WARRANTY, to the extent permitted by law."       << "\n"
                                                                  << "\n";
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Help(void){
  std::cout                                                       << "\n"
  << "Synopsis:"                                                  << "\n"
  << "    cryfa [OPTION]... -k [KEYFILENAME] [FILENAME]"          << "\n"
                                                                  << "\n"
  << "Options:"                                                   << "\n"
  << "    -h,  --help"                                            << "\n"
  << "         usage guide"                                       << "\n"
                                                                  << "\n"
  << "    -a,  --about"                                           << "\n"
  << "         about the program"                                 << "\n"
                                                                  << "\n"
  << "    -v,  --verbose"                                         << "\n"
  << "         verbose mode (more information)"                   << "\n"
                                                                  << "\n"
  << "    -d,  --decrypt"                                         << "\n"
  << "         decrypt mode"                                      << "\n"
                                                                  << "\n"
  << "    -k [KEYFILE],  --key [KEYFILE]"                         << "\n"
  << "         key filename"                                      << "\n"
                                                                  << "\n";
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintKey(byte *key){
  std::cerr << "KEY: [";
  for(int i = 0 ; i < CryptoPP::AES::DEFAULT_KEYLENGTH ; ++i)
    std::cerr << (int) key[i] << " ";
  std::cerr << "]\n";
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintIV(byte *iv){ // XXX: SHOULD THIS HAVE 32 OF SIZE?
  std::cerr << "IV : [";
  for(int i = 0 ; i < CryptoPP::AES::BLOCKSIZE ; ++i)
    std::cerr << (int) iv[i] << " ";
  std::cerr << "]\n";
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EvaluatePasswordSize(std::string pwd){
  if(pwd.size() < 8){
    std::cerr << "Error: password is too short!\n";
    exit(1);
    }
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void BuildIV(byte *iv, std::string pwd){

  std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
  rng_type rng;

  EvaluatePasswordSize(pwd);

  // USING OLD RAND TO GENERATE THE NEW RAND SEED
  srand(7919 * (pwd[2] * pwd[5]) + 75653);
  ULL seed = 0;
  for(int i = 0 ; i < pwd.size() ; ++i)
    seed += ((ULL) pwd[i] * rand()) + rand();
  seed %= 4294967295;

  rng_type::result_type const seedval = seed;
  rng.seed(seedval);

  for(int i = 0 ; i < CryptoPP::AES::BLOCKSIZE ; ++i)
    iv[i] = udist(rng) % 255;

  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void BuildKey(byte *key, std::string pwd){

  std::uniform_int_distribution<rng_type::result_type> udist(0, 255);
  rng_type rng;

  EvaluatePasswordSize(pwd);

  // USING OLD RAND TO GENERATE THE NEW RAND SEED
  srand(24593 * (pwd[0] * pwd[2]) + 49157);
  ULL seed = 0;
  for(int i = 0 ; i < pwd.size() ; ++i)
    seed += ((ULL) pwd[i] * rand()) + rand();
  seed %= 4294967295;

  rng_type::result_type const seedval = seed;
  rng.seed(seedval);

  for(int i = 0 ; i < CryptoPP::AES::DEFAULT_KEYLENGTH ; ++i)
    key[i] = udist(rng) % 255;

  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string GetPasswordFromFile(std::string keyFileName){

  std::ifstream input(keyFileName);
  std::string line;

  if(keyFileName == ""){
    std::cerr << "Error: no password file has been set!\n";
    exit(1);
    }

  if(!input.good()){
    std::cerr << "Error opening '"<<keyFileName<<"'." << std::endl;
    exit(1);
    }

  while(std::getline(input, line).good()){
    if(line.empty()){
      std::cerr << "Error: empty password line file!\n";
      exit(1);
      }
    return line;
    }
  
  return "unknown";
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ENCAPSULATE 3 DNA BASES IN 1 BYTE
//
std::string PackIn3bDNASeq(std::string seq){
  std::string packedSeq;
  ULL seqSize = seq.length(), x;

  for(x = 0 ; x < seqSize ; x += 3){
    int first = 0, second = 0, third = 0;
    char firstSym, secondSym, thirdSym;

    if(seq[x] != 'A' && seq[x] != 'C' && seq[x] != 'G' && seq[x] != 'T' &&
    seq[x] != 'N'){
      first = 1;
      firstSym = seq[x]; 
      seq[x] = 'X';
      }

    if(seq[x+1] != 'A' && seq[x+1] != 'C' && seq[x+1] != 'G' && seq[x+1] != 'T' 
    && seq[x+1] != 'N'){
      second = 1;
      secondSym = seq[x+1]; 
      seq[x+1] = 'X';
      }

    if(seq[x+2] != 'A' && seq[x+2] != 'C' && seq[x+2] != 'G' && seq[x+2] != 'T' 
    && seq[x+2] != 'N'){
      third = 1;
      thirdSym = seq[x+2]; 
      seq[x+2] = 'X';
      }

//fprintf(stderr, "%llu,%u:%u:%u\n", seqSize, (int) seq[x], (int) seq[x+1], (int) seq[x+2]);

    std::string triplet;
    triplet += seq[x];
    triplet += seq[x+1];
    triplet += seq[x+2];

    packedSeq += (char) DNA_PACK(triplet);

    if(first  == 1) packedSeq += firstSym;
    if(second == 1) packedSeq += secondSym;
    if(third  == 1) packedSeq += thirdSym;
    }

  ULL plus = seqSize % 3;
  if(plus == 1){
    packedSeq += 231;
    packedSeq += seq[x];
    }
  else if(plus == 2){
    packedSeq += 232;
    packedSeq += seq[x];
    packedSeq += seq[x+1];
    }

  //packedSeq = seq;
  return packedSeq;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string Unpack3binDNASeq(std::string seq){
  std::string unpackedSeq;


  return unpackedSeq;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncryptFA(int argc, char **argv, int v_flag, std::string keyFileName){
  //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
  //bit). This key is secretly exchanged between two parties before communication   
  //begins. DEFAULT_KEYLENGTH= 16 bytes
  byte key[CryptoPP::AES::DEFAULT_KEYLENGTH],
        iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH); // AES KEY
  memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE);     // INITIALLIZATION VECTOR

  std::string password = GetPasswordFromFile(keyFileName);
  BuildKey(key, password);
  BuildIV(iv, password);

  PrintIV(iv);
  PrintKey(key);

  std::ifstream input(argv[argc-1]);
  std::string line, header, dna_seq, header_and_dna_seq;

  if(!input.good()){
    std::cerr << "Error opening '"<<argv[argc-1]<<"'." << std::endl;
    exit(1);
    }

  while(std::getline(input, line).good()){
    if(line.empty() || line[0] == '>'){ // FASTA identifier 
      if(!header.empty()){ // Print out last entry
        // std::cout << ">" header << "\n" << dna_seq << std::endl; // DEBUG PURPOSE
        header_and_dna_seq += ('>' + header + '\n' + PackIn3bDNASeq(dna_seq)); 
        header.clear();
        }
      if(!line.empty()) 
        header = line.substr(1);
      dna_seq.clear();
      }
    else if(!header.empty()){
      if(line.find(' ') != std::string::npos){ // Invalid sequence--no spaces allowed
        header.clear();
        dna_seq.clear();
        }
      else{
        dna_seq += (line + '\n');
        }
      }
    }

  // LAST ENTRY HANDLING:
  if(!header.empty()){ 
    // std::cout << ">" header << "\n" << dna_seq << std::endl; // DEBUG PURPOSE
    header_and_dna_seq += ('>' + header + '\n' + PackIn3bDNASeq(dna_seq)); 
    }

  input.close();

/*
  // DO RANDOM SHUFFLE:
  srand(0);
  std::random_shuffle(header_and_dna_seq.begin(),header_and_dna_seq.end());
  * NEED TO KNOW THE REVERSE OF SHUFFLE, FOR DECRYPT!
*/

  header_and_dna_seq += "<"; // KNOW WHERE IS END ON DECRYPTION

  std::string ciphertext;
  CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
  CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new
  CryptoPP::StringSink(ciphertext));
  stfEncryptor.Put(reinterpret_cast<const unsigned char*>
  (header_and_dna_seq.c_str()), header_and_dna_seq.length()+1);
  stfEncryptor.MessageEnd();

  if(v_flag){
    std::cerr << "   sym size: " << header_and_dna_seq.size() << "\n";
    std::cerr << "cipher size: " << ciphertext.size() << "\n";
    std::cerr << " block size: " << CryptoPP::AES::BLOCKSIZE << "\n";
    }

  // WATERMAK FOR ENCRYPTED FASTA FILE
  std::cout << "#cryfa v" << VERSION << "." << RELEASE << "" << std::endl;

  // DUMP CYPHERTEXT FOR READ
  for(ULL i = 0; i < ciphertext.size(); ++i)
    std::cout << (char) (0xFF & static_cast<byte>(ciphertext[i]));

  std::cout << std::endl << std::endl;
  header_and_dna_seq.clear();
  ciphertext.clear();
  keyFileName.clear();
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecryptFA(int argc, char **argv, int v_flag, std::string keyFileName){

  byte key[CryptoPP::AES::DEFAULT_KEYLENGTH],
        iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH); // AES KEY
  memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE);     // INITIALLIZATION VECTOR

  std::string password = GetPasswordFromFile(keyFileName);
  BuildKey(key, password);
  BuildIV(iv, password);

  PrintIV(iv);
  PrintKey(key);

  std::string line, decryptedtext;
  std::ifstream input(argv[argc-1]);

  if(!input.good()){
    std::cerr << "Error opening '"<<argv[argc-1]<<"'." << std::endl;
    exit(1);
    }

  std::string ciphertext((std::istreambuf_iterator<char>(input)),
              std::istreambuf_iterator<char>());

  std::string watermark = "#cryfa v1.1\n";
  std::string::size_type i = ciphertext.find(watermark);

  if(i == std::string::npos){
    std::cerr << "Error: invalid encrypted file!\n";
    exit(1);
    }
  else{
    ciphertext.erase(i, watermark.length());
    }

  if(v_flag){
    std::cerr << "cipher size: " << ciphertext.size() << "\n";
    std::cerr << " block size: " << CryptoPP::AES::BLOCKSIZE << "\n";
    }

  CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
  CryptoPP::CBC_Mode_ExternalCipher::Decryption 
  cbcDecryption(aesDecryption, iv);
  CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new  
  CryptoPP::StringSink(decryptedtext));
  stfDecryptor.Put(reinterpret_cast<const unsigned char*>(ciphertext.c_str()), 
  ciphertext.size()-2);
  stfDecryptor.MessageEnd();

  // Dump Decrypted Text
  std::cerr << "Decrypted Text: " << std::endl;
  for(ULL i = 0 ; i < decryptedtext.size() ; ++i){
    if((char) decryptedtext[i] == '<') break;
    std::cout << (char) decryptedtext[i];
    }
  
  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char* argv[]){
  static int h_flag, a_flag, v_flag, d_flag;
  bool t_flag = false;          // target file name entered
  std::string KeyFileName = "";    // argument of option 'k'
  int c;              // deal with getopt_long()
  int option_index;   // option index stored by getopt_long()

  opterr = 0;   // force getopt_long() to remain silent when it finds a problem

  static struct option long_options[] = {
    {"help",            no_argument, &h_flag, (int) 'h'},   // help
    {"about",           no_argument, &a_flag, (int) 'a'},   // about
    {"verbose",         no_argument, &v_flag, (int) 'v'},   // verbose
    {"decrypt",         no_argument, &d_flag, (int) 'd'},   // decrypt mode
    {"key",       required_argument,       0,       'k'},   // key file
    {0, 0, 0, 0}
    };

  while(1){
    option_index = 0;

    if((c = getopt_long(argc, argv, ":havdk:", 
    long_options, &option_index)) == -1)
      break;

    switch(c){
      case 0:
        // If this option set a flag, do nothing else now.
        if (long_options[option_index].flag != 0)
          break;
        std::cout << "option '" << long_options[option_index].name << "'\n";
        if(optarg)
          std::cout << " with arg " << optarg << "\n";
      break;

      case 'h':   // show usage guide
        h_flag = 1;
        Help();
        exit(1);
      break;

      case 'a':   // show about
        a_flag = 1;
        About();
        exit(1);
      break;

      case 'v':   // verbose mode
        v_flag = 1;
      break;

      case 'd':   // decrypt mode
        d_flag = 1;
      break;

      case 'k':   // needs key filename
        t_flag = true;
        KeyFileName = (std::string) optarg;
      break;

      default:
        std::cerr << "Option '" << (char) optopt << "' is invalid.\n";
      break;
      }
    }

  if(v_flag){
    std::cerr << "Verbose mode on.\n";
    }

  if(d_flag){
    std::cerr << "Decryption mode on.\n";
    DecryptFA(argc, argv, v_flag, KeyFileName);
    return 0;
    }

  std::cerr << "Encryption mode on.\n";
  EncryptFA(argc, argv, v_flag, KeyFileName);
  
  return 0;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

