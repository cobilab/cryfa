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
using std::vector;
using std::cout;
using std::cerr;
using std::make_pair;

string Hdrs_g;    // max: 39 values -- global
string QSs_g;     // max: 39 values -- global

/*******************************************************************************
    build hash table
*******************************************************************************/
inline htable_t buildHashTable (const string &strIn, short keyLen)
{
    ull elementNo = 0;
    string element;
    htable_t map;
    
    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                map.insert(make_pair(element, elementNo++));
////            map.insert({element, elementNo++});
////            map[element] = elementNo++;
            }
            break;
    
        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                map.insert(make_pair(element, elementNo++));
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                map.insert(make_pair(element, elementNo++));
            }
            break;
            
        default: break;
    }
    
    return map;
    
    // TEST
//    for (htable_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << elementNo << '\n';
}

/*******************************************************************************
    build table for unpacking
*******************************************************************************/
inline vector<string> buildUnpack (const string &strIn, us keyLen)
{
    ull elementNo = 0;
    string element;
    const ull unpackSize = std::pow(strIn.size(), keyLen);
    vector<string> unpack;    unpack.reserve(unpackSize);

    switch (keyLen)
    {
        case 3:
            LOOP3(i, j, k, strIn)
            {
                element=i;    element+=j;    element+=k;
                unpack.push_back(element);
            }
            break;

        case 2:
            LOOP2(i, j, strIn)
            {
                element=i;    element+=j;
                unpack.push_back(element);
            }
            break;

        case 1:
            LOOP(i, strIn)
            {
                element=i;
                unpack.push_back(element);
            }
            break;

        case 5:
            LOOP5(i, j, k, l, m, strIn)
            {
                element=i;  element+=j;  element+=k;  element+=l;  element+=m;
                unpack.push_back(element);
            }
            break;

        case 7:
            LOOP7(i, j, k, l, m, n, o, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;
                unpack.push_back(element);
            }
            break;

        case 4:
            LOOP4(i, j, k, l, strIn)
            {
                element=i;    element+=j;    element+=k;    element+=l;
                unpack.push_back(element);
            }
            break;

        case 6:
            LOOP6(i, j, k, l, m, n, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;
                unpack.push_back(element);
            }
            break;

        case 8:
            LOOP8(i, j, k, l, m, n, o, p, strIn)
            {
                element =i;  element+=j;  element+=k;  element+=l;  element+=m;
                element+=n;  element+=o;  element+=p;
                unpack.push_back(element);
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
inline us dnaPack (const string &dna)
{
    htable_t::const_iterator got = DNA_MAP.find(dna);
    if (got == DNA_MAP.end())
    { cerr << "Error: key '" << dna << "'not found!\n";    return 0; }
    else  return got->second;
}

/*******************************************************************************
    index of each pack, when # > 39
*******************************************************************************/
inline us largePack (const string &strIn, const htable_t &map)
{
    htable_t::const_iterator got = map.find(strIn);
    if (got == map.end())
    { cerr << "Error: key '" << strIn << "' not found!\n";    return 0; }
    else  return got->second;
}

/*******************************************************************************
    encapsulate each 3 DNA bases in 1 byte -- reduction: ~2/3
*******************************************************************************/
inline string packSeq_3to1 (const string& seq)
{
    string packedSeq;
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    string tuple;
    string::const_iterator i = seq.begin(),   iEnd = seq.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);

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
            packedSeq += 255;   packedSeq += *i;
            break;

        case 2:
            packedSeq += 255;   packedSeq += *i;
            packedSeq += 255;   packedSeq += *(i+1);
            break;

        default: break;
    }

    return packedSeq;
}

/*******************************************************************************
    encapsulate 3 header symbols in 2 bytes -- reduction ~1/3.           40 <= #
*******************************************************************************/
inline string packLargeHdr_3to2 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    us shortTuple;
    string hdrs = Hdrs_g;
    // ASCII char after the last char in HEADERS string
    const char XChar = (char) (hdrs.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),    s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (hdrs.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (hdrs.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (hdrs.find(s2)==string::npos)) ? XChar : s2;
        
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
            packed += 255;   packed += *i;
            break;
        
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 3 quality score symbols in 2 bytes -- reduction ~1/3.    40 <= #
*******************************************************************************/
inline string packLargeQs_3to2 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    us shortTuple;
    string qss = QSs_g;
    // ASCII char after the last char in QUALITY_SCORES string
    const char XChar = (char) (qss.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),  s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (qss.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (qss.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (qss.find(s2)==string::npos)) ? XChar : s2;
        
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
            packed += 255;   packed += *i;
            break;
        
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
        
        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 3 symbols in 2 bytes -- reduction ~1/3.            16 <= # <= 39
*******************************************************************************/
inline string pack_3to2 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    us shortTuple;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        shortTuple = map.find(tuple)->second;
        packed += (byte) (shortTuple >> 8);      // left byte
        packed += (byte) (shortTuple & 0xFF);    // right byte
    }
    
    // if len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;

        default: break;
    }

    return packed;
}

