#!/bin/bash

#todo. remove this
rm CRYFA_ENC*


cmake .
make

#in="in.fa"
in=$1
out="CRYFA_OUT"

./cryfa -t8 -k pass.txt $in > CRYFA_ENCRYPTED
./cryfa -dk pass.txt CRYFA_ENCRYPTED > $out
#
#cmp $in $out