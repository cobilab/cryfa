#!/bin/bash

#todo. remove this
rm CRYFA_ENC*
rm CRYFA_UPK*

cmake .
make

#in="in.fa"
in=$1
out="CRYFA_OUT"

#for i in {1..10}; do
./cryfa -t8 -k pass.txt $in > CRYFA_ENCRYPTED
./cryfa -t2 -dk pass.txt CRYFA_ENCRYPTED > $out
cmp $in $out
#done
#./cryfa -t8 -sk pass.txt $in > CRYFA_ENCRYPTED
#./cryfa -dsk pass.txt CRYFA_ENCRYPTED > $out
#cmp $in $out