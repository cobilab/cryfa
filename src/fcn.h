/**
 * @file      fcn.h
 * @brief     Functions
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#ifndef CRYFA_FCN_H
#define CRYFA_FCN_H

#include <fstream>
using std::ifstream;
using std::cerr;

/**
 * @brief  Find file type: FASTA (A), FASTQ (Q), none (n)
 * @param  inFileName  Input file name
 * @return A, Q or n
 */
inline char fileType (const string& inFileName)
{
    char     c;
    ifstream in(inFileName);
    
    if (!in.good())
    { cerr << "Error: failed opening '" << inFileName << "'.\n";    exit(1); }
    
    // Skip leading blank lines (0xA='\n') or spaces (0x20=' ')
    while (in.peek()==0xA || in.peek()==0x20)    in.get(c);
    
    
    
    // SAM/FASTQ
    while (in.peek() == 0x40)    IGNORE_THIS_LINE(in);        // 0x40='@'
    byte nTabs = 0;    while (in.get(c) && c!='\n')  if (c=='\t') ++nTabs;

    if (nTabs >= 0xA)           { in.close();   return 'S'; } // SAM:   0xA =10
    else if (in.peek() == 0x2B) { in.close();   return 'Q'; } // FASTQ: 0x2B='+'
    
    // FASTA/Not valid
    in.clear();   in.seekg(0, std::ios::beg); // Return to beginning of the file

    while (in.peek()!=0x3E && in.peek()!=EOF)   IGNORE_THIS_LINE(in); //0x3E='>'
    if (in.get(c) && c==0x3E)   { in.close();   return 'A'; } // FASTA
    else                        { in.close();   return 'n'; } // Not valid
    
    
    
    // FASTQ
    if (in.peek() == '@')
    {
        IGNORE_THIS_LINE(in);                          // Ignore the first line
        IGNORE_THIS_LINE(in);                          // Ignore the second line
        if (in.peek() == '+') { in.close();    return 'Q'; }
    }

    // FASTA
    if (in.peek() == '>') { in.close();    return 'A'; }
    
    // Neither FASTA nor FASTQ
    in.close();
    return 'n';
}

/**
 * @brief  Check password taken from a file
 * @param  keyFileName  Name of the file containing the password
 * @param  k_flag       If '-k' is written in the command to run cryfa
 */
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
            // Extract the password
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