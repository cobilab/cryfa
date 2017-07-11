/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Definitions
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef CRYFA_DEF_H
#define CRYFA_DEF_H

#include <iostream>
#include <unordered_map>    // hash table
#include <random>           // std::mt19937
using std::cout;
using std::string;
using std::unordered_map;

/*******************************************************************************
    version and release
*******************************************************************************/
#define VERSION_CRYFA 1
#define RELEASE_CRYFA 17.05

/*******************************************************************************
    typedefs
*******************************************************************************/
typedef unsigned char                     byte;
typedef unsigned short                    us;
typedef unsigned int                      ui;
typedef unsigned long long                ull;
typedef long long                         ll;
typedef std::mt19937                      rng_type;
typedef std::unordered_map<string, ull>   htable_t;
typedef std::char_traits<char>::pos_type  pos_t;    // tellg(), tellp()

/*******************************************************************************
    metaprograms
*******************************************************************************/
// power (B^E) -- base (B) and exponent (E) MUST be known at compile time
// usage: "cerr << POWER<3,2>::val;" which yields 9
template<ui B, ui E>
struct POWER
{ static const ull val = B * POWER<B, E-1>::val; };

template<ui B>
struct POWER<B, 0>
{ static const ull val = 1; };
//..............................................................................

/*******************************************************************************
    macros
*******************************************************************************/
//#define LOOP(i,S)            for(byte (i)=0; i!=(S); ++i)
#define LOOP(i,S)            for(const char& (i) : (S))
#define LOOP2(i,j,S)         LOOP(i,S) LOOP(j,S)
#define LOOP3(i,j,k,S)       LOOP(i,S) LOOP(j,S) LOOP(k,S)
#define LOOP4(i,j,k,l,S)     LOOP(i,S) LOOP(j,S) LOOP(k,S) LOOP(l,S)
#define LOOP5(i,j,k,l,m,S)   LOOP(i,S) LOOP(j,S) LOOP(k,S) LOOP(l,S) LOOP(m,S)
#define LOOP6(i,j,k,l,m,n,S) LOOP(i,S) LOOP(j,S) LOOP(k,S) LOOP(l,S) LOOP(m,S) \
                             LOOP(n,S)
#define LOOP7(i,j,k,l,m,n,\
                    o,S)     LOOP(i,S) LOOP(j,S) LOOP(k,S) LOOP(l,S) LOOP(m,S) \
                             LOOP(n,S) LOOP(o,S)
#define LOOP8(i,j,k,l,m,n,\
                    o,p,S)   LOOP(i,S) LOOP(j,S) LOOP(k,S) LOOP(l,S) LOOP(m,S) \
                             LOOP(n,S) LOOP(o,S) LOOP(p,S)

/*******************************************************************************
    constants
*******************************************************************************/
#define THR_ID_HDR      "THRD="      // thread ID header
#define ENC_FILENAME    "CRYFA_ENC" // encrypted file name
#define PKD_FILENAME    "CRYFA_PKD" // packed file name
#define ENW_FILENAME    "CRYFA_ENW" // encrypted without watermark file name
#define DEC_FILENAME    "CRYFA_DEC" // decrypted file name
#define UPK_FILENAME    "CRYFA_UPK" // unpacked file name
#define DEFAULT_N_THR   1           // default number of threads
#define LINE_BUFFER     600000//4//12//8//16//36000//6000//
                                    // buffer size (lines) to separate files for
                                    // multithreading. MUST be multiple of 4
#define LARGE_NUMBER    std::numeric_limits<std::streamsize>::max()
#define C1              2           //       cat 1  =  2
#define C2              3           //       cat 2  =  3
#define MIN_C3          4           //  4 <= cat 3 <=  6
#define MID_C3          5
#define MAX_C3          6
#define MIN_C4          7           //  7 <= cat 4 <= 15
#define MAX_C4          15
#define MIN_C5          16          // 16 <= cat 5 <= 39
#define MAX_C5          39
#define KEYLEN_C1       7           // 7 to 1 byte.  for building hash table
#define KEYLEN_C2       5           // 5 to 1 byte
#define KEYLEN_C3       3           // 3 to 1 byte
#define KEYLEN_C4       2           // 2 to 1 byte
#define KEYLEN_C5       3           // 3 to 2 byte

