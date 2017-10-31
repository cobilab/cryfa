/**
 * @file      Security.h
 * @brief     Security
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_SECURITY_H
#define CRYFA_SECURITY_H

#include "def.h"


/**
 * @brief Security
 */
class Security
{
public:
    static bool   disableShuffle; /**< @brief Disable shuffle @hideinitializer*/
    static bool   verbose;        /**< @brief Verbose mode    @hideinitializer*/
    static string inFileName;     /**< @brief Input file name */
    static string keyFileName;    /**< @brief Password file name */

    void   decrypt ();
    
protected:
    bool shuffInProgress=true;/**< @brief Shuffle in progress @hideinitializer*/
    bool shuffled       =true;/**< @hideinitializer */

    void encrypt       ();
    void shuffle       (string&);
    void unshuffle     (string::iterator&, u64);

private:
    u64  seed_shared;         /**< @brief Shared seed */

    string extractPass ()  const;
    void newSrand      (u32);
    int  newRand       ();
    std::minstd_rand0 &randomEngine ();
//    inline u64 shuffSeedGen (const u32);
    void shuffSeedGen  ();
    void buildIV       (byte*, const string&);
    void buildKey      (byte*, const string&);
    void printIV       (byte*)  const;
    void printKey      (byte*)  const;
};

#endif //CRYFA_SECURITY_H