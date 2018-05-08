/**
 * @file      FASTA.hpp
 * @brief     Compression/Decompression of FASTA
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FASTA_H
#define CRYFA_FASTA_H

#include "EnDecrypto.hpp"
#include "Security.hpp"


/** @brief Packing FASTA */
struct packfa_s
{
    packFP_t packHdrFP;        /**< @brief Points to a header packing function */
};

/** @brief Unpakcing FASTA */
struct unpackfa_s
{
    char           XChar_hdr;  /**< @brief Extra char if header's length > 39 */
    pos_t          begPos;     /**< @brief Begining position for each thread */
    u64            chunkSize;  /**< @brief Chunk size */
    vector<string> hdrUnpack;  /**< @brief Lookup table for unpacking headers */
    unpackFP_t     unpackHdrFP;/**< @brief Points to a header unpacking fn */
};

/**
 * @brief Compression/Decompression of FASTA
 */
class FASTA : public EnDecrypto
{
public:
    FASTA                       () = default;
    void compress               ();
    void decompress             ();

private:
    void gatherHdrBs            (string&);
    void set_hashTbl_packFn     (packfa_s&, const string&);
    void pack                   (const packfa_s&, byte);
    void set_unpackTbl_unpackFn (unpackfa_s&, const string&);
    void unpackHS               (const unpackfa_s&, byte);
    void unpackHL               (const unpackfa_s&, byte);
};

#endif //CRYFA_FASTA_H