/*******************************************************************************
    lookup tables
*******************************************************************************/
const string DNA_UNPACK[] =     // 216 elements
{
    "AAA", "AAC", "AAG", "AAT", "AAN", "AAX", "ACA", "ACC", "ACG", "ACT", "ACN",
    "ACX", "AGA", "AGC", "AGG", "AGT", "AGN", "AGX", "ATA", "ATC", "ATG", "ATT",
    "ATN", "ATX", "ANA", "ANC", "ANG", "ANT", "ANN", "ANX", "AXA", "AXC", "AXG",
    "AXT", "AXN", "AXX", "CAA", "CAC", "CAG", "CAT", "CAN", "CAX", "CCA", "CCC",
    "CCG", "CCT", "CCN", "CCX", "CGA", "CGC", "CGG", "CGT", "CGN", "CGX", "CTA",
    "CTC", "CTG", "CTT", "CTN", "CTX", "CNA", "CNC", "CNG", "CNT", "CNN", "CNX",
    "CXA", "CXC", "CXG", "CXT", "CXN", "CXX", "GAA", "GAC", "GAG", "GAT", "GAN",
    "GAX", "GCA", "GCC", "GCG", "GCT", "GCN", "GCX", "GGA", "GGC", "GGG", "GGT",
    "GGN", "GGX", "GTA", "GTC", "GTG", "GTT", "GTN", "GTX", "GNA", "GNC", "GNG",
    "GNT", "GNN", "GNX", "GXA", "GXC", "GXG", "GXT", "GXN", "GXX", "TAA", "TAC",
    "TAG", "TAT", "TAN", "TAX", "TCA", "TCC", "TCG", "TCT", "TCN", "TCX", "TGA",
    "TGC", "TGG", "TGT", "TGN", "TGX", "TTA", "TTC", "TTG", "TTT", "TTN", "TTX",
    "TNA", "TNC", "TNG", "TNT", "TNN", "TNX", "TXA", "TXC", "TXG", "TXT", "TXN",
    "TXX", "NAA", "NAC", "NAG", "NAT", "NAN", "NAX", "NCA", "NCC", "NCG", "NCT",
    "NCN", "NCX", "NGA", "NGC", "NGG", "NGT", "NGN", "NGX", "NTA", "NTC", "NTG",
    "NTT", "NTN", "NTX", "NNA", "NNC", "NNG", "NNT", "NNN", "NNX", "NXA", "NXC",
    "NXG", "NXT", "NXN", "NXX", "XAA", "XAC", "XAG", "XAT", "XAN", "XAX", "XCA",
    "XCC", "XCG", "XCT", "XCN", "XCX", "XGA", "XGC", "XGG", "XGT", "XGN", "XGX",
    "XTA", "XTC", "XTG", "XTT", "XTN", "XTX", "XNA", "XNC", "XNG", "XNT", "XNN",
    "XNX", "XXA", "XXC", "XXG", "XXT", "XXN", "XXX"
};
//..............................................................................
const htable_t DNA_MAP =
{
    {"AAA",   0}, {"AAC",   1}, {"AAG",   2}, {"AAT",   3}, {"AAN",   4},
    {"AAX",   5}, {"ACA",   6}, {"ACC",   7}, {"ACG",   8}, {"ACT",   9},
    {"ACN",  10}, {"ACX",  11}, {"AGA",  12}, {"AGC",  13}, {"AGG",  14},
    {"AGT",  15}, {"AGN",  16}, {"AGX",  17}, {"ATA",  18}, {"ATC",  19},
    {"ATG",  20}, {"ATT",  21}, {"ATN",  22}, {"ATX",  23}, {"ANA",  24},
    {"ANC",  25}, {"ANG",  26}, {"ANT",  27}, {"ANN",  28}, {"ANX",  29},
    {"AXA",  30}, {"AXC",  31}, {"AXG",  32}, {"AXT",  33}, {"AXN",  34},
    {"AXX",  35}, {"CAA",  36}, {"CAC",  37}, {"CAG",  38}, {"CAT",  39},
    {"CAN",  40}, {"CAX",  41}, {"CCA",  42}, {"CCC",  43}, {"CCG",  44},
    {"CCT",  45}, {"CCN",  46}, {"CCX",  47}, {"CGA",  48}, {"CGC",  49},
    {"CGG",  50}, {"CGT",  51}, {"CGN",  52}, {"CGX",  53}, {"CTA",  54},
    {"CTC",  55}, {"CTG",  56}, {"CTT",  57}, {"CTN",  58}, {"CTX",  59},
    {"CNA",  60}, {"CNC",  61}, {"CNG",  62}, {"CNT",  63}, {"CNN",  64},
    {"CNX",  65}, {"CXA",  66}, {"CXC",  67}, {"CXG",  68}, {"CXT",  69},
    {"CXN",  70}, {"CXX",  71}, {"GAA",  72}, {"GAC",  73}, {"GAG",  74},
    {"GAT",  75}, {"GAN",  76}, {"GAX",  77}, {"GCA",  78}, {"GCC",  79},
    {"GCG",  80}, {"GCT",  81}, {"GCN",  82}, {"GCX",  83}, {"GGA",  84},
    {"GGC",  85}, {"GGG",  86}, {"GGT",  87}, {"GGN",  88}, {"GGX",  89},
    {"GTA",  90}, {"GTC",  91}, {"GTG",  92}, {"GTT",  93}, {"GTN",  94},
    {"GTX",  95}, {"GNA",  96}, {"GNC",  97}, {"GNG",  98}, {"GNT",  99},
    {"GNN", 100}, {"GNX", 101}, {"GXA", 102}, {"GXC", 103}, {"GXG", 104},
    {"GXT", 105}, {"GXN", 106}, {"GXX", 107}, {"TAA", 108}, {"TAC", 109},
    {"TAG", 110}, {"TAT", 111}, {"TAN", 112}, {"TAX", 113}, {"TCA", 114},
    {"TCC", 115}, {"TCG", 116}, {"TCT", 117}, {"TCN", 118}, {"TCX", 119},
    {"TGA", 120}, {"TGC", 121}, {"TGG", 122}, {"TGT", 123}, {"TGN", 124},
    {"TGX", 125}, {"TTA", 126}, {"TTC", 127}, {"TTG", 128}, {"TTT", 129},
    {"TTN", 130}, {"TTX", 131}, {"TNA", 132}, {"TNC", 133}, {"TNG", 134},
    {"TNT", 135}, {"TNN", 136}, {"TNX", 137}, {"TXA", 138}, {"TXC", 139},
    {"TXG", 140}, {"TXT", 141}, {"TXN", 142}, {"TXX", 143}, {"NAA", 144},
    {"NAC", 145}, {"NAG", 146}, {"NAT", 147}, {"NAN", 148}, {"NAX", 149},
    {"NCA", 150}, {"NCC", 151}, {"NCG", 152}, {"NCT", 153}, {"NCN", 154},
    {"NCX", 155}, {"NGA", 156}, {"NGC", 157}, {"NGG", 158}, {"NGT", 159},
    {"NGN", 160}, {"NGX", 161}, {"NTA", 162}, {"NTC", 163}, {"NTG", 164},
    {"NTT", 165}, {"NTN", 166}, {"NTX", 167}, {"NNA", 168}, {"NNC", 169},
    {"NNG", 170}, {"NNT", 171}, {"NNN", 172}, {"NNX", 173}, {"NXA", 174},
    {"NXC", 175}, {"NXG", 176}, {"NXT", 177}, {"NXN", 178}, {"NXX", 179},
    {"XAA", 180}, {"XAC", 181}, {"XAG", 182}, {"XAT", 183}, {"XAN", 184},
    {"XAX", 185}, {"XCA", 186}, {"XCC", 187}, {"XCG", 188}, {"XCT", 189},
    {"XCN", 190}, {"XCX", 191}, {"XGA", 192}, {"XGC", 193}, {"XGG", 194},
    {"XGT", 195}, {"XGN", 196}, {"XGX", 197}, {"XTA", 198}, {"XTC", 199},
    {"XTG", 200}, {"XTT", 201}, {"XTN", 202}, {"XTX", 203}, {"XNA", 204},
    {"XNC", 205}, {"XNG", 206}, {"XNT", 207}, {"XNN", 208}, {"XNX", 209},
    {"XXA", 210}, {"XXC", 211}, {"XXG", 212}, {"XXT", 213}, {"XXN", 214},
    {"XXX", 215}
};

