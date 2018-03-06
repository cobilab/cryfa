<p align="center">
<img src="img/logo.png" alt="Cryfa" width="150" border="0" /></p>
<br>

Cryfa is a secure encryption tool for genomic data, and is also capable of 
compacting FASTA and FASTQ sequences. For the purpose of authenticated 
encryption, it employs AES (Advanced Encryption Standard) algorithm in GCM 
mode (Galois/counter mode) of operation. Cryfa is able to decrease the file 
sizes by a factor of 3, without creating security problems such as those 
derived from CRIME or BREACH attacks.


## INSTALL
Get Cryfa and make the project, using:
```bash
git clone https://github.com/pratas/cryfa.git
cd cryfa
cmake .
make
```
Note, an already compiled version of Cryfa is available for 64 bit Linux OS
in the `bin/` directory.


## RUN
If you want to run Cryfa in stand-alone mode, use the following command:
```bash
./cryfa [OPTION]... -k [KEY_FILE] [-d] [IN_FILE] > [OUT_FILE]
```
For example, to compact & encrypt:
```bash
./cryfa -k pass.txt in.fq > comp
```
and, to decrypt & unpack:
```bash
./cryfa -k pass.txt -d comp > orig.fq
```
Options are described in the following section.

If you want to compare Cryfa with other methods, set the parameters in 
**run.sh** bash script, then run it:
```bash
./run.sh
```
With this script, you can download the datasets, install the dependencies, 
install the compression and encryption tools, run these tools, and finally,
print the results.


### OPTIONS
To see the possible options, type:
```bash
./cryfa -h
```

which provides the following:
```
SYNOPSIS
      ./cryfa [OPTION]... -k [KEY_FILE] [-d] [IN_FILE] > [OUT_FILE]

SAMPLE
      Encrypt and compact:   ./cryfa -k pass.txt in.fq > comp     
      Decrypt and unpack:    ./cryfa -k pass.txt -d comp > orig.fq
      
      Encrypt:               ./cryfa -k pass.txt in > enc
      Decrypt:               ./cryfa -k pass.txt -d enc > orig

DESCRIPTION
      Compact & encrypt FASTA/FASTQ files.
      Encrypt any text-based genomic data.

      The KEY_FILE specifies a file including the password.

      -h,  --help
           usage guide

      -k [KEY_FILE],  --key [KEY_FILE]
           key file name -- MANDATORY

      -d,  --dec
           decrypt & unpack

      -v,  --verbose
           verbose mode (more information)

      -s,  --disable_shuffle
           disable input shuffling

      -t [NUMBER],  --thread [NUMBER]
           number of threads
```

Cryfa uses standard input and ouput streams, hence, it can be directly 
integrated with pipelines.


## CITE
Please cite the following, if you use Cryfa:
* D. Pratas, M. Hosseini and A.J. Pinho, "Cryfa: a tool to compact and encrypt
FASTA files," *11'th International Conference on Practical Applications of 
Computational Biology & Bioinformatics* (PACBB), Springer, June 2017.


## RELEASES
* [Release](https://github.com/pratas/cryfa/releases) 2: FASTA and FASTQ 
handling.
* [Release](https://github.com/pratas/cryfa/releases) 1: FASTA handling.


## ISSUES
Please let us know if there is any 
[issues](https://github.com/pratas/cryfa/issues).


## LICENSE
Cryfa is under GPL v3 license. For more information, click 
[here](http://www.gnu.org/licenses/gpl-3.0.html).