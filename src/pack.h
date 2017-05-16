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

htable_t HDR_MAP;
htable_t QS_MAP;
string*  HDR_UNPACK;
string*  QS_UNPACK;
string   HEADERS;           // 39 values
string   QUALITY_SCORES;    // 39 values

/*******************************************************************************
    build hash table
*******************************************************************************/
inline void buildHashTable (htable_t &map, const string &strIn, short keyLen)
{
    const byte strInLen = strIn.length();
    ULL elementNo = 0;
    string element;
    
    switch (keyLen)
    {
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
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        case 5:
            LOOP5(i, j, k, l, m, strInLen)
            {
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];   element += strIn[l];
                element += strIn[m];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        case 7:
            LOOP7(i, j, k, l, m, n, o, strInLen)
            {
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];   element += strIn[l];
                element += strIn[m];   element += strIn[n];
                element += strIn[o];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        case 4:
            LOOP4(i, j, k, l, strInLen)
            {
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];   element += strIn[l];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        case 6:
            LOOP6(i, j, k, l, m, n, strInLen)
            {
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];   element += strIn[l];
                element += strIn[m];   element += strIn[n];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
        
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strInLen)
            {
                element  = strIn[i];   element += strIn[j];
                element += strIn[k];   element += strIn[l];
                element += strIn[m];   element += strIn[n];
                element += strIn[o];   element += strIn[p];
                
                map.insert(make_pair(element, elementNo));
                ++elementNo;
            }
            break;
            
        default: break;
    }
    
    // TEST
//    htable_t::const_iterator got = map.find("II!");
//    if (got == map.end()) cerr << "Error: key not found!\n";
//    else  cerr << got->second;

//    for (htable_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << elementNo << '\n';
}

/*******************************************************************************
    build table for unpacking headers
*******************************************************************************/
inline void buildHdrUnpack (const string &headers, short keyLen)
{
    const byte HdrLen = headers.length();
    ULL elementNo = 0;
    string element;
    ULL arrSize = pow(HdrLen, keyLen);   // size of HDR_UNPACK
    HDR_UNPACK = new string[arrSize];

    switch (keyLen)
    {
        case 2:
            LOOP2(i, j, HdrLen)
            {
                element = headers[i];   element += headers[j];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 3:
            LOOP3(i, j, k, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];   element += headers[l];
                element += headers[m];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];   element += headers[l];
                element += headers[m];   element += headers[n];
                element += headers[o];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 4:
            LOOP4(i, j, k, l, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];   element += headers[l];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];   element += headers[l];
                element += headers[m];   element += headers[n];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, HdrLen)
            {
                element  = headers[i];   element += headers[j];
                element += headers[k];   element += headers[l];
                element += headers[m];   element += headers[n];
                element += headers[o];   element += headers[p];

                HDR_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;

        default: break;
    }

    // test
//    for (int i = 0; i != arrSize; ++i)
//        cerr << HDR_UNPACK[i] << '\n';
}

