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
    
    void encryptFA (int, char**, const int, const string&);
    void decryptFA (int, char**, const int, const string&);

private:
    inline void buildIV  (byte*, string);
    inline void buildKey (byte*, string);
    inline void printIV  (byte*) const;
    inline void printKey (byte*) const;
    inline void evaluatePasswordSize (const string&) const;
    inline string getPasswordFromFile (const string&) const;
    inline char penaltySym (const string&, const ULL) const;
};

#endif //CRYFA_ENDECRYPTO_H