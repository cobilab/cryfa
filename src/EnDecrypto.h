/**
 * @file      EnDecrypto.h
 * @brief     Encryption / Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_ENDECRYPTO_H
#define CRYFA_ENDECRYPTO_H

#include "def.h"
using std::string;
using std::vector;


/**
 * @brief Packing
 */
struct pack_s
{
    /**
     * @fn    void (*packHdrFPtr) (string&, const string&, const htbl_t&)
     * @brief Points to a header packing function
     * @fn    void (*packQSFPtr)  (string&, const string&, const htbl_t&)
     * @brief Points to a quality score packing function
     */
    void (*packHdrFPtr) (string&, const string&, const htbl_t&);
    void (*packQSFPtr)  (string&, const string&, const htbl_t&);
};

/**
 * @brief Unpakcing
 */
struct unpack_s
{
    /**
     * @fn void(*unpackHdrFPtr)(string&,string::iterator&,const vector<string>&)
     * @brief Points to a header unpacking function
     * @fn void(*unpackQSFPtr) (string&,string::iterator&,const vector<string>&)
     * @brief Points to a quality score unpacking function
     */
    char  XChar_hdr;          /**< @brief Extra char if header's length > 39 */
    char  XChar_qs;           /**< @brief Extra char if q scores length > 39 */
    pos_t begPos;             /**< @brief Begining position for each thread */
    u64   chunkSize;          /**< @brief Chunk size */
    vector<string> hdrUnpack; /**< @brief Lookup table for unpacking headers */
    vector<string> qsUnpack;  /**< @brief Lookup table for unpacking q scores */
    void (*unpackHdrFPtr) (string&, string::iterator&, const vector<string>&);
    void (*unpackQSFPtr)  (string&, string::iterator&, const vector<string>&);
};

/**
 * @brief Encryption / Decryption
 */
class EnDecrypto
{
    /**
     * @var   bool verbose
     * @brief Verbose mode     @hideinitializer
     * @var   bool disable_shuffle
     * @brief Disable shuffle  @hideinitializer
     */
public:
    bool   verbose = false;
    bool   disable_shuffle = false;
    byte   n_threads;                         /**< @brief Number of threads */
    string inFileName;                        /**< @brief Input file name */
    string keyFileName;                       /**< @brief Password file name */
    
    EnDecrypto          () = default;         /**< @brief Default constructor */
    void   decrypt      ();                   /**< @brief Decrypt */
    void   compressFA   ();                   /**< @brief Compress FASTA */
    void   decompressFA ();                   /**< @brief Decompress FASTA */
    void   compressFQ   ();                   /**< @brief Compress FASTQ */
    void   decompressFQ ();                   /**< @brief Decompress FASTQ */
    
    /**
     * @var   bool shufflingInProgress
     * @brief Shuffle in progress  @hideinitializer
     * @var   bool justPlus
     * @brief If line 3 is just +  @hideinitializer
     */
private:
    bool   shufflingInProgress = true;
    bool   justPlus = true;
    bool   shuffled = true;                       /**< @hideinitializer */
    u64    seed_shared;                           /**< @brief Shared seed */
    string Hdrs;                                  /**< @brief Max: 39 values */
    string QSs;                                   /**< @brief Max: 39 values */
    string HdrsX;                                 /**< @brief Extended Hdrs */
    string QSsX;                                  /**< @brief Extended QSs */
    htbl_t HdrMap;                                /**< @brief Hdrs hash table */
    htbl_t QsMap;                                 /**< @brief QSs hash table */
    u32    BlockLine;                             /**< @brief Max block lines */
    
    inline void encrypt       ();                       /**< @brief Encrypt */
    inline void buildIV       (byte*, const string&);   /**< @brief Build IV */
    inline void buildKey      (byte*, const string&);   /**< @brief Build key */
    inline void printIV       (byte*)            const; /**< @brief Print IV */
    inline void printKey      (byte*)            const;  // print key
    inline string extractPass ()                 const;  // extract password
    inline bool hasFQjustPlus ()                 const;  // check '+' line
    inline void gatherHdrBs   (string &);                // gather hdr Base - FA
    inline void gatherHdrQs   (string&, string&);        // gather hdrs & qss
    inline void my_srand      (u32);                     // random no. seed
    inline int  my_rand       ();                        // random no generate
    inline std::minstd_rand0 &randomEngine ();           // random no. engine
//    inline u64  un_shuffleSeedGen (const u32);         // (un)shuffle seed gen
    inline void un_shuffleSeedGen ();                    // (un)shuffle seed gen
    inline void shufflePkd    (string&);                 // shuffle packed
    inline void unshufflePkd  (string::iterator&, u64);  // unshuffle packed
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