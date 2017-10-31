/**
 * @file      EnDecrypto.cpp
 * @brief     Encryption/Decryption
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <fstream>
#include <functional>
#include <algorithm>
#include "EnDecrypto.h"

using std::vector;
using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::to_string;
using std::stoull;


/**
 * @brief      Build a hash table
 * @param[out] map     Hash table
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::buildHashTbl (htbl_t &map, const string &strIn, short keyLen)
{
    u64 elementNo = 0;
    string element;    element.reserve(keyLen);
    map.clear();
    map.reserve((u64) std::pow(strIn.size(), keyLen));
    
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
    
    
    // Test
//    for (htbl_t::iterator i = map.begin(); i != map.end(); ++i)
//        cerr << i->first << "\t" << i->second << '\n';
//    cerr << "elementNo = " << elementNo << '\n';
}

/**
 * @brief      Build a table for unpacking
 * @param[out] unpack  Table (vector of strings)
 * @param[in]  strIn   The string including the keys
 * @param[in]  keyLen  Length of the keys
 */
void EnDecrypto::buildUnpackTbl (vector<string> &unpack, const string &strIn,
                                 u16 keyLen)
{
    string element;    element.reserve(keyLen);
    unpack.clear();
    unpack.reserve((u64) std::pow(strIn.size(), keyLen));
    
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
    
    
//    // Test
//    u64 arrSize = (u64) std::pow(strIn.size(), keyLen);
//    for (u64 i = 0; i != arrSize; ++i)
//        cerr << unpack[i] << '\n';
}

/**
 * @brief  Index of each DNA bases pack
 * @param  key  Key
 * @return Value (based on the idea of key-value in a hash table)
 */
// Maybe byte <-> u16 replacement
byte EnDecrypto::dnaPackIndex (const string &key)
{
    htbl_t::const_iterator got = DNA_MAP.find(key);
    if (got == DNA_MAP.end())
    { cerr << "Error: key '" << key << "'not found!\n";    return 0; }
    else  return (byte) got->second;
}

/**
 * @brief  Index of each pack, when # > 39
 * @param  key  Key
 * @param  map  Hash table
 * @return Value (based on the idea of key-value in a hash table)
 */
u16 EnDecrypto::largePackIndex (const string &key, const htbl_t &map)
{
    htbl_t::const_iterator got = map.find(key);
    if (got == map.end())
    { cerr << "Error: key '" << key << "' not found!\n";    return 0; }
    else  return (u16) got->second;
}

/**
 * @brief      Encapsulate each 3 DNA bases in 1 byte. Reduction: ~2/3
 * @param[out] packedSeq  Packed sequence
 * @param[in]  seq        Sequence
 */
