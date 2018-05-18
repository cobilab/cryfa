<p align="center">
<img src="img/logo.png" alt="Cryfa" width="150" border="0" /></p>
<br>

Cryfa is a secure encryption tool for genomic data, and is also capable of 
compacting FASTA and FASTQ sequences. For the purpose of authenticated 
encryption, it employs AES (Advanced Encryption Standard) algorithm in GCM 
mode (Galois / counter mode) of operation. Cryfa is able to decrease the file 
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
./cryfa
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

      -h,  --help
           usage guide

      -k [KEY_FILE],  --key [KEY_FILE]
           key file name -- MANDATORY
           The KEY_FILE contains a password.
           You can employ the "keygen" program to make a strong 
           password, via the command "./keygen".

      -d,  --dec
           decrypt & unpack
           
      -f,  --format
           force specified format -- 'a':FASTA, 'q':FASTQ, 'n':others
           
      -v,  --verbose
           verbose mode (more information)

      -s,  --stop_shuffle
           stop shuffling the input

      -t [NUMBER],  --thread [NUMBER]
           number of threads
```
Cryfa uses standard ouput stream, hence, its output can be directly integrated
with pipelines.


#### Choose the password
Regarding `-k` or `--key` flags, there are two options: either saving a 
raw password in a file and passing the file name to the flags; or, 
employing the "keygen" program, which is provided to generate a strong password.
This second method is highly recommended.

To employ the first method, you can use the following commands to save an 
arbitrary raw password, in this case "Such a strong password!", into the 
"pass.txt" file. A text editor can also be used for this purpose.
Then, you are able to pass the key file to Cryfa:
```bash
echo Such a strong password! > pass.txt
./cryfa -k pass.txt in_file > out_file
```
Note, this raw password must include at least 8 characters.
Although, employing a raw password is not recommended, but if you tend to use 
it, it would be a better practice to choose a strong password.

A "strong password" is




With the second method, that is highly recommended, you are able to have a 
strong password. 



## CITE
Please cite the following, if you use Cryfa:
* D. Pratas, M. Hosseini and A.J. Pinho, "Cryfa: a tool to compact and encrypt
FASTA files," *11'th International Conference on Practical Applications of 
Computational Biology & Bioinformatics* (PACBB), Springer, June 2017.


## RELEASES
* [Release](https://github.com/pratas/cryfa/releases) 2: Secure encryption of FASTA, FASTQ, VCF, SAM and BAM PLUS compacting FASTA and FASTQ.
* [Release](https://github.com/pratas/cryfa/releases) 1: Encryption PLUS compacting FASTA.


## ISSUES
Please let us know if there is any 
[issues](https://github.com/pratas/cryfa/issues).


## LICENSE
Cryfa is under GPL v3 license. For more information, click 
[here](http://www.gnu.org/licenses/gpl-3.0.html).