/*******************************************************************************
    build table for unpacking quality scores
*******************************************************************************/
inline void buildQsUnpack (const string &quality_scores, short keyLen)
{
    const byte QSLen = quality_scores.length();
    ULL elementNo = 0;
    string element;
    ULL arrSize = pow(QSLen, keyLen);   // size of QS_UNPACK
    QS_UNPACK = new string[arrSize];
    
    switch (keyLen)
    {
        case 2:
            LOOP2(i, j, QSLen)
            {
                element = quality_scores[i];   element += quality_scores[j];
                
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 3:
            LOOP3(i, j, k, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 5:
            LOOP5(i, j, k, l, m, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];   element += quality_scores[l];
                element += quality_scores[m];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 7:
            LOOP7(i, j, k, l, m, n, o, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];   element += quality_scores[l];
                element += quality_scores[m];   element += quality_scores[n];
                element += quality_scores[o];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 4:
            LOOP4(i, j, k, l, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];   element += quality_scores[l];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 6:
            LOOP6(i, j, k, l, m, n, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];   element += quality_scores[l];
                element += quality_scores[m];   element += quality_scores[n];
    
                QS_UNPACK[elementNo] = element;
                ++elementNo;
            }
            break;
            
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, QSLen)
            {
                element  = quality_scores[i];   element += quality_scores[j];
                element += quality_scores[k];   element += quality_scores[l];
                element += quality_scores[m];   element += quality_scores[n];
                element += quality_scores[o];   element += quality_scores[p];
    
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
inline string packLarge_3to2 (string strIn, string SYM_RANGE, htable_t &map)
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
    
//        shortTuple = hdrLargePack(tuple);
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
inline string pack_3to2 (string strIn, string SYM_RANGE, htable_t &map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 2;
    LL x = 0;
    unsigned short shortTuple;
    
    for (x = 0; x < iterLen; x += 3)
    {
        tuple.clear();   tuple = strIn[x];   tuple += strIn[x+1];   tuple += strIn[x+2];
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
inline string pack_2to1 (string strIn, string SYM_RANGE, htable_t &map)
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
    if (strIn.length() & 1)
    {
        packed += 255;   packed += strIn[x];
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 3 symbols in 1 bytes -- reduction ~2/3.              # = 4, 5, 6
*******************************************************************************/
inline string pack_3to1 (string strIn, string SYM_RANGE, htable_t &map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 2;
    LL x = 0;
    
    for (x = 0; x < iterLen; x += 3)
    {
        tuple.clear();  tuple = strIn[x];  tuple += strIn[x+1];  tuple += strIn[x+2];
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
inline string pack_5to1 (string strIn, string SYM_RANGE, htable_t &map)
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
inline string pack_7to1 (string strIn, string SYM_RANGE, htable_t &map)
{
    string tuple, packed;
    const LL iterLen = strIn.length() - 6;
    LL x = 0;
    
    for (x = 0; x < iterLen; x += 7)
    {
        tuple.clear();       tuple  = strIn[x];   tuple += strIn[x+1]; tuple += strIn[x+2];
        tuple += strIn[x+3]; tuple += strIn[x+4]; tuple += strIn[x+5]; tuple += strIn[x+6];
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
    penalty symbol
*******************************************************************************/
inline char penaltySym (char c)
{
    return (c != (char) 254 && c != (char) 252) ? c : (char) 10; //(char)10='\n'
}

/*******************************************************************************
    unpack 1 byte to 3 DNA bases -- FASTQ
*******************************************************************************/
inline void unpackSeqFQ_3to1 (string::iterator &i)
{
    string tpl;
    
    for (; *i != (char) 254; ++i)
    {
        //seq len not multiple of 3
        if (*i == (char) 255) { cout << penaltySym(*(++i));  continue; }
        
        tpl = DNA_UNPACK[(byte) *i];
    
        if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] != 'X')         // ...
            cout << tpl;
            
        else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] != 'X')    // X..
            cout << penaltySym(*(++i)) << tpl[1] << tpl[2];
            
        else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] != 'X')    // .X.
            cout << tpl[0] << penaltySym(*(++i)) << tpl[2];
            
        else if (tpl[0] == 'X' && tpl[1] == 'X' && tpl[2] != 'X')    // XX.
            cout << penaltySym(*(++i)) << penaltySym(*(++i)) << tpl[2];
            
        else if (tpl[0] != 'X' && tpl[1] != 'X' && tpl[2] == 'X')    // ..X
            cout << tpl[0] << tpl[1] << penaltySym(*(++i));
            
        else if (tpl[0] == 'X' && tpl[1] != 'X' && tpl[2] == 'X')    // X.X
            cout << penaltySym(*(++i)) << tpl[1] << penaltySym(*(++i));
            
        else if (tpl[0] != 'X' && tpl[1] == 'X' && tpl[2] == 'X')    // .XX
            cout << tpl[0] << penaltySym(*(++i)) << penaltySym(*(++i));
            
        else  cout << penaltySym(*(++i)) << penaltySym(*(++i))       // XXX
                   << penaltySym(*(++i));
    }
    cout << '\n';
}











/*******************************************************************************
    unpack headers by reading 2 byte by 2 byte, when #hdr > 39
*******************************************************************************/
inline void unpackHdrLarge_read2B (string::iterator &i, const char XChar,
                                   string &plusMore)
{
    byte leftB, rightB;     // left and right bytes
    unsigned short doubleB; // double byte
    string tpl;             // tuplet
    
    plusMore.clear();
    while (*i != (char) 254)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255)
        {
            cout << penaltySym(*(i+1));
            plusMore += penaltySym(*(i+1));
            i += 2;
            continue;
        }

        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes

        tpl = HDR_UNPACK[doubleB];

        if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] != XChar)        // ...
        {
            cout << tpl;
            plusMore += tpl;
            i += 2;
        }
        else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] != XChar)   // X..
        {
            cout << penaltySym(*(i+2)) << tpl[1] << tpl[2];
            plusMore += penaltySym(*(i+2));
            plusMore += tpl[1];
            plusMore += tpl[2];
            i += 3;
        }
        else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] != XChar)   // .X.
        {
            cout << tpl[0] << penaltySym(*(i+2)) << tpl[2];
            plusMore += tpl[0];
            plusMore += penaltySym(*(i+2));
            plusMore += tpl[2];
            i += 3;
        }
        else if (tpl[0] == XChar && tpl[1] == XChar && tpl[2] != XChar)   // XX.
        {
            cout << penaltySym(*(i+2)) << penaltySym(*(i+3)) << tpl[2];
            plusMore += penaltySym(*(i+2));
            plusMore += penaltySym(*(i+3));
            plusMore += tpl[2];
            i += 4;
        }
        else if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] == XChar)   // ..X
        {
            cout << tpl[0] << tpl[1] << penaltySym(*(i+2));
            plusMore += tpl[0];
            plusMore += tpl[1];
            plusMore += penaltySym(*(i+2));
            i += 3;
        }
        else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] == XChar)   // X.X
        {
            cout << penaltySym(*(i+2)) << tpl[1] << penaltySym(*(i+3));
            plusMore += penaltySym(*(i+2));
            plusMore += tpl[1];
            plusMore += penaltySym(*(i+3));
            i += 4;
        }
        else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] == XChar)   // .XX
        {
            cout << tpl[0] << penaltySym(*(i+2)) << penaltySym(*(i+3));
            plusMore += tpl[0];
            plusMore += penaltySym(*(i+2));
            plusMore += penaltySym(*(i+3));
            i += 4;
        }
        else                                                              // XXX
        {
            cout << penaltySym(*(i+2)) << penaltySym(*(i+3))
                 << penaltySym(*(i+4));
            plusMore += penaltySym(*(i+2));
            plusMore += penaltySym(*(i+3));
            plusMore += penaltySym(*(i+4));
            i += 5;
        }
    }
    cout << '\n';
}

