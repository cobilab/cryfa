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
    
    void encryptFA (int, char**, const int, const string&);   // encrypt FASTA
    void decryptFA (int, char**, const int, const string&);   // decrypt FASTA

private:
    inline void buildIV  (byte*, string);                     // build IV
    inline void buildKey (byte*, string);                     // build key
    inline void printIV  (byte*) const;                       // print IV
    inline void printKey (byte*) const;                       // print key
    inline void evalPassSize (const string&) const;           // eval. pass size
    inline string getPassFromFile (const string &) const;     // get password
    inline char penaltySym (char) const;                      // penalty symbol
};

#endif //CRYFA_ENDECRYPTO_H