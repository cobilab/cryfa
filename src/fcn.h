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

/*******************************************************************************
    check password taken from a file
*******************************************************************************/
inline void checkPass (const string& keyFileName, const bool k_flag)
{
    if (!k_flag) { cerr<< "Error: no password file has been set.\n";  exit(1); }
    else
    {
        ifstream in(keyFileName);
        
        if (in.peek() == EOF)
        {
            cerr << "Error: password file is empty.\n";
            in.close();
            exit(1);
        }
        else if (!in.good())
        {
            cerr << "Error opening \"" << keyFileName << "\".\n";
            in.close();
            exit(1);
        }
        else
        {
            // extract the password
            char c;
            string pass;
            pass.clear();
            while (in.get(c))    pass += c;
    
            if (pass.size() < 8)
            {
                cerr << "Error: password size is " << pass.size()
                     << ". It must be at least 8.\n";
                in.close();
                exit(1);
            }
    
            in.close();
        }
    }
}

#endif //CRYFA_FCN_H