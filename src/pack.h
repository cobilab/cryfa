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

extern htable_t QS_MAP;

/*******************************************************************************
    build hash table for quality scores
*******************************************************************************/
inline void buildQsHashTable(const string &QUALITY_SCORES, short keyLen)
{
    cerr << QUALITY_SCORES << '\n';
    
    
    const byte len = QUALITY_SCORES.length();
    
    
    ULL nElement = 0;
    string element;
    switch (keyLen)
    {
        case 2:
            //todo. insert into hash table
            LOOP2(i, j, len)
            {
//                cerr << QUALITY_SCORES[i] << QUALITY_SCORES[j] << ' ';
                    element = std::to_string(QUALITY_SCORES[i]);
                    element += std::to_string(QUALITY_SCORES[j]);
                QS_MAP.insert({element,nElement});
                ++nElement;
            }
            break;
            
//        case 3:
//            LOOP3(i, j, k, len)
//            {
//                cerr << QUALITY_SCORES[i] << QUALITY_SCORES[j]
//                     << QUALITY_SCORES[k] << ' ';
//                ++nElement;
//            }
//            break;
//
//        case 4:
//            LOOP4(i, j, k, l, len)
//            {
//                cerr << QUALITY_SCORES[i] << QUALITY_SCORES[j]
//                     << QUALITY_SCORES[k] << QUALITY_SCORES[l] << ' ';
//                ++nElement;
//            }
//            break;
//
//        case 5:
//            LOOP5(i, j, k, l, m, len)
//            {
//                cerr << QUALITY_SCORES[i] << QUALITY_SCORES[j]
//                     << QUALITY_SCORES[k] << QUALITY_SCORES[l]
//                     << QUALITY_SCORES[m] << ' ';
//                ++nElement;
//            }
//            break;
//
//        case 8:
//            LOOP8(i, j, k, l, m, n, o, p, len)
//            {
//                cerr << QUALITY_SCORES[i] << QUALITY_SCORES[j]
//                     << QUALITY_SCORES[k] << QUALITY_SCORES[l]
//                     << QUALITY_SCORES[m] << QUALITY_SCORES[n]
//                     << QUALITY_SCORES[o] << QUALITY_SCORES[p] << ' ';
//                ++nElement;
//            }
//            break;
    
        default:    break;
    }
    
    
    
    cerr<<'\n'<<(int)len<<'\t'<<nElement;
}



//fqHeaderPack
/*******************************************************************************
    index of each 3 DNA bases pack
*******************************************************************************/
inline unsigned int dnaPack (const string &dna)
{
    htable_t::const_iterator got = DNA_MAP.find(dna);
    if (got == DNA_MAP.end()) { cerr << "Error: key not found!\n"; return 0; }
    else  return got->second;
}

/*******************************************************************************
    index of each 4 quality scores pack
*******************************************************************************/
inline unsigned int qsPack (const string &qs)
{
    htable_t::const_iterator got = QS_MAP.find(qs);
    if (got == QS_MAP.end()) { cerr << "Error: key not found!\n"; return 0; }
    else  return got->second;
}

