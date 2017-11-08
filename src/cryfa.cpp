/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <     CRYFA :: FASTA/FASTQ compaction plus encryption     >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <          Morteza Hosseini    seyedmorteza@ua.pt         >
          <          Diogo Pratas        pratas@ua.pt               >
          <          Armando J. Pinho    ap@ua.pt                   >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          <     Copyright (C) 2017, IEETA, University of Aveiro     >
          <<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @file      cryfa.cpp
 * @brief     Main
 * @author    Morteza Hosseini  (seyedmorteza@ua.pt)
 * @author    Diogo Pratas      (pratas@ua.pt)
 * @author    Armando J. Pinho  (ap@ua.pt)
 * @copyright The GNU General Public License v3.0
 */

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include "def.h"
#include "Security.h"
#include "EnDecrypto.h"
#include "FASTA.h"
#include "FASTQ.h"

using std::string;
using std::cout;
using std::cerr;
using std::ifstream;
using std::setprecision;
using std::chrono::high_resolution_clock;


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
    
    // Skip leading blank lines or spaces
    while (in.peek()=='\n' || in.peek()==' ')    in.get(c);
    
    
    
    //todo sam
    // SAM/FASTQ
    while (in.peek() == '@')     IGNORE_THIS_LINE(in);
    byte nTabs=0;    while (in.get(c) && c!='\n')  if (c=='\t') ++nTabs;

    if (nTabs > 9)             { in.close();    return 'S'; }       // SAM
    else if (in.peek() == '+') { in.close();    return 'Q'; }       // FASTQ

    // FASTA/Not valid
    in.clear();   in.seekg(0, std::ios::beg); // Return to beginning of the file
    while (in.peek()!='>' && in.peek()!=EOF)    IGNORE_THIS_LINE(in);

    if (in.peek() == '>')      { in.close();    return 'A'; }       // FASTA
    else                       { in.close();    return 'n'; }       // Not valid
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


// Instantiation of static variables in InArgs structure
bool   InArgs::VERBOSE         = false;
bool   InArgs::DISABLE_SHUFFLE = false;
byte   InArgs::N_THREADS       = DEFAULT_N_THR;
string InArgs::IN_FILE_NAME    = "";
string InArgs::KEY_FILE_NAME   = "";


/**
 * @brief Main function
 */
int main (int argc, char* argv[])
{
////auto       startTime = high_resolution_clock::now();          // Start timer
    InArgs     inArgsObj;
    Security   secObj;
    EnDecrypto cryptObj;
    FASTA      fastaObj;
    FASTQ      fastqObj;
    
    inArgsObj.IN_FILE_NAME = argv[argc-1];    // Input file name

    static int h_flag, a_flag, v_flag, d_flag, s_flag;
    bool       k_flag = false;
    int        c;                      // Deal with getopt_long()
    int        option_index;           // Option index stored by getopt_long()
    opterr = 0;  // Force getopt_long() to remain silent when it finds a problem

    static struct option long_options[] =
    {
        {"help",            no_argument, &h_flag, (int) 'h'},   // Help
        {"about",           no_argument, &a_flag, (int) 'a'},   // About
        {"verbose",         no_argument, &v_flag, (int) 'v'},   // Verbose
        {"disableShuffle",  no_argument, &s_flag, (int) 's'},   // D (un)shuffle
        {"decrypt",         no_argument, &d_flag, (int) 'd'},   // Decrypt mode
        {"key",       required_argument,       0,       'k'},   // Key file
        {"thread",    required_argument,       0,       't'},   // #threads >= 1
        {0,                           0,       0,         0}
    };

    while (true)
    {
        option_index = 0;
        if ((c = getopt_long(argc, argv, ":havsdk:t:",
                             long_options, &option_index)) == -1)         break;

        switch (c)
        {
            case 0:
                // If this option set a flag, do nothing else now.
                if (long_options[option_index].flag != 0)                 break;
                cout << "option '" << long_options[option_index].name << "'\n";
                if (optarg)    cout << " with arg " << optarg << '\n';
                break;

            case 'k':
                k_flag = true;
                inArgsObj.KEY_FILE_NAME = string(optarg);
                break;
                
            case 'h':  h_flag=1;    Help();                               break;
            case 'a':  a_flag=1;    About();                              break;
            case 'v':  v_flag=1;    inArgsObj.VERBOSE = true;             break;
            case 's':  s_flag=1;    inArgsObj.DISABLE_SHUFFLE = true;     break;
            case 'd':  d_flag=1;                                          break;
            case 't':  inArgsObj.N_THREADS = (byte) stoi(string(optarg)); break;

            default:
                cerr << "Option '" << (char) optopt << "' is invalid.\n"; break;
        }
    }

    // Check password file
    if (!h_flag && !a_flag)    checkPass(inArgsObj.KEY_FILE_NAME, k_flag);
    
    // Verbose mode
    if (v_flag)
        cerr << "Verbose mode on.\n";
    
    // Decrypt + Decompress
    if (d_flag)
    {
        cryptObj.decrypt();                                         // Decrypt

        ifstream in(DEC_FILENAME);
        cerr << "Decompressing...\n";
        switch (in.peek())
        {
            case (char) 127:    fastaObj.decompress();    break;    // FASTA
            case (char) 126:    fastqObj.decompress();    break;    // FASTQ
            default:                                      break;
        }
        in.close();
        
////    auto finishTime = high_resolution_clock::now();        // Stop timer
////    std::chrono::duration<double> elapsed =                // Duration (sec)
////            finishTime - startTime;
////
////    cerr << "took " << std::fixed << setprecision(4) << elapsed.count()
////         << " seconds.\n";

        return 0;
    }
    
    if (!h_flag && !a_flag)
    {//todo sam
        switch (fileType(inArgsObj.IN_FILE_NAME))
        {
            case 'A':  cerr<<"Compacting...\n";   fastaObj.compress();    break;
            case 'Q':  cerr<<"Compacting...\n";   fastqObj.compress();    break;
            case 'S':  cerr<<"Compacting...\n";   cerr<<"SAM";            break;
            case 'n':
            default :  cerr<<"Error: \"" << inArgsObj.IN_FILE_NAME << "\" "
                           <<"is not a valid FASTA or FASTQ file.\n";
                       return 0;                                          break;
        }

////    auto finishTime = high_resolution_clock::now();        // Stop timer
////    std::chrono::duration<double> elapsed =                // Duration (sec)
////            finishTime - startTime;
////
////    cerr << "took " << std::fixed << setprecision(4) << elapsed.count()
////         << " seconds.\n";
    }
    
    return 0;
}