void EnDecrypto::packSeq (string &packedSeq, const string &seq)
{
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    string tuple;   tuple.reserve(3);
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
        
        packedSeq += dnaPackIndex(tuple);
        if (firstNotIn)  packedSeq += s0;
        if (secondNotIn) packedSeq += s1;
        if (thirdNotIn)  packedSeq += s2;
    }
    
    // If seq len isn't multiple of 3, add (char) 255 before each sym
    switch (seq.length() % 3)
    {
        case 1:
            packedSeq+=(char) 255;    packedSeq+=*i;
            break;

        case 2:
            packedSeq+=(char) 255;    packedSeq+=*i;
            packedSeq+=(char) 255;    packedSeq+=*(i+1);
            break;
            
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 header symbols in 2 bytes, when # >= 40.
 *             -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed  Packed header
 * @param[in]  strIn   Header
 * @param[in]  map     Hash table
 */
void EnDecrypto::packLHdrFaFq (string &packed, const string &strIn,
                               const htbl_t &map)
{
    packLarge(packed, strIn, Hdrs, map);
}

/**
 * @brief      Encapsulate 3 quality score symbols in 2 bytes, when # >= 40.
 *             -- FASTQ. Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Quality scores
 * @param[in]  map     Hash table
 */
void EnDecrypto::packLQsFq (string &packed, const string &strIn,
                            const htbl_t &map)
{
    packLarge(packed, strIn, QSs, map);
}

/**
 * @brief      Encapsulate 3 header/quality score symbols in 2 bytes,
 *             when # >= 40 -- FASTA/FASTQ. Reduction ~1/3
 * @param[out] packed  Packed qulity scores
 * @param[in]  strIn   Input header/quality score
 * @param[in]  hdrQs   Collection of headers/quality scores
 * @param[in]  map     Hash table
 */
inline void EnDecrypto::packLarge (string &packed, const string &strIn,
                                   const string &hdrQs, const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    bool firstNotIn, secondNotIn, thirdNotIn;
    char s0, s1, s2;
    u16 shortTuple;
    string hQ = hdrQs;    // header/quality score
    // ASCII char after the last char in QUALITY_SCORES string
    const char XChar = (char) (hQ.back() + 1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        s0 = *i,    s1 = *(i+1),  s2 = *(i+2);
        
        tuple.clear();
        tuple  = (firstNotIn  = (hQ.find(s0)==string::npos)) ? XChar : s0;
        tuple += (secondNotIn = (hQ.find(s1)==string::npos)) ? XChar : s1;
        tuple += (thirdNotIn  = (hQ.find(s2)==string::npos)) ? XChar : s2;
        
        shortTuple = largePackIndex(tuple, map);
        packed += (unsigned char) (shortTuple >> 8);      // Left byte
        packed += (unsigned char) (shortTuple & 0xFF);    // Right byte
        
        if (firstNotIn)   packed += s0;
        if (secondNotIn)  packed += s1;
        if (thirdNotIn)   packed += s2;
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;
        
        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;
        
        default: break;
    }
}

/**
 * @brief      Encapsulate 3 symbols in 2 bytes, when 16 <= # <= 39.
 *             Reduction ~1/3
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_3to2 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    u16 shortTuple;
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;
    
    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        shortTuple = (u16) map.find(tuple)->second;
        packed += (byte) (shortTuple >> 8);      // Left byte
        packed += (byte) (shortTuple & 0xFF);    // Right byte
    }
    
    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 2 symbols in 1 byte, when 7 <= # <= 15.
 *             Reduction ~1/2
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_2to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(2);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-1;
    
    for (; i < iEnd; i += 2)
    {
        tuple.clear();    tuple = *i;    tuple += *(i+1);
        packed += (char) map.find(tuple)->second;
    }
    
    // If len isn't multiple of 2 (it's odd), add (char) 255 before each sym
    if (strIn.length() & 1) { packed+=(char) 255;    packed+=*i; }
}

/**
 * @brief         Encapsulate 3 symbols in 1 byte, when # = 4, 5, 6.
 *                Reduction ~2/3
 * @param packed  Packed string
 * @param strIn   Input string
 * @param map     Hash table
 */
void EnDecrypto::pack_3to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(3);
    
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-2;

    for (; i < iEnd; i += 3)
    {
        tuple.clear();    tuple=*i;    tuple+=*(i+1);    tuple+=*(i+2);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 3, add (char) 255 before each sym
    switch (strIn.length() % 3)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 5 symbols in 1 byte, when # = 3. Reduction ~4/5
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_5to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(5);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-4;
    
    for (; i < iEnd; i += 5)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);
        tuple+=*(i+2);    tuple+=*(i+3);    tuple+=*(i+4);
        packed += (char) map.find(tuple)->second;
    }
    
    // If len isn't multiple of 5, add (char) 255 before each sym
    switch (strIn.length() % 5)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;
            
        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;
            
        case 3:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            break;

        case 4:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 7 symbols in 1 byte, when # = 2. Reduction ~6/7
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_7to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string tuple;    tuple.reserve(7);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end()-6;
    
    for (; i < iEnd; i += 7)
    {
        tuple.clear();    tuple=*i;         tuple+=*(i+1);    tuple+=*(i+2);
        tuple+=*(i+3);    tuple+=*(i+4);    tuple+=*(i+5);    tuple+=*(i+6);
        packed += (char) map.find(tuple)->second;
    }

    // If len isn't multiple of 7, add (char) 255 before each sym
    switch (strIn.length() % 7)
    {
        case 1:
            packed+=(char) 255;    packed+=*i;
            break;

        case 2:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            break;

        case 3:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            break;

        case 4:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            break;

        case 5:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            packed+=(char) 255;    packed+=*(i+4);
            break;

        case 6:
            packed+=(char) 255;    packed+=*i;
            packed+=(char) 255;    packed+=*(i+1);
            packed+=(char) 255;    packed+=*(i+2);
            packed+=(char) 255;    packed+=*(i+3);
            packed+=(char) 255;    packed+=*(i+4);
            packed+=(char) 255;    packed+=*(i+5);
            break;

        default: break;
    }
}

/**
 * @brief      Encapsulate 1 symbol in 1 byte, when # = 1.
 * @param[out] packed  Packed string
 * @param[in]  strIn   Input string
 * @param[in]  map     Hash table
 */
void EnDecrypto::pack_1to1 (string &packed, const string &strIn,
                            const htbl_t &map)
{
    string single;    single.reserve(1);
    string::const_iterator i = strIn.begin(),   iEnd = strIn.end();
    
    for (; i < iEnd; ++i)
    {
        single.clear();   single = *i;
        packed += (char) map.find(single)->second;
    }
}

/**
 * @brief  Penalty symbol
 * @param  c  Input char
 * @return Input char or (char)10='\\n'
 */
