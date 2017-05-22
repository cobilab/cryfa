<p align="center"><img src="imgs/logo.png" 
alt="Cryfa" width="150" border="0" /></p>
<br>
<p>
Cryfa is a FASTA and FASTQ encryption and decryption tool.
Cryfa uses AES symmetric encryption, with very fast processing times. 
Cryfa can be applied to any FASTA or FASTQ file (DNA sequences, headers and quality-scores).
Cryfa compacts 3 DNA bases into 1 char, using a fixed block size packing. When compared with general encryption tools it allows to reduce the storage (~3x), without creating security problems such as those derived from CRIME or BREACH attacks.
</p>

## INSTALLATION
First you need to get cryfa using:
```bash
git clone https://github.com/pratas/cryfa.git
cd cryfa/src/
```
Then, you need to install CryptoPP using:
```bash
git clone https://github.com/weidai11/cryptopp
cd cryptopp/
make
cp libcryptopp.a ..
cd ..
```
Finally, compile with:
```bash
g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
```
or use
```bash
./execute.sh
```

## PARAMETERS
To see the possible options type
```bash
./cryfa -h
```
These will print the following options:
```bash
Synopsis:
    cryfa [OPTION]... -k [KEYFILENAME] [FILENAME]

Options:
    -h,  --help
         usage guide

    -a,  --about
         about the program

    -v,  --verbose
         verbose mode (more information)

    -d,  --decrypt
         decryption mode

    -k [KEYFILE],  --key [KEYFILE]
         key filename
         
    -t [NUMBER],  --thread [NUMBER]
         number of threads
```
Cryfa uses stdin and stdout and, hence, can be directly integrated on pipelines.

## CITATION
Please cite the followings, if you use <i>cryfa</i>:
* D. Pratas, M. Hosseini and A.J. Pinho, "Cryfa: a tool to compact and encrypt FASTA files," 11'th International Conference on Practical Applications of Computational Biology & Bioinformatics (PACBB), Springer, June 2017.

## RELEASES
https://github.com/pratas/cryfa/releases:

* Release 2: FASTA and FASTQ handling (being developed);
* Release 1: FASTA handling only;

## ISSUES
Please let us know if there is any [issues](https://github.com/pratas/cryfa/issues).

## LICENSE
GPL v3.

For more information:
<pre>http://www.gnu.org/licenses/gpl-3.0.html</pre>
