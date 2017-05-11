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
using std::make_pair;

htable_t QS_MAP;    // global
string*  QS_UNPACK; // global

/*******************************************************************************
    build hash table for quality scores
*******************************************************************************/
inline void buildQsHashTable (const string &QUALITY_SCORES, short keyLen)
{
    const byte QSLen = QUALITY_SCORES.length();
    ULL elementNo = 0;
    string element;
    
    switch (keyLen)
    {
        case 2:
            LOOP2(i, j, QSLen)
            {
                element = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                
                QS_MAP.insert(make_pair(element, elementNo));
////                QS_MAP.insert({element, elementNo});
////                QS_MAP[element] = elementNo;
                ++elementNo;
            }
            break;
            
        case 3:
            LOOP3(i, j, k, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];
                
                QS_MAP.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 4:
            LOOP4(i, j, k, l, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
                
                QS_MAP.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
                element += QUALITY_SCORES[m];
                
                QS_MAP.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
                element += QUALITY_SCORES[m];   element += QUALITY_SCORES[n];
                element += QUALITY_SCORES[o];   element += QUALITY_SCORES[p];
                
                QS_MAP.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        default: break;
    }
    
    // test
//    for (htable_t::iterator i = QS_MAP.begin(); i != QS_MAP.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
}

/*******************************************************************************
    build table for unpacking quality scores
*******************************************************************************/
inline void buildQsUnpack (const string &QUALITY_SCORES, short keyLen)
{
    const byte QSLen = QUALITY_SCORES.length();
    ULL elementNo = 0;
    string element;
    ULL arrSize = pow(QSLen, keyLen);   // size of QS_UNPACK
    QS_UNPACK = new string[arrSize];
    
    switch (keyLen)
    {
        case 2:
            LOOP2(i, j, QSLen)
            {
                element = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 3:
            LOOP3(i, j, k, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 4:
            LOOP4(i, j, k, l, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 5:
            LOOP5(i, j, k, l, m, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
                element += QUALITY_SCORES[m];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, QSLen)
            {
                element  = QUALITY_SCORES[i];   element += QUALITY_SCORES[j];
                element += QUALITY_SCORES[k];   element += QUALITY_SCORES[l];
                element += QUALITY_SCORES[m];   element += QUALITY_SCORES[n];
                element += QUALITY_SCORES[o];   element += QUALITY_SCORES[p];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
        
        default: break;
    }
    
    // test
//    for (int i = 0; i != arrSize; ++i)
//        cerr << QS_UNPACK[i] << '\n';
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
    string tuple;
    
    for (x = 0; x < iterLen; x += 3)
    {
        s0 = seq[x], s1 = seq[x+1], s2 = seq[x+2];
        
        tuple.clear();
        tuple +=
           (firstNotIn = (s0!='A' && s0!='C' && s0!='G' && s0!='T' && s0!='N'))
           ? 'X' : s0;
        tuple += 
           (secondNotIn = (s1!='A' && s1!='C' && s1!='G' && s1!='T' && s1!='N'))
           ? 'X' : s1;
        tuple +=
           (thirdNotIn = (s2!='A' && s2!='C' && s2!='G' && s2!='T' && s2!='N'))
           ? 'X' : s2;
        
        packedSeq += (char) dnaPack(tuple);
        if (firstNotIn)  packedSeq += s0;
        if (secondNotIn) packedSeq += s1;
        if (thirdNotIn)  packedSeq += s2;
    }
    
    // if seq len isn't multiple of 3, add (char) 255 before each sym
    switch (seq.length() % 3)
    {
        case 1:
            packedSeq += 255;   packedSeq += seq[x];
            break;
        
        case 2:
            packedSeq += 255;   packedSeq += seq[x];
            packedSeq += 255;   packedSeq += seq[x+1];
            break;
        
        default: break;
    }
    
    return packedSeq;
}

/*******************************************************************************
    encapsulate 3 quality scores in 2 bytes -- reduction ~1/3. 16 <= No.QS <= 40
*******************************************************************************/
inline string packQS_3to2 (string qs)
{
    return "3to2";
    
    
//    string packedQS;
//    const  LL iterLen = qs.length() - 3;
//    LL     x = 0;
//    bool   firstNotIn, secondNotIn, thirdNotIn, fourthNotIn;
//    char   s0, s1, s2, s3;
//    string tuple;
//
//    for (x = 0; x < iterLen; x += 4)
//    {
//        s0 = qs[x], s1 = qs[x+1], s2 = qs[x+2], s3 = qs[x+3];
//
//        //todo. be jaye ~ ye char>126 bezar, masalan 127
//        tuple.clear();
////        tuple +=
////           (firstNotIn  = (QUALITY_SCORES.find(s0)==string::npos)) ? '~' : s0;
////        tuple +=
////           (secondNotIn = (QUALITY_SCORES.find(s1)==string::npos)) ? '~' : s1;
////        tuple +=
////           (thirdNotIn  = (QUALITY_SCORES.find(s2)==string::npos)) ? '~' : s2;
////        tuple +=
////           (fourthNotIn = (QUALITY_SCORES.find(s3)==string::npos)) ? '~' : s3;
//
//       //todo. KOLLAN be jaye (char) bas function tabdile 'second' be 1 ya chand char benevisi
////     packedQS += (char) dnaPack(tuple); function: hash index -> (c0)(c1)(c2)
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
    encapsulate 2 quality scores in 1 bytes -- reduction ~1/2.  7 <= No.QS <= 15
*******************************************************************************/
inline string packQS_2to1 (string qs)
{
    string packedQs;
    const  LL iterLen = qs.length() - 1;
    LL     x = 0;
    string tuple;
    
    for (x = 0; x < iterLen; x += 2)
    {
        tuple.clear();   tuple = qs[x];   tuple += qs[x+1];
        packedQs += (char) qsPack(tuple);
    }
    
    // if qs len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (qs.length() & 1)
    {
        packedQs += 255;   packedQs += qs[x];
    }

    return packedQs;
}

/*******************************************************************************
    encapsulate 3 quality scores in 1 bytes -- reduction ~2/3.   No.QS = 4, 5, 6
*******************************************************************************/
inline string packQS_3to1 (string qs)
{
    string packedQs;
    const  LL iterLen = qs.length() - 2;
    LL     x = 0;
    string tuple;
    
    for (x = 0; x < iterLen; x += 3)
    {
        tuple.clear();   tuple = qs[x];   tuple += qs[x+1];   tuple += qs[x+2];
        packedQs += (char) qsPack(tuple);
    }
    
    // if qs len isn't multiple of 3, add (char) 255 before each sym
    switch (qs.length() % 3)
    {
        case 1:
            packedQs += 255;   packedQs += qs[x];
            break;
        
        case 2:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            break;
        
        default: break;
    }
    
    return packedQs;
}

/*******************************************************************************
    encapsulate 5 quality scores in 1 bytes -- reduction ~4/5.         No.QS = 3
*******************************************************************************/
inline string packQS_5to1 (string qs)
{
    string packedQs;
    const  LL iterLen = qs.length() - 4;
    LL     x = 0;
    string tuple;
    
    for (x = 0; x < iterLen; x += 5)
    {
        tuple.clear();     tuple  = qs[x];    tuple += qs[x+1];
        tuple += qs[x+2];  tuple += qs[x+3];  tuple += qs[x+4];
        packedQs += (char) qsPack(tuple);
    }
    
    // if qs len isn't multiple of 5, add (char) 255 before each sym
    switch (qs.length() % 5)
    {
        case 1:
            packedQs += 255;   packedQs += qs[x];
            break;
        
        case 2:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            break;
        
        case 3:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            break;
        
        case 4:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            packedQs += 255;   packedQs += qs[x+3];
            break;
        
        default: break;
    }
    
    return packedQs;
}

/*******************************************************************************
    encapsulate 7 quality scores in 1 bytes -- reduction ~6/7.         No.QS = 2
*******************************************************************************/
inline string packQS_7to1 (string qs)
{
    string packedQs;
    const  LL iterLen = qs.length() - 6;
    LL     x = 0;
    string tuple;
    
    for (x = 0; x < iterLen; x += 7)
    {
        tuple.clear();    tuple  = qs[x];   tuple += qs[x+1]; tuple += qs[x+2];
        tuple += qs[x+3]; tuple += qs[x+4]; tuple += qs[x+5]; tuple += qs[x+6];
        packedQs += (char) qsPack(tuple);
    }
    
    // if qs len isn't multiple of 7, add (char) 255 before each sym
    switch (qs.length() % 7)
    {
        case 1:
            packedQs += 255;   packedQs += qs[x];
            break;
            
        case 2:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            break;
            
        case 3:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            break;
            
        case 4:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            packedQs += 255;   packedQs += qs[x+3];
            break;
            
        case 5:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            packedQs += 255;   packedQs += qs[x+3];
            packedQs += 255;   packedQs += qs[x+4];
            break;
            
        case 6:
            packedQs += 255;   packedQs += qs[x];
            packedQs += 255;   packedQs += qs[x+1];
            packedQs += 255;   packedQs += qs[x+2];
            packedQs += 255;   packedQs += qs[x+3];
            packedQs += 255;   packedQs += qs[x+4];
            packedQs += 255;   packedQs += qs[x+5];
            break;
            
        default: break;
    }
    
    return packedQs;
}

#endif //CRYFA_PACK_H