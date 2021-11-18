<img src="img/logo.png" alt="Cryfa" height="125" border="0"/> &emsp;&emsp; [![Build Status](https://travis-ci.org/cobilab/cryfa.svg?branch=master)](https://travis-ci.org/cobilab/cryfa)
![Conda](https://img.shields.io/conda/dn/bioconda/cryfa)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)

Cryfa is an ultrafast secure encryption tool for genomic data, that is also able to compact FASTA/FASTQ sequences by a factor of three.

## INSTALL
### Conda
```bash
conda install -c bioconda -y cryfa
```

### Linux
Install "git" and "cmake":
```bash
sudo apt update
sudo apt install git cmake
```

Clone Cryfa and make the project:
```bash
git clone https://github.com/cobilab/cryfa.git
cd cryfa
sh install.sh
```

### macOS
Install "Homebrew", "git" and "cmake":
```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install git cmake
```

Clone Cryfa and make the project:
```bash
git clone https://github.com/cobilab/cryfa.git
cd cryfa
sh install.sh
```

Note that a pre-compiled version of Cryfa is available for 64 bit Linux OS and macOS in
the `bin/` directory.

## USAGE
If you want to run Cryfa in stand-alone mode, use the following command:
```bash
./cryfa [OPTION]... -k [KEY_FILE] [-d] [IN_FILE] > [OUT_FILE]
```
For example, to compact & encrypt, run
```bash
./cryfa -k pass.txt in.fq > comp
```
and to decrypt & unpack, run
```bash
./cryfa -k pass.txt -d comp > orig.fq
```
There is a copy of file "in.fq" in `example/` directory. Options are described in the following sections.

Note that the maximum file size supported by Cryfa is 64 GB. For larger files, you can split them, e.g. by "split" command in Linux, and encrypt each chunk. After the decryption, you can concatenate the chunks, e.g. by "cat" command.

### Input file format
Cryfa automatically detects a genomic data file format by looking inside the
file and not by the file extension. For example, a FASTA file, say “test”, can
be fed into Cryfa as "test", "test.fa", "test.fasta", "test.fas", "test.fsa" or
any other file extension. By this explanation, running
```bash
./cryfa -k pass.txt test > comp
```
will be exactly the same as running
```bash
./cryfa -k pass.txt test.fa > comp
```

Note that password file is not limited to any extension, therefore, it can have either no extension or any extension. For example, using "pass", "pass.txt", "pass.dat", etc provides the same result.

### Compare Cryfa with other methods
If you want to compare Cryfa with other methods, set the parameters in 
**run.sh** bash script, then run it:
```bash
./run.sh
```
With this script, you can download the datasets, install the dependencies, 
install the compression and encryption tools, run these tools, and finally,
print the results.

## OPTIONS
To see the possible options, type:
```bash
./cryfa
```

which provides the following:
```text
SYNOPSIS
      ./cryfa [OPTION]... -k [KEY_FILE] [-d] [IN_FILE] > [OUT_FILE]

SAMPLE
      Encrypt and compact:  ./cryfa -k pass.txt in.fq > comp     
      Decrypt and unpack:   ./cryfa -k pass.txt -d comp > orig.fq
      
      Encrypt:              ./cryfa -k pass.txt in > enc
      Decrypt:              ./cryfa -k pass.txt -d enc > orig

OPTIONS
      Compact & encrypt FASTA/FASTQ files.
      Encrypt any text-based genomic data, e.g., VCF/SAM/BAM.

      -k [KEY_FILE],  --key [KEY_FILE]
           key file name -- MANDATORY
           The KEY_FILE should contain a password.
           To make a strong password, the "keygen" program can be
           used via the command "./keygen".

      -d,  --dec
           decrypt & unpack
           
      -f,  --force
           force to consider input as non-FASTA/FASTQ
           Forces Cryfa not to compact, but shuffle and encrypt.
           If the input is FASTA/FASTQ, it is considered as
           non-FASTA/FASTQ; so, compaction will be ignored, but 
           shuffling and encryption will be performed.
           
      -s,  --stop_shuffle
           stop shuffling the input

      -t [NUMBER],  --thread [NUMBER]
           number of threads

      -v,  --verbose
           verbose mode (more information)
      
      -h,  --help
           usage guide

      --version
           version information
```
Cryfa uses standard ouput stream, hence, its output can be directly integrated
with pipelines.

### Make a key file
There are two methods to make a "KEY_FILE", in order to pass to the `-k` or
`--key` flags, either saving a raw password in a file, or, employing the
"keygen" program, which is provided to generate a strong password. The second
method is highly recommended.

To apply the first method, the following commands can be used to save a raw 
password in a file, then passing it to Cryfa. In this case, 
"Such a strong password!" is our raw password and "pass.txt" is the file where 
we save our password. For the purpose of saving a password in a file, a text 
editor can also be used:
```bash
echo "Such a strong password!" > pass.txt
./cryfa -k pass.txt IN_FILE > OUT_FILE
```
Note, the password must include at least 8 characters. Although, employing this
method is not recommended, but if you tend to use it, it would be a better
practice to choose a "strong password".

A strong password:
* has at least 12 characters;

* includes lowercase letters (a-z), uppercase letters (A-Z), digits (0-9) and
  symbols (e.g. !, #, $, % and });
  
* is not a character repetition (e.g. zzzzzz), keyboard pattern (e.g. qwerty) or
  digit sequences (e.g. 123456).

The second method to make a key file is using the "keygen" program, which 
automatically provides a strong password. Running
```bash
./keygen
```
the following message appears:
```text
Enter a password, then press 'Enter':
```
After typing a raw password, e.g. "A keygen raw pass!", and pressing "Enter",
the following message appears:
```text
Enter a file name to save the generated key, then press 'Enter':
```
The automatically generated strong password will be saved in the file that you
specify its name, e.g. "key.txt", in this step. Note, the "keygen" program needs
an initial raw password, which is not required to be strong itself, to generate
a strong password. Afterward, you can use the following command to pass the key
file, in this case "key.txt", to Cryfa:
```bash
./cryfa -k key.txt IN_FILE > OUT_FILE
```

If you are interested in the topic of "key management", which is to deal with
generating, exchanging, storing, using and replacing keys, you can read the 
articles [[1]](https://en.wikipedia.org/wiki/Key_management),
[[2]](https://info.townsendsecurity.com/definitive-guide-to-encryption-key-management-fundamentals),
[[3]](https://csrc.nist.gov/projects/key-management/cryptographic-key-management-systems)
and
[[4]](https://www.cryptomathic.com/news-events/blog/what-is-key-management-a-ciso-perspective).

## CITE
Please cite the followings, if you use Cryfa:
* M. Hosseini, D. Pratas and A.J. Pinho, "Cryfa: a secure encryption tool for genomic data," *Bioinformatics*, vol. 35, no. 1, pp. 146--148, 2018. [DOI: 10.1093/bioinformatics/bty645](https://doi.org/10.1093/bioinformatics/bty645)

* D. Pratas, M. Hosseini and A.J. Pinho, "Cryfa: a tool to compact and encrypt FASTA files," *11'th International Conference on Practical Applications of Computational Biology & Bioinformatics* (PACBB), Springer, June 2017. [DOI: 10.1007/978-3-319-60816-7_37](https://doi.org/10.1007/978-3-319-60816-7_37)

<!-- ## RELEASES
* [Release](https://github.com/pratas/cryfa/releases) 2: Secure encryption of
  FASTA/FASTQ/VCF/SAM/BAM PLUS compacting FASTA/FASTQ.

* [Release](https://github.com/pratas/cryfa/releases) 1: Encryption PLUS 
  compacting FASTA. -->

## LICENSE
Cryfa is under [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html) license.
