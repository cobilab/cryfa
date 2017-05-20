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
    EnDecrypto();                                               // constructor
    
    void compressFA (const string&, const string&, const int);  // compress FA
//    void compressFQ (const string&, const string&, const int);  // compress FQ
    void compressFQ (const string&, const string&, const int,
                     string, string, byte);  // compress FQ
    void encrypt    (const string&, const string&, const int,
                     byte);  // encrypt
    string decrypt  (const string&, const string&, const int);  // decrypt
    void decompress (const string&, const string&, const int);  // decompress

private:
    void decompFA   (string, const string&);                  // decomp. FA
    void decompFQ   (string, const string&);                  // decomp. FQ
    inline void buildIV  (byte*, string);                     // build IV
    inline void buildKey (byte*, string);                     // build key
    inline void printIV  (byte*) const;                       // print IV
    inline void printKey (byte*) const;                       // print key
    inline string getPassFromFile (const string&) const;      // get password
    inline void evalPassSize (const string&) const;           // eval. pass size
};

#endif //CRYFA_ENDECRYPTO_H