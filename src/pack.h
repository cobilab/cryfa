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

/*******************************************************************************
    build hash table
*******************************************************************************/
inline htable_t buildHashTable (htable_t map, const string &strIn, short keyLen)
{
    const byte strInLen = strIn.length();
    ULL elementNo = 0;
    string element;

    switch (keyLen)
    {
        case 1:
            LOOP(i, strInLen)
            {
                element = strIn[i];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 2:
            LOOP2(i, j, strInLen)
            {
                element = strIn[i];   element += strIn[j];
                map.insert(make_pair(element, elementNo));
////                map.insert({element, elementNo});
////                map[element] = elementNo;
                ++elementNo;
            }
            break;

        case 3:
            LOOP3(i, j, k, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                element += strIn[o];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strInLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                element += strIn[o];  element += strIn[p];
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;

        default: break;
    }
    
    return map;
    
    // TEST
//    htable_t::const_iterator got = map.find("II!");
//    if (got == map.end()) cerr << "Error: key not found!\n";
//    else  cerr << got->second;

//    for (htable_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << elementNo << '\n';
}

/*******************************************************************************
    build table for unpacking
*******************************************************************************/
inline string* buildUnpack (const string &strIn, short keyLen, string* unpack)
{
    const byte strLen = strIn.length();
    ULL elementNo = 0;
    string element;
    const ULL arrSize = pow(strLen, keyLen);
    unpack = new string[arrSize];
    
    switch (keyLen)
    {
        case 1:
            LOOP(i, strLen)
            {
                element = strIn[i];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 2:
            LOOP2(i, j, strLen)
            {
                element = strIn[i];   element += strIn[j];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 3:
            LOOP3(i, j, k, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        case 5:
            LOOP5(i, j, k, l, m, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        case 7:
            LOOP7(i, j, k, l, m, n, o, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                element += strIn[o];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        case 4:
            LOOP4(i, j, k, l, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        case 6:
            LOOP6(i, j, k, l, m, n, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strLen)
            {
                element  = strIn[i];  element += strIn[j];  element += strIn[k];
                element += strIn[l];  element += strIn[m];  element += strIn[n];
                element += strIn[o];  element += strIn[p];
                unpack[elementNo] = element;
                ++elementNo;
            }
            break;
        
        default: break;
    }
    
    return unpack;
    
    // test
//    for (int i = 0; i != arrSize; ++i)
//        cerr << unpack[i] << '\n';
}

/*******************************************************************************
    index of each DNA bases pack
*******************************************************************************/
inline unsigned short dnaPack (const string &dna)
{
    htable_t::const_iterator got = DNA_MAP.find(dna);
    if (got == DNA_MAP.end()) { cerr << "Error: key not found!\n"; return 0; }
    else  return got->second;
}

/*******************************************************************************
    index of each pack, when # > 39
*******************************************************************************/
inline unsigned short largePack (const string &strIn, htable_t &map)
{
    htable_t::const_iterator got = map.find(strIn);
    if (got == map.end()) { cerr << "Error: key not found!\n"; return 0; }
    else  return got->second;
}

/*******************************************************************************
    encapsulate each 3 DNA bases in 1 byte -- reduction: ~2/3
*******************************************************************************/
inline string packSeq_3to1 (string seq)
{
    string packedSeq;
    const LL iterLen = seq.length() - 2;
    LL x = 0;
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
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
    encapsulate 3 symbols in 2 bytes -- reduction ~1/3.                  40 <= #
*******************************************************************************/
inline string packLarge_3to2 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 2;
    LL x = 0;
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    unsigned short shortTuple;
    // ASCII char after the last char in SYM_RANGE string
    const char XChar = (char) (SYM_RANGE[SYM_RANGE.size()-1] + 1);
    
    for (x = 0; x < iterLen; x += 3)
    {
        s0 = strIn[x], s1 = strIn[x+1], s2 = strIn[x+2];
        
        tuple.clear();
        tuple  =(firstNotIn  = (SYM_RANGE.find(s0)==string::npos)) ? XChar : s0;
        tuple +=(secondNotIn = (SYM_RANGE.find(s1)==string::npos)) ? XChar : s1;
        tuple +=(thirdNotIn  = (SYM_RANGE.find(s2)==string::npos)) ? XChar : s2;
    
        shortTuple = largePack(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // if len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += strIn[x];
            break;
        
        case 2:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 3 symbols in 2 bytes -- reduction ~1/3.            16 <= # <= 39
*******************************************************************************/
inline string pack_3to2 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 2;
    LL x = 0;
    unsigned short shortTuple;
    
    for (x = 0; x < iterLen; x += 3)
    {
        tuple.clear();   tuple=strIn[x];  tuple+=strIn[x+1];  tuple+=strIn[x+2];
        shortTuple = map.find(tuple)->second;
        packed += (byte) (shortTuple >> 8);      // left byte
        packed += (byte) (shortTuple & 0xFF);    // right byte
    }
    
    // if len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += strIn[x];
            break;
        
        case 2:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 2 symbols in 1 bytes -- reduction ~1/2.             7 <= # <= 15
*******************************************************************************/
inline string pack_2to1 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 1;
    LL x = 0;

    for (x = 0; x < iterLen; x += 2)
    {
        tuple.clear();   tuple = strIn[x];   tuple += strIn[x+1];
        packed += (char) map.find(tuple)->second;
    }
    
    // if len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (strIn.length() & 1) { packed += 255;    packed += strIn[x]; }
    
    return packed;
}

/*******************************************************************************
    encapsulate 3 symbols in 1 bytes -- reduction ~2/3.              # = 4, 5, 6
*******************************************************************************/
inline string pack_3to1 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 2;
    LL x = 0;
    
    for (x = 0; x < iterLen; x += 3)
    {
        tuple.clear();   tuple=strIn[x];  tuple+=strIn[x+1];  tuple+=strIn[x+2];
        packed += (char) map.find(tuple)->second;
    }
    
    // if len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += strIn[x];
            break;
        
        case 2:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 5 symbols in 1 bytes -- reduction ~4/5.                    # = 3
*******************************************************************************/
inline string pack_5to1 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 4;
    LL x = 0;
    
    for (x = 0; x < iterLen; x += 5)
    {
        tuple.clear();        tuple  = strIn[x];    tuple += strIn[x+1];
        tuple += strIn[x+2];  tuple += strIn[x+3];  tuple += strIn[x+4];
        packed += (char) map.find(tuple)->second;
    }
    
    // if len isn't multiple of 5, add (char) 255 before each sym
    switch (strIn.length() % 5)
    {
        case 1:
            packed += 255;   packed += strIn[x];
            break;
        
        case 2:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            break;
        
        case 3:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            break;
        
        case 4:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            packed += 255;   packed += strIn[x+3];
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 7 symbols in 1 bytes -- reduction ~6/7.                    # = 2
*******************************************************************************/
inline string pack_7to1 (string strIn, string SYM_RANGE, htable_t map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 6;
    LL x = 0;
    
    for (x = 0; x < iterLen; x += 7)
    {
        tuple.clear();          tuple  = strIn[x];      tuple += strIn[x+1];
        tuple += strIn[x+2];    tuple += strIn[x+3];    tuple += strIn[x+4];
        tuple += strIn[x+5];    tuple += strIn[x+6];
        packed += (char) map.find(tuple)->second;
    }
    
    // if len isn't multiple of 7, add (char) 255 before each sym
    switch (strIn.length() % 7)
    {
        case 1:
            packed += 255;   packed += strIn[x];
            break;
        
        case 2:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            break;
        
        case 3:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            break;
        
        case 4:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            packed += 255;   packed += strIn[x+3];
            break;
        
        case 5:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            packed += 255;   packed += strIn[x+3];
            packed += 255;   packed += strIn[x+4];
            break;
        
        case 6:
            packed += 255;   packed += strIn[x];
            packed += 255;   packed += strIn[x+1];
            packed += 255;   packed += strIn[x+2];
            packed += 255;   packed += strIn[x+3];
            packed += 255;   packed += strIn[x+4];
            packed += 255;   packed += strIn[x+5];
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    show 1 symbol in 1 byte.                                               # = 1
*******************************************************************************/
inline string pack_1to1 (string strIn, string SYM_RANGE, htable_t map)
{
    string single;
    string packed;
    const LL iterLen = strIn.length();
    LL x = 0;
    
    for (x = 0; x < iterLen; ++x)
    {
        single.clear();   single = strIn[x];
        packed += (char) map.find(single)->second;
    }
    
    return packed;
}

/*******************************************************************************
    penalty symbol -- returns either input char or (char)10='\n'
*******************************************************************************/
inline char penaltySym (char c)
{
    return (c != (char) 254 && c != (char) 252) ? c : (char) 10;
}

/*******************************************************************************
    unpack 1 byte to 3 DNA bases -- FASTQ
*******************************************************************************/
inline string unpackSeqFQ_3to1 (string::iterator &i)
{
    string tpl, out;
    
    for (; *i != (char) 254; ++i)
    {
        //seq len not multiple of 3
        if (*i == (char) 255) { out += penaltySym(*(++i));  continue; }
        
        tpl = DNA_UNPACK[(byte) *i];
        
        if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')    out+=tpl;       // ...
        
        else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')               // X..
        { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2]; }
        
        else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')               // .X.
        {out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2]; }
        
        else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')               // XX.
        {out+=penaltySym(*(++i));    out+=penaltySym(*(++i));    out+=tpl[2]; }
        
        else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')               // ..X
        { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i)); }
        
        else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')               // X.X
        { out+=penaltySym(*(++i));    out+=tpl[1];    out+=penaltySym(*(++i)); }
        
        else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')               // .XX
        { out+=tpl[0];    out+=penaltySym(*(++i));    out+=penaltySym(*(++i)); }
        
        else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));       // XXX
               out+=penaltySym(*(++i)); }
    }
    
    return out;
}

/*******************************************************************************
    unpack by reading 2 byte by 2 byte, when # > 39
*******************************************************************************/
inline string unpackLarge_read2B (string::iterator &i, const char XChar,
                                  string* unpack)
{
    byte MSB, LSB;
    unsigned short doubleB; // double byte
    string tpl;             // tuplet
    string out;
    
    while (*i != (char) 254)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));   i+=2;   continue; }
        
        MSB     = *i;
        LSB     = *(i+1);
        doubleB = MSB<<8 | LSB;    // join two bytes
        
        tpl = unpack[doubleB];
        
        if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]!=XChar)              // ...
        { out+=tpl;                                                      i+=2; }
        
        else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]!=XChar)         // X..
        { out+=penaltySym(*(i+2));    out+=tpl[1];    out+=tpl[2];       i+=3; }
        
        else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]!=XChar)         // .X.
        { out+=tpl[0];    out+=penaltySym(*(i+2));    out+=tpl[2];       i+=3; }
        
        else if (tpl[0]==XChar && tpl[1]==XChar && tpl[2]!=XChar)         // XX.
        { out+=penaltySym(*(i+2)); out+=penaltySym(*(i+3)); out+=tpl[2]; i+=4; }
        
        else if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]==XChar)         // ..X
        { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(i+2));       i+=3; }
        
        else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]==XChar)         // X.X
        { out+=penaltySym(*(i+2)); out+=tpl[1]; out+=penaltySym(*(i+3)); i+=4; }
        
        else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]==XChar)         // .XX
        { out+=tpl[0]; out+=penaltySym(*(i+2)); out+=penaltySym(*(i+3)); i+=4; }
        
        else { out+=penaltySym(*(i+2));    out+=penaltySym(*(i+3));       // XXX
               out+=penaltySym(*(i+4));                                  i+=5; }
    }
    
    return out;
}

/*******************************************************************************
    unpack by reading 2 byte by 2 byte
*******************************************************************************/
inline string unpack_read2B (string::iterator &i, string* unpack)
{
    byte MSB, LSB;
    unsigned short doubleB; // double byte
    string out;
    
    for (; *i != (char) 254; i += 2)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    continue; }
        
        MSB     = *i;
        LSB     = *(i+1);
        doubleB = MSB<<8 | LSB;    // join two bytes
        
        out += unpack[doubleB];
    }
    
    return out;
}

/*******************************************************************************
    unpack by reading 1 byte by 1 byte
*******************************************************************************/
inline string unpack_read1B (string::iterator &i, string* unpack)
{
    string out;
    
    for (; *i != (char) 254; ++i)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(++i));    continue; }
        out += unpack[(byte) *i];
    }

    return out;
}

#endif //CRYFA_PACK_H