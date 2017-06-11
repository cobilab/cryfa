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
    EnDecrypto();                                        // constructor
    void compressFA ();                                  // compress FASTA
    void compressFQ ();                                  // compress FASTQ
    void decompress ();                                  // decompress
    
    byte   n_threads;                                    // number of threads
    string inFileName;                                   // input file name
    string keyFileName;                                  // password file name
    bool   verbose = false;                              // for verbose mode
    bool   disable_shuffle = false;                      // disable shuffle
    
private:
    inline string encrypt (const string&);               // encrypt
    inline string decrypt ();                            // decrypt
    inline void pack (const ull, const byte,             // pack
                      string (*)(const string&, const htable_t&),
                      string (*)(const string&, const htable_t&));
    inline void decompFA (string);                       // decomp. FA
    inline void decompFQ (string);                       // decomp. FQ
    inline void buildIV  (byte*, const string&);         // build IV
    inline void buildKey (byte*, const string&);         // build key
    inline void printIV  (byte*)                const;   // print IV
    inline void printKey (byte*)                const;   // print key
    inline string extractPass ()                const;   // get password
    inline void evalPassSize (const string&)    const;   // evaluate pass size
    inline bool hasFQjustPlus ()                const;   // check '+' line
    inline void gatherHdrQs (string&, string&)  const;   // gather hdrs & qss
    inline ull  un_shuffleSeedGen (ui)          const;   // (un)shuffle seed gen
    inline void shufflePkd (string&)            const;   // shuffle packed
    inline void unshufflePkd (string::iterator&,         // unshuffle packed
                              const ull)        const;
    
    string   HdrsX;                                      // extended Hdrs
    string   QSsX;                                       // extended QSs
    htable_t HdrMap;                                     // Hdrs hash table
    htable_t QsMap;                                      // QSs hash table
    // check if reading input file reached to the end. MUST be initialized
    bool     isInEmpty = false;
};

#endif //CRYFA_ENDECRYPTO_H