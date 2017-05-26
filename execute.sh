#!/bin/bash

cmake .
make

#in="in.fa"
in=$1
out="out"

./cryfa -t1 -k pass.txt $in > ENCRYPTED
#./cryfa -dk pass.txt ENCRYPTED > $out
#./cryfa -dk pass.txt CRYFA_OUT0 > $out
#
#cmp $in $out