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
 * @brief Packing FASTA
 */
struct packfa_s
{
    /** @brief Points to a header packing function */
    void (*packHdrFPtr) (string&, const string&, const htbl_t&);
};

/**
 * @brief Packing FASTQ
 */
struct packfq_s
{
    /**  @brief Points to a header packing function */
    void (*packHdrFPtr) (string&, const string&, const htbl_t&);
    void (*packQSFPtr)  (string&, const string&, const htbl_t&);
    /**< @brief Points to a quality score packing function */
};

/**
 * @brief Unpakcing FASTA
 */
struct unpackfa_s
{
    char  XChar_hdr;           /**< @brief Extra char if header's length > 39 */
    pos_t begPos;              /**< @brief Begining position for each thread */
    u64   chunkSize;           /**< @brief Chunk size */
    vector<string> hdrUnpack;  /**< @brief Lookup table for unpacking headers */
    void (*unpackHdrFPtr) (string&, string::iterator&, const vector<string>&);
    /**< @brief Points to a header unpacking function */
};

/**
 * @brief Unpakcing FASTQ
 */
struct unpackfq_s
{
    char  XChar_hdr;          /**< @brief Extra char if header's length > 39 */
    char  XChar_qs;           /**< @brief Extra char if q scores length > 39 */
    pos_t begPos;             /**< @brief Begining position for each thread */
    u64   chunkSize;          /**< @brief Chunk size */
    vector<string> hdrUnpack; /**< @brief Lookup table for unpacking headers */
    vector<string> qsUnpack;  /**< @brief Lookup table for unpacking q scores */
    /**  @brief Points to a header unpacking function */
    void (*unpackHdrFPtr) (string&, string::iterator&, const vector<string>&);
    void (*unpackQSFPtr)  (string&, string::iterator&, const vector<string>&);
    /**< @brief Points to a quality score unpacking function */
};

/**
 * @brief Encryption / Decryption
 */
class EnDecrypto
{
public:
    /**
     * @var   bool verbose
     * @brief Verbose mode     @hideinitializer
     * @var   bool disable_shuffle
     * @brief Disable shuffle  @hideinitializer
     */
    bool   verbose = false;
    bool   disable_shuffle = false;
    byte   n_threads;                         /**< @brief Number of threads */
    string inFileName;                        /**< @brief Input file name */
    string keyFileName;                       /**< @brief Password file name */
    
    EnDecrypto          () = default;         // Default constructor
    void   decrypt      ();                   // Decrypt
    void   compressFA   ();                   // Compress FASTA
    void   decompressFA ();                   // Decompress FASTA
    void   compressFQ   ();                   // Compress FASTQ
    void   decompressFQ ();                   // Decompress FASTQ
    
    
    
    //todo added
    inline void buildHashTable (htbl_t&, const string&, short);
    inline void buildUnpack(vector<string>&, const string&, u16);
    inline byte dnaPack (const string&);
    inline u16 largePack (const string&, const htbl_t&);
    inline void packSeq_3to1 (string&, const string&);
    inline void packLargeHdr_3to2 (string&, const string&, const htbl_t&);
    inline void packLargeQs_3to2 (string&, const string&, const htbl_t&);
    inline void pack_3to2 (string&, const string&, const htbl_t&);
    inline void pack_2to1 (string&, const string&, const htbl_t&);
    inline void pack_3to1 (string&, const string&, const htbl_t&);
    inline void pack_5to1 (string&, const string&, const htbl_t&);
    inline void pack_7to1 (string&, const string&, const htbl_t&);
    inline void pack_1to1 (string&, const string&, const htbl_t&);
    inline char penaltySym (char);
    inline void unpackSeqFA_3to1 (string&, string::iterator&);
    inline void unpackSeqFQ_3to1 (string&, string::iterator&);
    inline void unpackLarge_read2B (string&, string::iterator&, char, const vector<string>&);
    inline void unpack_read2B (string&, string::iterator&, const vector<string>&);
    inline void unpack_read1B (string&, string::iterator&, const vector<string>&);





private:
    /**
     * @var   bool shufflingInProgress
     * @brief Shuffle in progress  @hideinitializer
     * @var   bool justPlus
     * @brief If line 3 is just +  @hideinitializer
     */
    bool   shufflingInProgress = true;
    bool   justPlus = true;
    bool   shuffled = true;                   /**< @hideinitializer */
    u64    seed_shared;                       /**< @brief Shared seed */
    string Hdrs;                              /**< @brief Max: 39 values */
    string QSs;                               /**< @brief Max: 39 values */
    string HdrsX;                             /**< @brief Extended Hdrs */
    string QSsX;                              /**< @brief Extended QSs */
    htbl_t HdrMap;                            /**< @brief Hdrs hash table */
    htbl_t QsMap;                             /**< @brief QSs hash table */
    u32    BlockLine;                         /**< @brief Max block lines */
    
    inline void encrypt       ();                        // Encrypt
    inline void buildIV       (byte*, const string&);    // Build IV
    inline void buildKey      (byte*, const string&);    // Build key
    inline void printIV       (byte*)            const;  // Print IV
    inline void printKey      (byte*)            const;  // Print key
    inline string extractPass ()                 const;  // Extract password
    inline bool hasFQjustPlus ()                 const;  // Check '+' line
    inline void gatherHdrBs   (string &);                // Gather hdr Base - FA
    inline void gatherHdrQs   (string&, string&);        // Gather hdrs & qss
    inline void my_srand      (u32);                     // Random no. seed
    inline int  my_rand       ();                        // Random no generate
    inline std::minstd_rand0 &randomEngine ();           // Random no. engine
//    inline u64  un_shuffleSeedGen (const u32);         // (Un)shuffle seed gen
    inline void un_shuffleSeedGen ();                    // (Un)shuffle seed gen
    inline void shufflePkd    (string&);                 // Shuffle packed
    inline void unshufflePkd  (string::iterator&, u64);  // Unshuffle packed
    inline void packFA        (const packfa_s&,   byte); // Pack FA
    inline void unpackHS      (const unpackfa_s&, byte); // Unpack H:Small -- FA
    inline void unpackHL      (const unpackfa_s&, byte); // Unpack H:Large -- FA
    inline void packFQ        (const packfq_s&,   byte); // Pack FQ
    inline void unpackHSQS    (const unpackfq_s&, byte); // Unpack H:Small, Q:S
    inline void unpackHSQL    (const unpackfq_s&, byte); // Unpack H:S, Q:Large
    inline void unpackHLQS    (const unpackfq_s&, byte); // Unpack H:Large, Q:S
    inline void unpackHLQL    (const unpackfq_s&, byte); // Unpack H:Large, Q:L
};

#endif //CRYFA_ENDECRYPTO_H