char EnDecrypto::penaltySym (char c)
{
    const char lookupTable[2] = {c, (char) 10};
    return lookupTable[c==(char) 254 || c==(char) 252];

//    // More readable; Perhaps slower, because of conditional branch
//    return (c != (char) 254 && c != (char) 252) ? c : (char) 10;
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte, when # > 39
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  XChar   Extra character for unpacking
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpackLarge (string &out, string::iterator &i,
                              char XChar, const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;                      // Double byte
    string tpl;    tpl.reserve(3);    // Tuplet
    out.clear();

    while (*i != (char) 254)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    i+=2; }
        else
        {
            leftB   = (byte) *i;
            rightB  = (byte) *(i+1);
            doubleB = leftB<<8 | rightB;    // Join two bytes
            
            tpl = unpack[doubleB];
            
            if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]!=XChar)          // ...
            { out+=tpl;                                                  i+=2; }
            
            else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]!=XChar)     // X..
            { out+=penaltySym(*(i+2));    out+=tpl[1];    out+=tpl[2];   i+=3; }
            
            else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]!=XChar)     // .X.
            { out+=tpl[0];    out+=penaltySym(*(i+2));    out+=tpl[2];   i+=3; }
            
            else if (tpl[0]==XChar && tpl[1]==XChar && tpl[2]!=XChar)     // XX.
            { out+=penaltySym(*(i+2));  out+=penaltySym(*(i+3));  out+=tpl[2];
                                                                         i+=4; }
            
            else if (tpl[0]!=XChar && tpl[1]!=XChar && tpl[2]==XChar)     // ..X
            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(i+2));   i+=3; }
            
            else if (tpl[0]==XChar && tpl[1]!=XChar && tpl[2]==XChar)     // X.X
            { out+=penaltySym(*(i+2));  out+=tpl[1];  out+=penaltySym(*(i+3));
                                                                         i+=4; }
            
            else if (tpl[0]!=XChar && tpl[1]==XChar && tpl[2]==XChar)     // .XX
            { out+=tpl[0];  out+=penaltySym(*(i+2));  out+=penaltySym(*(i+3));
                                                                         i+=4; }
            
            else { out+=penaltySym(*(i+2));    out+=penaltySym(*(i+3));   // XXX
                   out+=penaltySym(*(i+4));                              i+=5; }
        }
    }
}

/**
 * @brief      Unpack by reading 2 byte by 2 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_2B (string &out, string::iterator &i,
                            const vector<string> &unpack)
{
    byte leftB, rightB;
    u16 doubleB;     // Double byte
    out.clear();
    
    for (; *i != (char) 254; i += 2)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(i+1));    continue; }
        
        leftB   = (byte) *i;
        rightB  = (byte) *(i+1);
        doubleB = leftB<<8 | rightB;    // Join two bytes
        
        out += unpack[doubleB];
    }
}

/**
 * @brief      Unpack by reading 1 byte by 1 byte
 * @param[out] out     Unpacked string
 * @param[in]  i       Input string iterator
 * @param[in]  unpack  Table for unpacking
 */
void EnDecrypto::unpack_1B (string &out, string::iterator &i,
                            const vector<string> &unpack)
{
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        // Hdr len not multiple of keyLen
        if (*i == (char) 255) { out += penaltySym(*(++i));    continue; }
        out += unpack[(byte) *i];
    }
}

/**
 * @brief      Unpack 1 byte to 3 DNA bases
 * @param[out] out  DNA bases
 * @param[in]  i    Input string iterator
 */
void EnDecrypto::unpackSeq (string &out, string::iterator &i)
{
    string tpl;    tpl.reserve(3);     // Tuplet
    out.clear();
    
    for (; *i != (char) 254; ++i)
    {
        if (*i == (char) 255)                       // Seq len not multiple of 3
            out += penaltySym(*(++i));
        else
        {
            tpl = DNA_UNPACK[(byte) *i];
            
            if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]!='X')                // ...
            { out+=tpl;                                                        }
                // Using just one 'out' makes trouble
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]!='X')           // X..
            { out+=penaltySym(*(++i));    out+=tpl[1];    out+=tpl[2];         }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]!='X')           // .X.
            { out+=tpl[0];    out+=penaltySym(*(++i));    out+=tpl[2];         }
            
            else if (tpl[0]=='X' && tpl[1]=='X' && tpl[2]!='X')           // XX.
            { out+=penaltySym(*(++i));  out+=penaltySym(*(++i));  out+=tpl[2]; }
            
            else if (tpl[0]!='X' && tpl[1]!='X' && tpl[2]=='X')           // ..X
            { out+=tpl[0];    out+=tpl[1];    out+=penaltySym(*(++i));         }
            
            else if (tpl[0]=='X' && tpl[1]!='X' && tpl[2]=='X')           // X.X
            { out+=penaltySym(*(++i));  out+=tpl[1];  out+=penaltySym(*(++i)); }
            
            else if (tpl[0]!='X' && tpl[1]=='X' && tpl[2]=='X')           // .XX
            { out+=tpl[0];  out+=penaltySym(*(++i));  out+=penaltySym(*(++i)); }
            
            else { out+=penaltySym(*(++i));    out+=penaltySym(*(++i));   // XXX
                   out+=penaltySym(*(++i));                                    }
        }
    }
}