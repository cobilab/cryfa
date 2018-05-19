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
           The KEY_FILE would contain a password.
           To make a strong password, the "keygen" program can be
           employed via the command "./keygen".

      -d,  --dec
           decrypt & unpack
           
      -f,  --force
           force to consider input as non-FASTA/FASTQ
           Forces Cryfa not to compress, but shuffle and encrypt.
           If the input is FASTA/FASTQ, it is again considered as
           non-FASTA/FASTQ, therefore, compression will be ignored,
           but shuffling and encryption will be performed.
           
      -v,  --verbose
           verbose mode (more information)

      -s,  --stop_shuffle
           stop shuffling the input

      -t [NUMBER],  --thread [NUMBER]
           number of threads
```
Cryfa uses standard ouput stream, hence, its output can be directly integrated
with pipelines.

#### Make a key file
To make a "KEY_FILE", in order to pass to the `-k` or `--key` flags, there are
two methods: either saving a raw password in a file; or, employing the "keygen"
program, which is provided to generate a strong password. The second method is
highly recommended.

To apply the first method, the following commands can be used to save a raw 
password in a file, then passing it to Cryfa. In this case, 
"Such a strong password!" is our raw password and "pass.txt" is the file we save
our password in it. For the purpose of saving a password in a file, a text 
editor can also be used:
```bash
echo Such a strong password! > pass.txt
./cryfa -k pass.txt in_file > out_file
```
Note, the password must include at least 8 characters.
Although, employing this method is not recommended, but if you tend to use 
it, it would be a better practice to choose a "strong password".

A strong password:
* Has at least 12 characters
* Includes lowercase letters (a-z), uppercase letters (A-Z), digits (0-9) and
symbols (e.g. !, #, $, % and })
* Is not a character repetition (e.g. zzzzzz), keyboard pattern (e.g. qwerty)
or digit sequences (e.g. 123456)

The second method to make a key file is using the "keygen" program, which 
automatically provides with a strong password. Runnig
```bash
./keygen
```
the following message appears:
```
Enter a password, then press 'Enter':
```
After typing a raw password, e.g. `A keygen raw pass!`, and pressing "Enter",
the following message appears:
```
Enter a file name to save the generated key, then press 'Enter':
```
The automatically generated strong password will be saved in the file that you
specify its name, e.g. `key.txt`, in this step. Note, the "keygen" program needs
an initial raw password, which is not required to be strong itself, to generate
a strong password. Afterward, you can use the following command to pass the 
key file, in this case "key.txt", to Cryfa:
```bash
./cryfa -k key.txt in_file > out_file
```

If you are interested in the topic of "key management", which is to deal with
generating, exchanging, storing, using and replacing keys, you can read the 
articles at [this](https://en.wikipedia.org/wiki/Key_management),
[this](https://info.townsendsecurity.com/definitive-guide-to-encryption-key-management-fundamentals),
[this](https://csrc.nist.gov/projects/key-management/cryptographic-key-management-systems)
and
[this](https://www.cryptomathic.com/news-events/blog/what-is-key-management-a-ciso-perspective)
links.


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