/*******************************************************************************
    unpack headers by reading 2 byte by 2 byte
*******************************************************************************/
inline void unpackHdr_read2B (string::iterator &i, string &plusMore)
{
    byte leftB, rightB;     // left and right bytes
    unsigned short doubleB; // double byte
    
    plusMore.clear();
    for (; *i != (char) 254; i += 2)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255)
        {
            cout << penaltySym(*(i+1));
            plusMore += penaltySym(*(i+1));
            continue;
        }

        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes

        cout << HDR_UNPACK[doubleB];
        plusMore += HDR_UNPACK[doubleB];
    }
    cout << '\n';
}

/*******************************************************************************
    unpack headers by reading 1 byte by 1 byte
*******************************************************************************/
inline void unpackHdr_read1B (string::iterator &i, string &plusMore)
{
    plusMore.clear();
    for (; *i != (char) 254; ++i)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255)
        {
            cout << penaltySym(*(i+1));
            plusMore += penaltySym(*(i+1));
            continue;
        }

        cout << HDR_UNPACK[(byte) *i];
        plusMore += HDR_UNPACK[(byte) *i];
    }
    cout << '\n';
}

/*******************************************************************************
    unpack quality scores by reading 2 byte by 2 byte, when #QS > 39
*******************************************************************************/
inline void unpackQSLarge_read2B (string::iterator &i, const char XChar)
{
    byte leftB, rightB;     // left and right bytes
    unsigned short doubleB; // double byte
    string tpl;             // tuplet
    
    while (*i != (char) 254)
    {
        //seq len not multiple of keyLen
        if (*i == (char) 255) { cout << penaltySym(*(i+1));   i+=2;  continue; }

        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes
        
        tpl = QS_UNPACK[doubleB];
        
        if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] != XChar)        // ...
        { cout << tpl;                                                   i+=2; }
        
        else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] != XChar)   // X..
        { cout << penaltySym(*(i+2)) << tpl[1] << tpl[2];                i+=3; }
        
        else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] != XChar)   // .X.
        { cout << tpl[0] << penaltySym(*(i+2)) << tpl[2];                i+=3; }
        
        else if (tpl[0] == XChar && tpl[1] == XChar && tpl[2] != XChar)   // XX.
        { cout << penaltySym(*(i+2)) << penaltySym(*(i+3)) << tpl[2];    i+=4; }
        
        else if (tpl[0] != XChar && tpl[1] != XChar && tpl[2] == XChar)   // ..X
        { cout << tpl[0] << tpl[1] << penaltySym(*(i+2));                i+=3; }
        
        else if (tpl[0] == XChar && tpl[1] != XChar && tpl[2] == XChar)   // X.X
        { cout << penaltySym(*(i+2)) << tpl[1] << penaltySym(*(i+3));    i+=4; }
        
        else if (tpl[0] != XChar && tpl[1] == XChar && tpl[2] == XChar)   // .XX
        { cout << tpl[0] << penaltySym(*(i+2)) << penaltySym(*(i+3));    i+=4; }
        
        else { cout << penaltySym(*(i+2)) << penaltySym(*(i+3))           // XXX
                    << penaltySym(*(i+4));                               i+=5; }
    }
}

/*******************************************************************************
    unpack quality scores by reading 2 byte by 2 byte
*******************************************************************************/
inline void unpackQS_read2B (string::iterator &i)
{
    byte leftB, rightB;     // left and right bytes
    unsigned short doubleB; // double byte
    
    for (; *i != (char) 254; i += 2)
    {
        //seq len not multiple of keyLen
        if (*i == (char) 255) { cout << penaltySym(*(i+1));    continue; }
        
        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes
        
        cout << QS_UNPACK[doubleB];
    }
}

/*******************************************************************************
    unpack quality scores by reading 1 byte by 1 byte
*******************************************************************************/
inline void unpackQS_read1B (string::iterator &i)
{
    for (; *i != (char) 254; ++i)
    {
        //seq len not multiple of keyLen
        if (*i == (char) 255) { cout << penaltySym(*(++i));    continue; }
        
        cout << QS_UNPACK[(byte) *i];
    }
}

#endif //CRYFA_PACK_H