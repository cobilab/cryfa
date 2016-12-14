//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

#include "defs.h"

// CRYPTOAPP MODULES:
#include "modes.h"
#include "aes.h"
#include "filters.h"

typedef unsigned char byte;
typedef std::mt19937 rng_type;

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
  for(int i = 0 ; i < CryptoPP::AES::MAX_KEYLENGTH ; ++i)
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
  unsigned long long seed = 0;
  for(int i = 0 ; i < pwd.size() ; ++i)
    seed += ((unsigned long long) pwd[i] * rand()) + rand();
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
  unsigned long long seed = 0;
  for(int i = 0 ; i < pwd.size() ; ++i)
    seed += ((unsigned long long) pwd[i] * rand()) + rand();
  seed %= 4294967295;

  rng_type::result_type const seedval = seed;
  rng.seed(seedval);

  for(int i = 0 ; i < CryptoPP::AES::MAX_KEYLENGTH ; ++i)
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
    std::cerr << "Error opening '"<<keyFileName<<"'. Bailing out." << std::endl;
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

void EncryptFA(int argc, char **argv, int v_flag, std::string keyFileName){

  //Key and IV setup
  //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
  //bit). This key is secretly exchanged between two parties before communication   
  //begins. DEFAULT_KEYLENGTH= 16 bytes
  byte key[CryptoPP::AES::MAX_KEYLENGTH],
        iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::MAX_KEYLENGTH); // AES KEY
  memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE);     // INITIALLIZATION VECTOR

  // TODO: SET KEY!
  //std::string password = "password123";
  std::string password = GetPasswordFromFile(keyFileName);

  BuildKey(key, password);
  BuildIV(iv, password);

  PrintIV(iv);
  PrintKey(key);


  std::ifstream input(argv[argc-1]);
  std::string line, header, dna_seq, header_and_dna_seq;

  if(!input.good()){
    std::cerr << "Error opening '"<<argv[argc-1]<<"'. Bailing out." << std::endl;
    exit(1);
    }

  // WATERMAK FOR ENCRYPTED FASTA FILE
  std::cout << "#cryfa v" << VERSION << "." << RELEASE << "" << std::endl;

  while(std::getline(input, line).good()){
    if(line.empty() || line[0] == '>'){ // FASTA identifier 
      if(!header.empty()){ // Print out last entry

        // std::cout << header << " : " << dna_seq << std::endl; // DEBUG PURPOSE

        // TODO: ENCAPSULATE 3 DNA BASES IN 1 BYTE
        //
        
        header_and_dna_seq = header + '\n' + dna_seq; // JOIN STREAM SPLIT BY '\n'

        std::string ciphertext;
        std::string decryptedtext;
        CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
        CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new
        CryptoPP::StringSink(ciphertext));
        stfEncryptor.Put(reinterpret_cast<const unsigned char*>
        (header_and_dna_seq.c_str()), header_and_dna_seq.length()+1);
        stfEncryptor.MessageEnd();

        if(v_flag)
          std::cerr << "Cipher Text size: " << ciphertext.size() << " bytes." << std::endl;

        // DUMP CYPHERTEXT FOR READ
        for(int i = 0; i < ciphertext.size(); ++i)
          std::cout << "0x" << std::hex << (0xFF & 
          static_cast<byte>(ciphertext[i])) << " ";

        header.clear();
        }
      if(!line.empty()) header = line.substr(1);
      dna_seq.clear();
      header_and_dna_seq.clear();
      }
    else if(!header.empty()){
      if(line.find(' ') != std::string::npos){ // Invalid sequence--no spaces allowed
        header.clear();
        dna_seq.clear();
        }
      else{
        dna_seq += line;
        }
      }
    }

  // LAST ENTRY HANDLING:
  if(!header.empty()){ 
    header_and_dna_seq = header + '\n' + dna_seq;
    
    std::cout << header << " : " << dna_seq << std::endl;
    }
        
  std::cout << std::endl << std::endl;

  return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecryptFA(int argc, char **argv){

/*
  CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::MAX_KEYLENGTH);
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );
  CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedtext ) );
  stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext.c_str() ), ciphertext.size() );
  stfDecryptor.MessageEnd();

  //
  // Dump Decrypted Text
  //
  std::cout << "Decrypted Text: " << std::endl;
  std::cout << decryptedtext;
  std::cout << std::endl << std::endl;
*/

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

    if((c = getopt_long(argc, argv, ":havd:k:", 
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
    DecryptFA(argc, argv);
    return 0;
    }

  std::cerr << "Encryption mode on.\n";
  EncryptFA(argc, argv, v_flag, KeyFileName);
  
  return 0;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

