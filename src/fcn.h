/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Functions
    - - - - - - - - - - - - - - - - - - -
    Morteza Hosseini    seyedmorteza@ua.pt
    Diogo Pratas        pratas@ua.pt
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
inline char fileType (const string& inFileName)
{
    char c;
    ifstream in(inFileName);
    
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    in.get(c);    in.close();
    return (c=='@' ? 'Q' : 'A');
}

#endif //CRYFA_FCN_H
