/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Packing
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef CRYFA_PACK_H
#define CRYFA_PACK_H

#include <iostream>
#include "def.h"
using std::string;
using std::cout;
using std::cerr;

/*******************************************************************************
    index of each 3 based DNA pack
*******************************************************************************/
inline unsigned int dnaPack (const string &DNA)
{
    htable_t::const_iterator got = DNA_MAP.find(DNA);
    if (got == DNA_MAP.end()) { cerr << "Error: key not found!\n"; return 0; }
    else  return got->second;
}

/*******************************************************************************
    encapsulate each 3 DNA bases in 1 byte
*******************************************************************************/
inline string pack3bases (string seq)
{
    string packedSeq;
    const  LL iterLen = seq.length() - 2;
    LL     x = 0;
    bool   firstIsX, secondIsX, thirdIsX;
    char   s0, s1, s2;
    string triplet;
    
    for (x = 0; x < iterLen; x += 3)
    {
        s0 = seq[x], s1 = seq[x+1], s2 = seq[x+2];
        
        triplet.clear();
        triplet +=
             (firstIsX = (s0!='A' && s0!='C' && s0!='G' && s0!='T' && s0!='N'))
             ? 'X' : s0;
        triplet +=
             (secondIsX = (s1!='A' && s1!='C' && s1!='G' && s1!='T' && s1!='N'))
             ? 'X' : s1;
        triplet +=
             (thirdIsX = (s2!='A' && s2!='C' && s2!='G' && s2!='T' && s2!='N'))
             ? 'X' : s2;
        
        packedSeq += (char) dnaPack(triplet);
        if (firstIsX)  packedSeq += s0;
        if (secondIsX) packedSeq += s1;
        if (thirdIsX)  packedSeq += s2;
    }

    // if seq len isn't multiple of 3, add (char) 254 before each sym
    if (seq.length() % 3 == 1)
    {
        packedSeq += 255;   packedSeq += seq[x];
    }
    else if (seq.length() % 3 == 2)
    {
        packedSeq += 255;   packedSeq += seq[x];
        packedSeq += 255;   packedSeq += seq[x+1];
    }
    
    return packedSeq;
}

#endif //CRYFA_PACK_H