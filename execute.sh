#!/bin/bash

cmake .
make

in=$1
comFile="CRYFA_COMPRESSED"
decomFile="CRYFA_DECOMPRESSED"
n=8
#for n in {1..8}; do # i in {1..10}; do
./cryfa -t $n -k pass.txt $in > $comFile            # -s to disable shuffling
./cryfa -t $n -dk pass.txt $comFile > $decomFile    # -s to disable shuffling
#cmp $in $decomFile
#done