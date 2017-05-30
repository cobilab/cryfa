
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
    void compressFA  ();               // compress FA
    void compressFQ  ();   // compress FQ
    inline void pack (const ULL, string (*)(const string&, const htable_t&),
                      string (*)(const string&, const htable_t&), const byte);// pack
    inline string encrypt (const string&);  // encrypt
    inline string decrypt ();  // decrypt
    void decompress ();        // decompress
    
    byte   n_threads;// number of threads
    string inFileName;// input file name
    string keyFileName;// password file name
    bool   verbose = false;// for verbose mode
    
private:
    inline void decompFA (string);           // decomp. FA
    inline void decompFQ (string);           // decomp. FQ
    inline void buildIV  (byte*, const string&);                     // build IV
    inline void buildKey (byte*, const string&);                     // build key
    inline void printIV  (byte*) const;                       // print IV
    inline void printKey (byte*) const;                       // print key
    inline string getPassFromFile () const;      // get password
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