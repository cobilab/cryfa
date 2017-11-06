/**
 * @file      FASTQ.h
 * @brief     Compression/Decompression of FASTQ
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FASTQ_H
#define CRYFA_FASTQ_H

#include "EnDecrypto.h"
#include "Security.h"


/** @brief Packing FASTQ */
struct packfq_s
{
    /**  @brief Points to a header packing function */
    void (EnDecrypto::*packHdrFPtr) (string&, const string&, const htbl_t&);
    void (EnDecrypto::*packQSFPtr)  (string&, const string&, const htbl_t&);
    /**< @brief Points to a quality score packing function */
};

/** @brief Unpakcing FASTQ */
struct unpackfq_s
{
    char  XChar_hdr;          /**< @brief Extra char if header's length > 39 */
    char  XChar_qs;           /**< @brief Extra char if q scores length > 39 */
    pos_t begPos;             /**< @brief Begining position for each thread */
    u64   chunkSize;          /**< @brief Chunk size */
    vector<string> hdrUnpack; /**< @brief Lookup table for unpacking headers */
    vector<string> qsUnpack;  /**< @brief Lookup table for unpacking q scores */
    /**  @brief Points to a header unpacking function */
    void (EnDecrypto::*unpackHdrFPtr)
         (string&, string::iterator&, const vector<string>&);
    void (EnDecrypto::*unpackQSFPtr)
         (string&, string::iterator&, const vector<string>&);
    /**< @brief Points to a quality score unpacking function */
};

/**
 * @brief Compression/Decompression of FASTQ
 */
class FASTQ : public EnDecrypto
{
public:
    FASTQ            () = default;
    void compress    ();
    void decompress  ();

private:
    bool justPlus = true;   /**< @brief If line 3 is just +  @hideinitializer */
    
    bool hasJustPlus ()  const;
    void gatherHdrQs (string&, string&);
    void pack        (const packfq_s&,   byte);
    void unpackHSQS  (const unpackfq_s&, byte);
    void unpackHSQL  (const unpackfq_s&, byte);
    void unpackHLQS  (const unpackfq_s&, byte);
    void unpackHLQL  (const unpackfq_s&, byte);
};

#endif //CRYFA_FASTQ_H