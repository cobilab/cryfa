/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Functions
    - - - - - - - - - - - - - - - - - - -
    Diogo Pratas        pratas@ua.pt
    Morteza Hosseini    seyedmorteza@ua.pt
    Armando J. Pinho    ap@ua.pt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef CRYFA_FCN_H
#define CRYFA_FCN_H

#include <fstream>
using std::ifstream;
using std::cerr;

/*******************************************************************************
    find file type: FASTA (A) or FASTQ (Q)
*******************************************************************************/
inline char fileType (const string &inFileName)
{
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    string line;
    
    // FASTQ
    while (getline(in, line).good())
    {
        if (line[0] == '@')
        {
            in.clear();
            in.seekg(0, std::ios::beg);     // go to the beginning of file
            return 'Q';
        }
    }
    
    // FASTA
    in.clear();  in.seekg(0, std::ios::beg);
    return 'A';
}

/*******************************************************************************

*******************************************************************************/
//inline void splitNFingRange(const std::ifstream &in)
//{
// std::cerr<<"hi";
//}

#endif //CRYFA_FCN_H
