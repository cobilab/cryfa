#!/bin/bash

cmake .
make

#in="in.fa"
in=$1
out="OUT"

./cryfa -t2 -k pass.txt $in > ENCRYPTED
#./cryfa -dk pass.txt ENCRYPTED > $out
#
#cmp $in $out