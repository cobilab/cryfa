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

*******************************************************************************/
inline int DNA_PACK (const string &DNA)
{
    htable_t::const_iterator got = mymap.find(DNA);
    if (got == mymap.end())
    {
        cerr << "Error: key not found!\n";
        exit(1);
    }
    else  return got->second;
    
    return -1;
}

/*******************************************************************************
    encapsulate 3 DNA bases in 1 byte
*******************************************************************************/
inline string PackIn3bDNASeq (string seq)
{
    string packedSeq;
    const  LL iterLen  = seq.length() - 2;//cerr<<seq.length()<<' ';//todo. test
//    cerr<<seq.length(); //todo. test
    LL     x = 0;
    bool   firstIsX, secondIsX, thirdIsX;
    char   sym0, sym1, sym2;        /// to keep 3 symbols
    string triplet;
    
    for (x = 0; x < iterLen; x += 3)
    {
        firstIsX = false, secondIsX = false, thirdIsX = false;
        sym0 = seq[x], sym1 = seq[x+1], sym2 = seq[x+2];

        if ( !(sym0=='A' || sym0=='C' || sym0=='G' || sym0=='T' || sym0=='N') )
        {
            firstIsX = true;
            sym0 = 'X';
        }
        if ( !(sym1=='A' || sym1=='C' || sym1=='G' || sym1=='T' || sym1=='N') )
        {
            secondIsX = true;
            sym1 = 'X';
        }
        if ( !(sym2=='A' || sym2=='C' || sym2=='G' || sym2=='T' || sym2=='N') )
        {
            thirdIsX = true;
            sym2 = 'X';
        }

        triplet = "";
        triplet += sym0;
        triplet += sym1;
        triplet += sym2;

        packedSeq += (char) DNA_PACK(triplet);
        if (firstIsX)  packedSeq += seq[x];
        if (secondIsX) packedSeq += seq[x+1];
        if (thirdIsX)  packedSeq += seq[x+2];
    }

//    packedSeq += (int) 244;
//    x = seq.length() - 3 - rem;
//    while (x < seq.length())     packedSeq += seq[x++];


    // if seq len isn't multiplicant of 3, add (char) 254
    // to packedSeq, before each sym
//    if (seq.length() == 1)
//    {
//        packedSeq += 255;   packedSeq += seq[0];
//    }
//    else if (seq.length() == 2)
//    {
//        packedSeq += 255;   packedSeq += seq[0];
//        packedSeq += 255;   packedSeq += seq[1];
//    }
//    else
    if (seq.length() % 3 == 1)
    {
        packedSeq += 255;   packedSeq += seq[x];
    }
    else if (seq.length() % 3 == 2)
    {
        packedSeq += 255;   packedSeq += seq[x];
        packedSeq += 255;   packedSeq += seq[x+1];
    }
    
//    cerr<<packedSeq; //todo. test
    
    return packedSeq;
}

#endif //CRYFA_PACK_H