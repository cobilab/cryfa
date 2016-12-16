<b>Cryfa: a FASTA encryption and decryption tool.</b>

Cryfa uses AES symmetric encryption.


## INSTALLATION

First you need to get cryfa using:
<pre>
git clone https://github.com/pratas/cryfa.git
cd cryfa/src/
</pre>
Then, you need to install CryptoPP using:
<pre>
git clone https://github.com/weidai11/cryptopp
cd cryptopp/
make
cd ..
</pre>
Finally
<pre>
g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
</pre>

## PARAMETERS

To see the possible options type
<pre>
./cryfa -h
</pre>
These will print the following options:
<pre>
<p>

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
         decrypt mode

    -k [KEYFILE],  --key [KEYFILE]
         key filename
</p>
</pre>

## CITATION ##

On using this software/method, please cite: https://github.com/pratas/cryfa.

## ISSUES ##

For any issue let us know at [issues link](https://github.com/pratas/cryfa/issues).

## LICENSE

GPL v3.

For more information:
<pre>http://www.gnu.org/licenses/gpl-3.0.html</pre>

