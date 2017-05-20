/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Encryption / Decryption
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef CRYFA_ENDECRYPTO_H
#define CRYFA_ENDECRYPTO_H

#include "def.h"
using std::string;

class EnDecrypto
{
public:
    EnDecrypto();                                             // constructor
    
//    void compressFA();
//    void compressFQ();
//    void encrypt();
//    void decompressFA();
//    void decompressFQ();
//    void decrypt();
    
    void encrypt (int, char**, const string&, const int);     // encrypt
    void decrypt (int, char**, const string&, const int);     // decrypt
    
    
    inline char findFileType (std::ifstream&);                // FASTA or FASTQ?
private:
    inline void buildIV  (byte*, string);                     // build IV
    inline void buildKey (byte*, string);                     // build key
    inline void printIV  (byte*) const;                       // print IV
    inline void printKey (byte*) const;                       // print key
    inline string getPassFromFile (const string&) const;      // get password
    inline void evalPassSize (const string&) const;           // eval. pass size
};

#endif //CRYFA_ENDECRYPTO_H