/*******************************************************************************
    encapsulate each 3 DNA bases in 1 byte -- reduction: ~2/3
*******************************************************************************/
inline string packSeq_3to1 (string seq)
{
    string packedSeq;
    const  LL iterLen = seq.length() - 2;
    LL     x = 0;
    bool   firstNotIn, secondNotIn, thirdNotIn;
    char   s0, s1, s2;
    string triplet;
    
    for (x = 0; x < iterLen; x += 3)
    {
        s0 = seq[x], s1 = seq[x+1], s2 = seq[x+2];
        
        triplet.clear();
        triplet +=
           (firstNotIn = (s0!='A' && s0!='C' && s0!='G' && s0!='T' && s0!='N'))
           ? 'X' : s0;
        triplet +=
           (secondNotIn = (s1!='A' && s1!='C' && s1!='G' && s1!='T' && s1!='N'))
           ? 'X' : s1;
        triplet +=
           (thirdNotIn = (s2!='A' && s2!='C' && s2!='G' && s2!='T' && s2!='N'))
           ? 'X' : s2;
        
        packedSeq += (char) dnaPack(triplet);
        if (firstNotIn)  packedSeq += s0;
        if (secondNotIn) packedSeq += s1;
        if (thirdNotIn)  packedSeq += s2;
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

/*******************************************************************************
    encapsulate 3 quality scores in 2 bytes -- reduction ~1/3. 17 <= No.QS <= 40
*******************************************************************************/
inline string packQS_3to2 (string qs)
{
    return "3to2";
    
    
//    string packedQS;
//    const  LL iterLen = qs.length() - 3;
//    LL     x = 0;
//    bool   firstNotIn, secondNotIn, thirdNotIn, fourthNotIn;
//    char   s0, s1, s2, s3;
//    string quadruplet;
//
//    for (x = 0; x < iterLen; x += 4)
//    {
//        s0 = qs[x], s1 = qs[x+1], s2 = qs[x+2], s3 = qs[x+3];
//
//        //todo. be jaye ~ ye char>126 bezar, masalan 127
//        quadruplet.clear();
////        quadruplet +=
////           (firstNotIn  = (QUALITY_SCORES.find(s0)==string::npos)) ? '~' : s0;
////        quadruplet +=
////           (secondNotIn = (QUALITY_SCORES.find(s1)==string::npos)) ? '~' : s1;
////        quadruplet +=
////           (thirdNotIn  = (QUALITY_SCORES.find(s2)==string::npos)) ? '~' : s2;
////        quadruplet +=
////           (fourthNotIn = (QUALITY_SCORES.find(s3)==string::npos)) ? '~' : s3;
//
////     packedQS += (char) dnaPack(triplet); function: hash index -> (c0)(c1)(c2)
////                              function: hash index -> (c0)(c1) .unsigned short
//        if (firstNotIn)   packedQS += s0;
//        if (secondNotIn)  packedQS += s1;
//        if (thirdNotIn)   packedQS += s2;
//        if (fourthNotIn)  packedQS += s3;
//    }
//
////    // if seq len isn't multiple of 3, add (char) 254 before each sym
////    if (seq.length() % 3 == 1)
////    {
////        packedQS += 255;   packedQS += seq[x];
////    }
////    else if (seq.length() % 3 == 2)
////    {
////        packedQS += 255;   packedQS += seq[x];
////        packedQS += 255;   packedQS += seq[x+1];
////    }
//
//    return packedQS;
}

/*******************************************************************************
    encapsulate 2 quality scores in 1 bytes -- reduction ~1/2.  7 <= No.QS <= 16
*******************************************************************************/
inline string packQS_2to1 (string qs)
{
    string packedQs;
    const  LL iterLen = qs.length() - 1;
    LL     x = 0;
    string duplex;
    
    //todo. sakhte combination: for e tu dar tu
    // for i
    //  for j
    //   'i','j'
    
    
    for (x = 0; x < iterLen; x += 2)
    {
        duplex.clear();   duplex += qs[x];   duplex += qs[x+1];

//        cerr<<duplex << ' ';
        
//        packedQs += (char) qsPack(duplex);
    }

//    // if qs len isn't multiple of 2, add (char) 254 before each sym
//    if (seq.length() % 3 == 1)
//    {
//        packedQs += 255;   packedQs += seq[x];
//    }

    return packedQs;
}

/*******************************************************************************
    encapsulate 3 quality scores in 1 bytes -- reduction ~2/3.      No.QS = 5, 6
*******************************************************************************/
inline string packQS_3to1 (string qs)
{
}

/*******************************************************************************
    encapsulate 4 quality scores in 1 bytes -- reduction ~3/4.         No.QS = 4
*******************************************************************************/
inline string packQS_4to1 (string qs)
{
}

/*******************************************************************************
    encapsulate 5 quality scores in 1 bytes -- reduction ~4/5.         No.QS = 3
*******************************************************************************/
inline string packQS_5to1 (string qs)
{
}

/*******************************************************************************
    encapsulate 8 quality scores in 1 bytes -- reduction ~7/8.         No.QS = 2
*******************************************************************************/
inline string packQS_8to1 (string qs)
{
}

#endif //CRYFA_PACK_H