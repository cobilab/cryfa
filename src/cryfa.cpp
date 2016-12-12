//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// COMPILE:  g++ -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
//
// DEPENDENCIES: https://github.com/weidai11/cryptopp
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <iostream>
#include <fstream>
#include <iomanip>

#include "defs.h"

// CRYPTOAPP MODULES:
#include "modes.h"
#include "aes.h"
#include "filters.h"

typedef unsigned char byte;

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
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncryptFA(int argc, char **argv){

  //Key and IV setup
  //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
  //bit). This key is secretly exchanged between two parties before communication   
  //begins. DEFAULT_KEYLENGTH= 16 bytes
  byte key[CryptoPP::AES::MAX_KEYLENGTH],
        iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::MAX_KEYLENGTH);
  memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE);

  std::ifstream input(argv[argc-1]);
  std::string line, header, dna_seq;

  if(!input.good()){
    std::cerr << "Error opening '"<<argv[argc-1]<<"'. Bailing out." << std::endl;
    exit(1);
    }

  // WATERMAK FOR ENCRYPTED FASTA FILE
  std::cout << "#cryfa v" << VERSION << "." << RELEASE << "" << std::endl;

  while(std::getline(input, line).good()){
    if(line.empty() || line[0] == '>'){ // FASTA identifier 
      if(!header.empty()){ // Print out last entry

        // TODO: ENCRYPT NAME & ENCRYPT CONTENT
        std::cout << header << " : " << dna_seq << std::endl;
        // String and Sink setup
        // std::string plaintext = "Now is the time for all good men to come to the aide...";
        std::string ciphertext;
        std::string decryptedtext;

        CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
        CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new
        CryptoPP::StringSink(ciphertext));
        stfEncryptor.Put(reinterpret_cast<const unsigned char*>(dna_seq.c_str()),
        dna_seq.length()+1);
        stfEncryptor.MessageEnd();

        // DUMP CYPHER TEXT FOR DNA BASES
        // std::cout << "Cipher Text (" << ciphertext.size() << " bytes)" << std::endl;
        for(int i = 0; i < ciphertext.size(); ++i){
          std::cout << "0x" << std::hex << (0xFF & static_cast<byte>(ciphertext[i])) << " ";
        }
        // std::cout << std::endl << std::endl;


        header.clear();
        }
      if(!line.empty()){
        header = line.substr(1);
        }
      dna_seq.clear();
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

  if(!header.empty()){ // Print out what we read from the last entry
    std::cout << header << " : " << dna_seq << std::endl;
    }
 
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

  if(argc <= 1){
    std::cerr << "" << std::endl;
    std::cerr << "cryfa v" << VERSION << "." << RELEASE << 
    ": a FASTA encryption and decryption tool.\n" << std::endl;
    std::cerr << "Usage: "<<argv[0]<<" [file]\n" << std::endl;
    std::cerr << "The output is written in the stdout.\n" << std::endl;
    return 0;
    }

/*
  //Key and IV setup
  //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
  //bit). This key is secretly exchanged between two parties before communication   
  //begins. DEFAULT_KEYLENGTH= 16 bytes
  byte key[CryptoPP::AES::MAX_KEYLENGTH], 
        iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::MAX_KEYLENGTH);
  memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE);

  //
  // String and Sink setup
  //
  std::string plaintext = "Now is the time for all good men to come to the aide...";
  std::string ciphertext;
  std::string decryptedtext;

  //
  // Dump Plain Text
  //
  std::cerr << "Plain Text (" << plaintext.size() << " bytes)" << std::endl;
  std::cout << plaintext;
  std::cout << std::endl << std::endl;

  //
  // Create Cipher Text
  //
  CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );
  CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( ciphertext ) );
  stfEncryptor.Put( reinterpret_cast<const unsigned char*>( plaintext.c_str() ), plaintext.length() + 1 );
  stfEncryptor.MessageEnd();

  //
  // Dump Cipher Text
  //
  std::cout << "Cipher Text (" << ciphertext.size() << " bytes)" << std::endl;

  for(int i = 0; i < ciphertext.size(); ++i){
    std::cout << "0x" << std::hex << (0xFF & static_cast<byte>(ciphertext[i])) << " ";
    }

  std::cout << std::endl << std::endl;
*/

  EncryptFA(argc, argv);

  DecryptFA(argc, argv);

/*
  //
  // Decrypt
  //
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

  return 0;
  }


