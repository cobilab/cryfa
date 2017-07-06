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
using std::vector;

class EnDecrypto
{
public:
    byte   n_threads;                                    // number of threads
    string inFileName;                                   // input file name
    string keyFileName;                                  // password file name
    bool   disable_shuffle = false;                      // disable shuffle
    bool   verbose = false;                              // for verbose mode
    
    EnDecrypto();                                        // constructor
    void compressFA ();                                  // compress FASTA
    void compressFQ ();                                  // compress FASTQ
    void decrypt ();                                     // decrypt
    void decompressFA ();                                // decompress FA
    void decompressFQ ();                                // decompress FQ
    
private:
    ull seed_shared;
    
    
    string   Hdrs;                                       // max: 39 values
    string   QSs;                                        // max: 39 values
    htable_t HdrMap;                                     // Hdrs hash table
    htable_t QsMap;                                      // QSs hash table
    bool     justPlus = true;                            // if line 3 is just +
    // check if reading input file reached to the end. MUST be initialized
    bool     isEncInEmpty = false;
//    bool     isDecInEmpty = false;
    string   HdrsX;                                      // extended Hdrs
    string   QSsX;                                       // extended QSs
    
    inline void encrypt  ();                             // encrypt
    inline void buildIV  (byte*, const string&);         // build IV
    inline void buildKey (byte*, const string&);         // build key
    inline void printIV  (byte*)                const;   // print IV
    inline void printKey (byte*)                const;   // print key
    inline string extractPass ()                const;   // extract password
    inline void evalPassSize (const string&)    const;   // evaluate pass size
    inline bool hasFQjustPlus ()                const;   // check '+' line
    inline void gatherHdrQs (string&, string&)  const;   // gather hdrs & qss
    inline std::minstd_rand0 &randomEngine ();           // random no. engine
    inline void my_srand (const ui);                     // random no. seed
    inline int  my_rand ();                              // random no generate
//    inline ull  un_shuffleSeedGen (const ui);          // (un)shuffle seed gen
    inline void un_shuffleSeedGen ();                    // (un)shuffle seed gen
    inline void shufflePkd (string&);                    // shuffle packed
    inline void unshufflePkd (string::iterator&, const ull); // unshuffle packed
    inline void pack (const ull, const byte,             // pack
                      string (*)(const string&, const htable_t&),
                      string (*)(const string&, const htable_t&));
    inline void unpackHSQS (const pos_t, const ull,      // unpack H:Small, Q:S
                       const vector<string>&, const vector<string>&, const byte,
                       string (*) (string::iterator&, const vector<string>&),
                       string (*) (string::iterator&, const vector<string>&));
};

#endif //CRYFA_ENDECRYPTO_H