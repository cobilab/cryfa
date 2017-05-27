
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
    void compressFQ (const string&, const string&, const int, const byte);  // compress FQ
    inline void pack (const string&, const string&,
                      string (*)(string, string, htable_t&),
                      string (*)(string, string, htable_t&),
                      const int, const byte); // pack
    inline string encrypt (const string&, const string&, const int);  // encrypt
    inline void writeEncoded (string, const byte&);  // write encoded context
    
    string decrypt  (const string&, const string&, const int);  // decrypt
    void decompress (const string&, const string&, const int);  // decompress
    
private:
    inline void decompFA   (string, const string&);           // decomp. FA
    inline void decompFQ   (string, const string&);           // decomp. FQ
    inline void buildIV  (byte*, string);                     // build IV
    inline void buildKey (byte*, string);                     // build key
    inline void printIV  (byte*) const;                       // print IV
    inline void printKey (byte*) const;                       // print key
    inline string getPassFromFile (const string&) const;      // get password
    inline void evalPassSize (const string&) const;           // eval. pass size
};

#endif //CRYFA_ENDECRYPTO_H





// single-threaded version
///*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//    Encryption / Decryption
//    - - - - - - - - - - - - - - - - - - -
//    Diogo Pratas        pratas@ua.pt
//    Morteza Hosseini    seyedmorteza@ua.pt
//    Armando J. Pinho    ap@ua.pt
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//
//#ifndef CRYFA_ENDECRYPTO_H
//#define CRYFA_ENDECRYPTO_H
//
//#include "def.h"
//using std::string;
//
//class EnDecrypto
//{
//public:
//    EnDecrypto();                                             // constructor
//
//    void encrypt (int, char**, const string&, const int);     // encrypt
//    void decrypt (int, char**, const string&, const int);     // decrypt
//
//
//    inline char findFileType (std::ifstream&);                // FASTA or FASTQ?
//private:
//    inline void buildIV  (byte*, string);                     // build IV
//    inline void buildKey (byte*, string);                     // build key
//    inline void printIV  (byte*) const;                       // print IV
//    inline void printKey (byte*) const;                       // print key
//    inline string getPassFromFile (const string&) const;      // get password
//    inline void evalPassSize (const string&) const;           // eval. pass size
//};
//
//#endif //CRYFA_ENDECRYPTO_H