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
#include "Security.h"

using std::string;
using std::vector;


class EnDecrypto;

// Type define
typedef void (EnDecrypto::*packFP_t) (string&, const string&, const htbl_t&);
typedef void (EnDecrypto::*unpackFP_t)
             (string&, string::iterator&, const vector<string>&);

/**
 * @brief Encryption/Decryption
 */
class EnDecrypto : public Security
{
public:
    EnDecrypto          () = default;
    void packLHdrFaFq   (string&, const string&, const htbl_t&);
    void packLQsFq      (string&, const string&, const htbl_t&);
    void pack_3to2      (string&, const string&, const htbl_t&);
    void pack_2to1      (string&, const string&, const htbl_t&);
    void pack_3to1      (string&, const string&, const htbl_t&);
    void pack_5to1      (string&, const string&, const htbl_t&);
    void pack_7to1      (string&, const string&, const htbl_t&);
    void pack_1to1      (string&, const string&, const htbl_t&);
    void unpack_2B      (string&, string::iterator&, const vector<string>&);
    void unpack_1B      (string&, string::iterator&, const vector<string>&);
    void shuffleFile    ();
    void unshuffleFile  ();
    
protected:
    string Hdrs;        /**< @brief Max: 39 values */
    string QSs;         /**< @brief Max: 39 values */
    string HdrsX;       /**< @brief Extended Hdrs */
    string QSsX;        /**< @brief Extended QSs */
    htbl_t HdrMap;      /**< @brief Hdrs hash table */
    htbl_t QsMap;       /**< @brief QSs hash table */
    u32    BlockLine;   /**< @brief Max block lines */
    
    void buildHashTbl   (htbl_t&, const string&, short);
    void buildUnpackTbl (vector<string>&, const string&, u16);
    byte dnaPackIndex   (const string&);
    u16  largePackIndex (const string&, const htbl_t&);
    void packSeq        (string&, const string&);
    void unpackSeq      (string&, string::iterator&);
    void unpackLarge    (string&, string::iterator&,char,const vector<string>&);
    void joinPackedFiles     (const string&, const string&, char, bool)   const;
    void joinUnpackedFiles   ()                                           const;
    void joinShuffledFiles   ()                                           const;
    void joinUnshuffledFiles ()                                           const;

private:
    inline void packLarge (string&, const string&, const string&,const htbl_t&);
    inline char penaltySym     (char);
    inline void shuffleBlock   (byte);
    inline void unshuffleBlock (byte);
};

#endif //CRYFA_ENDECRYPTO_H