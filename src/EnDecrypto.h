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
    EnDecrypto();   // constructor
    
    void          encryptFA            (int, char**, int, string);
    void          decryptFA            (int, char**, int, string);
    inline void   buildIV              (byte*, string);
    inline void   buildKey             (byte*, string);
    inline void   printIV              (byte*);
    inline void   printKey             (byte*);
    inline string getPasswordFromFile  (string);
    inline void   evaluatePasswordSize (string);
};

#endif //CRYFA_ENDECRYPTO_H