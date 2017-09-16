<p align="center"><img src="imgs/logo.png" 
alt="Cryfa" width="150" border="0" /></p>
<br>

Cryfa is a FASTA/FASTQ packing plus encryption tool.
It uses AES (Advanced Encryption Standard) for the purpose of encryption.
Cryfa can be applied to any FASTA or FASTQ file (DNA sequences, headers and quality-scores).
It compacts 3 DNA bases into 1 char, using a fixed block size packing.
When compared with general compression tools, it allows to decrease the file size by a factor of 3, without creating security problems such as those derived from CRIME or BREACH attacks.


## INSTALLATION
First you need to get cryfa using:
```bash
git clone https://github.com/pratas/cryfa.git
```
Then, you need to set the parameters in **run.sh**, which is an all-in-one bash script. This way, you specify if you want to download a specific dataset, install dependencies or run the program.

Finally, run the **run.sh** script using:
```bash
. run.sh INPUT_FILE_NAME
```

## PARAMETERS
To see the possible options, first, you should make cryfa executable, using:
```bash
cmake .
make
```
Then you can type
```bash
./cryfa -h
```

This will print the following options:
```bash
Synopsis:
    cryfa [OPTION]... -k [KEYFILENAME] [FILENAME]

Options:
    -h,  --help
         usage guide

    -v,  --verbose
         verbose mode (more information)

    -s,  --disable_shuffle
         disable shuffling input
             
    -d,  --decrypt
         decryption mode

    -k [KEYFILE],  --key [KEYFILE]
         key filename
         
    -t [NUMBER],  --thread [NUMBER]
         number of threads
         
    -a,  --about
         about the program
```
Cryfa uses standard input and ouput streams, hence, it can be directly integrated with pipelines.

## CITATION
Please cite the followings, if you use cryfa:
* D. Pratas, M. Hosseini and A.J. Pinho, "Cryfa: a tool to compact and encrypt FASTA files," *11'th International Conference on Practical Applications of Computational Biology & Bioinformatics* (PACBB), Springer, June 2017.

## RELEASES
https://github.com/pratas/cryfa/releases:

* Release 2: FASTA and FASTQ handling (in development).
* Release 1: FASTA handling.

## ISSUES
Please let us know if there is any [issues](https://github.com/pratas/cryfa/issues).

## LICENSE
GPL v3.

For more information, [click](http://www.gnu.org/licenses/gpl-3.0.html).