/*******************************************************************************
    messages
*******************************************************************************/
inline void Help ()    // usage guide
{
    cout                                                              << '\n'
      << "Synopsis:"                                                  << '\n'
      << "    cryfa [OPTION]... -k [KEYFILENAME] [FILENAME]"          << '\n'
                                                                      << '\n'
      << "Options:"                                                   << '\n'
      << "    -h,  --help"                                            << '\n'
      << "         usage guide"                                       << '\n'
                                                                      << '\n'
      << "    -a,  --about"                                           << '\n'
      << "         about the program"                                 << '\n'
                                                                      << '\n'
      << "    -v,  --verbose"                                         << '\n'
      << "         verbose mode (more information)"                   << '\n'
                                                                      << '\n'
      << "    -s,  --disable_shuffle"                                 << '\n'
      << "         disable shuffling input. It should be used in both "
      <<                            "encryption and decryption sides" << '\n'
                                                                      << '\n'
      << "    -d,  --decrypt"                                         << '\n'
      << "         decrypt mode"                                      << '\n'
                                                                      << '\n'
      << "    -k [KEYFILE],  --key [KEYFILE]"                         << '\n'
      << "         key filename"                                      << '\n'
                                                                      << '\n'
      << "    -t [NUMBER],  --thread [NUMBER]"                        << '\n'
      << "         number of threads"                                 << '\n'
                                                                      << '\n';
}
//..............................................................................
inline void About ()   // About cryfa
{
    cout                                                              << '\n'
      << "cryfa v" << VERSION_CRYFA << "." << RELEASE_CRYFA           << '\n'
      << "================"                                           << '\n'
      << "A FASTA encryption and decryption tool"                     << '\n'
                                                                      << '\n'
      << "Diogo Pratas, Morteza Hosseini, Armando J. Pinho"           << '\n'
      << "Copyright (C) 2017, University of Aveiro"                   << '\n'
                                                                      << '\n'
      << "This is a Free software, under GPLv3. You may redistribute" << '\n'
      << "copies of it under the terms of the GNU - General Public"   << '\n'
      << "License v3 <http://www.gnu.org/licenses/gpl.html>. There"   << '\n'
      << "is NOT ANY WARRANTY, to the extent permitted by law."       << '\n'
                                                                      << '\n';
}

#endif //CRYFA_DEF_H
