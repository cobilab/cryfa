/**
 * @file      EnDecrypto.h
 * @brief     Encryption/Decryption
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
 * @brief Encryption/Decryption
 */
class EnDecrypto
{
public:
    static bool   verbose;        /**< @brief Verbose mode    @hideinitializer*/
    static bool   disable_shuffle;/**< @brief Disable shuffle @hideinitializer*/
    static byte   n_threads;      /**< @brief Number of threads */
    static string inFileName;     /**< @brief Input file name */
    static string keyFileName;    /**< @brief Password file name */
    
    EnDecrypto             () = default;
    void decrypt           ();
    void packLargeHdr_3to2 (string&, const string&, const htbl_t&);
    void packLargeQs_3to2  (string&, const string&, const htbl_t&);
    void pack_3to2         (string&, const string&, const htbl_t&);
    void pack_2to1         (string&, const string&, const htbl_t&);
    void pack_3to1         (string&, const string&, const htbl_t&);
    void pack_5to1         (string&, const string&, const htbl_t&);
    void pack_7to1         (string&, const string&, const htbl_t&);
    void pack_1to1         (string&, const string&, const htbl_t&);
    void unpack_read2B     (string&, string::iterator&, const vector<string>&);
    void unpack_read1B     (string&, string::iterator&, const vector<string>&);
    void unpackSeq         (string&, string::iterator&);

protected:
    bool shuffInProgress=true;/**< @brief Shuffle in progress @hideinitializer*/
    bool   justPlus     =true;/**< @brief If line 3 is just + @hideinitializer*/
    bool   shuffled     =true;/**< @hideinitializer */
    u64    seed_shared;       /**< @brief Shared seed */
    string Hdrs;              /**< @brief Max: 39 values */
    string QSs;               /**< @brief Max: 39 values */
    string HdrsX;             /**< @brief Extended Hdrs */
    string QSsX;              /**< @brief Extended QSs */
    htbl_t HdrMap;            /**< @brief Hdrs hash table */
    htbl_t QsMap;             /**< @brief QSs hash table */
    u32    BlockLine;         /**< @brief Max block lines */

    void encrypt            ();
    void buildIV            (byte*, const string&);
    void buildKey           (byte*, const string&);
    void printIV            (byte*)                 const;
    void printKey           (byte*)                 const;
    string extractPass      ()                      const;
    bool hasFQjustPlus      ()                      const;
    void buildHashTable     (htbl_t&, const string&, short);
    void buildUnpack        (vector<string>&, const string&, u16);
    byte dnaPack            (const string&);
    u16  largePack          (const string&, const htbl_t&);
    void packSeq_3to1       (string&, const string&);
    char penaltySym         (char);
    void unpackLarge_read2B (string&, string::iterator&, char,
                             const vector<string>&);
    void my_srand           (u32);
    int  my_rand            ();
    std::minstd_rand0 &randomEngine ();
//    inline u64  un_shuffleSeedGen (const u32);
    void un_shuffleSeedGen  ();
    void shufflePkd         (string&);
    void unshufflePkd       (string::iterator&, u64);
};

#endif //CRYFA_ENDECRYPTO_H