/*******************************************************************************
    encapsulate 2 symbols in 1 bytes -- reduction ~1/2.             7 <= # <= 15
*******************************************************************************/
inline string pack_2to1 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-1;
    
    for (; i < iEnd; i += 2)
    {
        tuple.clear();    tuple = *i;    tuple += *(i+1);
        packed += (char) map.find(tuple)->second;
    }
    
    // if len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (strIn.length() & 1) { packed += 255;    packed += *i; }

    return packed;
}

/*******************************************************************************
    encapsulate 3 symbols in 1 bytes -- reduction ~2/3.              # = 4, 5, 6
*******************************************************************************/
inline string pack_3to1 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        packed += (char) map.find(tuple)->second;
    }

    // if len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            break;

        default: break;
    }

    return packed;
}

/*******************************************************************************
    encapsulate 5 symbols in 1 bytes -- reduction ~4/5.                    # = 3
*******************************************************************************/
inline string pack_5to1 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-4;
    
    for (; i < iEnd; i += 5)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);
        tuple+=*(i+2);    tuple+=*(i+3);    tuple+=*(i+4);
        packed += (char) map.find(tuple)->second;
    }

    // if len isn't multiple of 5, add (char) 255 before each sym
    switch (strIn.length() % 5)
    {
        case 1:
            packed += 255;   packed += *i;
            break;
            
        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;
            
        case 3:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            break;

        case 4:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            break;

        default: break;
    }
    
    return packed;
}

/*******************************************************************************
    encapsulate 7 symbols in 1 bytes -- reduction ~6/7.                    # = 2
*******************************************************************************/
inline string pack_7to1 (const string& strIn, const htable_t &map)
{
    string tuple, packed;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-6;
    
    for (; i < iEnd; i += 7)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);    tuple+=*(i+2);
        tuple+=*(i+3);    tuple+=*(i+4);    tuple+=*(i+5);    tuple+=*(i+6);
        packed += (char) map.find(tuple)->second;
    }

    // if len isn't multiple of 7, add (char) 255 before each sym
    switch (strIn.length() % 7)
    {
        case 1:
            packed += 255;   packed += *i;
            break;

        case 2:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            break;

        case 3:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            break;

        case 4:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            break;

        case 5:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            packed += 255;   packed += *(i+4);
            break;

        case 6:
            packed += 255;   packed += *i;
            packed += 255;   packed += *(i+1);
            packed += 255;   packed += *(i+2);
            packed += 255;   packed += *(i+3);
            packed += 255;   packed += *(i+4);
            packed += 255;   packed += *(i+5);
            break;

        default: break;
    }

    return packed;
}

/*******************************************************************************
    show 1 symbol in 1 byte.                                               # = 1
*******************************************************************************/
inline string pack_1to1 (const string& strIn, const htable_t &map)
{
    string single, packed;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end();
    
    for (; i < iEnd; ++i)
    {
        single.clear();   single = *i;
        packed += (char) map.find(single)->second;
    }
    
    return packed;
}

/*******************************************************************************
    penalty symbol -- returns either input char or (char)10='\n'
*******************************************************************************/
inline constexpr char penaltySym (char c)
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
        { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2]; }

        else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')               // XX.
        { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));    out+=tpl[2]; }

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
                                  vector<string> &unpack)
{
    byte leftB, rightB;
    us doubleB;     // double byte
    string tpl;     // tuplet
    string out;

    while (*i != (char) 254)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));   i+=2;   continue; }
        
        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes
        
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
inline string unpack_read2B (string::iterator &i, vector<string> &unpack)
{
    byte leftB, rightB;
    us doubleB;     // double byte
    string out;
    
    for (; *i != (char) 254; i += 2)
    {
        // hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    continue; }

        leftB   = *i;
        rightB  = *(i+1);
        doubleB = leftB<<8 | rightB;    // join two bytes
        
        out += unpack[doubleB];
    }
    
    return out;
}

/*******************************************************************************
    unpack by reading 1 byte by 1 byte
*******************************************************************************/
inline string unpack_read1B (string::iterator &i, vector<string> &unpack)
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