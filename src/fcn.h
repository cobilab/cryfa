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
inline char fileType (const string& inFileName)
{
    ifstream in(inFileName);
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    // FASTQ
    string l;   // each line
    while (getline(in, l).good())   if (l[0]=='@') { in.close();   return 'Q'; }
    
    // FASTA
    in.close();   return 'A';
}

#endif //CRYFA_FCN_H
