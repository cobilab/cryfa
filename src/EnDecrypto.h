/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Encryption / Decryption
    - - - - - - - - - - - - - - - - - - -
    Morteza Hosseini    seyedmorteza@ua.pt
    Diogo Pratas        pratas@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef CRYFA_ENDECRYPTO_H
#define CRYFA_ENDECRYPTO_H

#include "def.h"
using std::string;
using std::vector;

/*******************************************************************************
    structure for packing
*******************************************************************************/
struct pack_s
{
    string (*packHdrFPtr) (const string&, const htbl_t&);   // function pointer
    string (*packQSFPtr)  (const string&, const htbl_t&);   // function pointer
};

/*******************************************************************************
    structure for unpacking
*******************************************************************************/
struct unpack_s
{
    char  XChar_hdr;                // extra char if headers length > 39
    char  XChar_qs;                 // extra char if quality scores length > 39
    pos_t begPos;                   // begin position for each thread
    u64   chunkSize;                // chunk size
    vector<string> hdrUnpack;       // table for unpacking headers
    vector<string> qsUnpack;        // table for unpacking quality scores
    string (*unpackHdrFPtr) (string::iterator&, const vector<string>&);
    string (*unpackQSFPtr)  (string::iterator&, const vector<string>&);
};

/*******************************************************************************
    class for encryption / decryption ...
*******************************************************************************/
class EnDecrypto
{
public:
    bool   disable_shuffle = false;                      // disable shuffle
    bool   verbose = false;                              // for verbose mode
    byte   n_threads;                                    // number of threads
    string inFileName;                                   // input file name
    string keyFileName;                                  // password file name
    
    EnDecrypto          () = default;                    // default constructor
    void   decrypt      ();                              // decrypt
    void   compressFA   ();                              // compress FASTA
    void   decompressFA ();                              // decompress FASTA
    void   compressFQ   ();                              // compress FASTQ
    void   decompressFQ ();                              // decompress FASTQ
    
private:
    bool   justPlus = true;                              // if line 3 is just +
    u64    seed_shared;                                  // shared seed
    string Hdrs;                                         // max: 39 values
    string QSs;                                          // max: 39 values
    string HdrsX;                                        // extended Hdrs
    string QSsX;                                         // extended QSs
    htbl_t HdrMap;                                       // Hdrs hash table
    htbl_t QsMap;                                        // QSs hash table
    
    inline void encrypt       ();                        // encrypt
    inline void buildIV       (byte*, const string&);    // build IV
    inline void buildKey      (byte*, const string&);    // build key
    inline void printIV       (byte*)            const;  // print IV
    inline void printKey      (byte*)            const;  // print key
    inline string extractPass ()                 const;  // extract password
    inline void evalPassSize  (const string&)    const;  // evaluate pass size
    inline bool hasFQjustPlus ()                 const;  // check '+' line
    inline void gatherHdr     (string&)          const;  // gather hdrs -- FA
    inline void gatherHdrQs   (string&, string&) const;  // gather hdrs & qss
    inline void my_srand      (const u32);               // random no. seed
    inline int  my_rand       ();                        // random no generate
    inline std::minstd_rand0 &randomEngine ();           // random no. engine
//    inline u64  un_shuffleSeedGen (const u32);         // (un)shuffle seed gen
    inline void un_shuffleSeedGen ();                    // (un)shuffle seed gen
    inline void shufflePkd    (string&);                 // shuffle packed
    inline void unshufflePkd  (string::iterator&, const u64);// unshuffle packed
    inline void packFA        (const pack_s&,   byte);   // pack FA
    inline void unpackHS      (const unpack_s&, byte);   // unpack H:Small -- FA
    inline void unpackHL      (const unpack_s&, byte);   // unpack H:Large -- FA
    inline void packFQ        (const pack_s&,   byte);   // pack FQ
    inline void unpackHSQS    (const unpack_s&, byte);   // unpack H:Small, Q:S
    inline void unpackHSQL    (const unpack_s&, byte);   // unpack H:S, Q:Large
    inline void unpackHLQS    (const unpack_s&, byte);   // unpack H:Large, Q:S
    inline void unpackHLQL    (const unpack_s&, byte);   // unpack H:Large, Q:L
};

#endif //CRYFA_ENDECRYPTO_H