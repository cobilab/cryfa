/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

           CRYFA :: A FASTA/FASTQ encryption/decryption tool
         -----------------------------------------------------
            Morteza Hosseini, Diogo Pratas, Armando J. Pinho
                    {seyedmorteza,pratas,ap}@ua.pt
             Copyright (C) 2017, IEETA, University of Aveiro
               
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


#include <iostream>
#include <getopt.h>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include "def.h"
#include "EnDecrypto.h"
#include "fcn.h"
using std::string;
using std::cout;
using std::cerr;
using std::chrono::high_resolution_clock;
using std::setprecision;

////////////////////////////////////////////////////////////////////////////////
///////////////////                 M A I N                 ////////////////////
////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
    // start timer
    high_resolution_clock::time_point startTime = high_resolution_clock::now();

    EnDecrypto cryptObj;
    cryptObj.inFileName = argv[argc-1];  // input file name
    cryptObj.n_threads = DEFAULT_N_THR;  // initialize number of threads

    static int h_flag, a_flag, v_flag, d_flag, s_flag;
    int c;                               // deal with getopt_long()
    int option_index;                    // option index stored by getopt_long()
    opterr = 0;  // force getopt_long() to remain silent when it finds a problem

    static struct option long_options[] =
    {
        {"help",            no_argument, &h_flag, (int) 'h'},   // help
        {"about",           no_argument, &a_flag, (int) 'a'},   // about
        {"verbose",         no_argument, &v_flag, (int) 'v'},   // verbose
        {"disable_shuffle", no_argument, &s_flag, (int) 's'},   // d (un)shuffle
        {"decrypt",         no_argument, &d_flag, (int) 'd'},   // decrypt mode
        {"key",       required_argument,       0,       'k'},   // key file
        {"thread",    required_argument,       0,       't'},   // #threads >= 1
        {0,                           0,       0,         0}
    };

    while (1)
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

            case 'h': h_flag = 1;    Help();                              break;
            case 'a': a_flag = 1;    About();                             break;
            case 'v': v_flag = 1;    cryptObj.verbose = true;             break;
            case 's': s_flag = 1;    cryptObj.disable_shuffle = true;     break;
            case 'd': d_flag = 1;                                         break;
            case 'k': cryptObj.keyFileName = (string) optarg;             break;
            case 't': cryptObj.n_threads = (byte) stoi((string) optarg);  break;

            default:
                cerr << "Option '" << (char) optopt << "' is invalid.\n"; break;
        }
    }

    if (v_flag)  cerr << "Verbose mode on.\n";
    if (d_flag)
    {
        cryptObj.decrypt();                                         // decrypt
        ifstream in(DEC_FILENAME);
        (in.peek() == (char) 127) ? cryptObj.decompressFA()         // FASTA
                                  : cryptObj.decompressFQ();        // FASTQ
        in.close();

        // stop timer
        high_resolution_clock::time_point finishTime =
                high_resolution_clock::now();
        // duration in seconds
        std::chrono::duration<double> elapsed = finishTime - startTime;
        cerr << "took " << std::fixed << setprecision(4) << elapsed.count()
             << " seconds.\n";

        return 0;
    }
    
    if (!h_flag && !a_flag)
    {
        char file_type = fileType(cryptObj.inFileName); //file type: FASTA/FASTQ
        
        // if input is neither FASTA nor FASTQ file
        if (file_type == 'n')
        {
            cerr << "Error: \"" << cryptObj.inFileName << '"'
                 << " is neither a FASTA nor a FASTQ file.\n";
            return 0;
        }
        
        cerr << "Encrypting...\n";
        (file_type == 'A') ? cryptObj.compressFA() : cryptObj.compressFQ();

        // stop timer
        high_resolution_clock::time_point finishTime =
                high_resolution_clock::now();
        // duration in seconds
        std::chrono::duration<double> elapsed = finishTime - startTime;
        cerr << "took " << std::fixed << setprecision(4) << elapsed.count()
             << " seconds.\n";
    }
    
    return 0;
}