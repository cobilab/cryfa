//
// Created by morteza on 02-05-2017.
//

#ifndef CRYFA_PACK_H
#define CRYFA_PACK_H


#include <iostream>
#include "def.h"

using std::string;
using std::cerr;

/*******************************************************************************

*******************************************************************************/
int DNA_PACK (string DNA)
{
    htable_t::const_iterator got = mymap.find( DNA );
    
    if (got == mymap.end())
    {
        cerr << "Error: key not found!\n";
        exit(1);
    }
    else    return got->second;
    
    return -1;
}


/*******************************************************************************
    encapsulate 3 DNA bases in 1 byte
*******************************************************************************/
string PackIn3bDNASeq (string seq)
{
    string packedSeq;
    ULL rest = seq.length() % 3;
    ULL seqSize = seq.length()-3-rest, x;
    bool first, second, third;
    char firstSym, secondSym, thirdSym;
    char seq0, seq1, seq2;        /// to keep 3 symbols
    
    for (x = 0; x < seqSize; x += 3)
    {
        first = false, second = false, third = false;
        seq0 = seq[x], seq1 = seq[x+1], seq2 = seq[x+2];
        
        if (seq0 != 'A' && seq0 != 'C' && seq0 != 'G'
            && seq0 != 'T' && seq0 != 'N')
        {
            first = true;
            firstSym = seq0;
            seq[x] = 'X';
        }
        
        if (seq1 != 'A' && seq1 != 'C' && seq1 != 'G'
            && seq1 != 'T' && seq1 != 'N')
        {
            second = true;
            secondSym = seq1;
            seq[x+1] = 'X';
        }
        
        if (seq2 != 'A' && seq2 != 'C' && seq2 != 'G'
            && seq2 != 'T' && seq2 != 'N')
        {
            third = true;
            thirdSym = seq2;
            seq[x+2] = 'X';
        }
        
        string triplet;
        triplet += seq[x  ];
        triplet += seq[x+1];
        triplet += seq[x+2];
        //triplet += '\0';
        
        packedSeq += (char) DNA_PACK(triplet);
        
        /*
        if (first)      packedSeq += firstSym;
        if (second)     packedSeq += secondSym;
        if (third)      packedSeq += thirdSym;
        */
    }
    
    packedSeq += (int) 244;
    x = seq.length() - 3 - rest;
    while (x < seq.length())     packedSeq += seq[x++];
    
    return packedSeq;
}


#endif //CRYFA_PACK_H
