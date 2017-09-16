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
    find file type: FASTA (A), FASTQ (Q), none (n)
*******************************************************************************/
inline char fileType (const string& inFileName)
{
    char     c;
    ifstream in(inFileName);
    
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    // FASTQ
    if (in.peek() == '@')
    {
        in.ignore(LARGE_NUMBER, '\n');                 // ignore the first line
        in.ignore(LARGE_NUMBER, '\n');                 // ignore the second line
        if (in.peek() == '+') { in.close();    return 'Q'; }
    }
    
    // FASTA
    while (in.peek()==' ' || in.peek()=='\n')    in.get(c);       // skip spaces
    if (in.peek() == '>') { in.close();    return 'A'; }
    
    // neither FASTA nor FASTQ
    in.close();
    return 'n';
}

#endif //CRYFA_FCN_H