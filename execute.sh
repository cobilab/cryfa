#!/bin/bash

rm CRYFA_ENC*

cmake .
make

#in="in.fa"
in=$1
out="CRYFA_OUT"
n=8
#for n in {1..8}; do # i in {1..10}; do
./cryfa -t $n -k pass.txt $in > CRYFA_ENCRYPTED
#cmp $in CRYFA_PKD
./cryfa -t $n -dk pass.txt CRYFA_ENCRYPTED > $out
cmp $in $out
#done
#./cryfa -t8 -sk pass.txt $in > CRYFA_ENCRYPTED
#./cryfa -dsk pass.txt CRYFA_ENCRYPTED > $out
#cmp $in $out
