#!/bin/bash

cd ..
cmake .
make

#in="in.fa"
in="HS1.fa"
#in="archaea.fa"
out="out.fa"

#./cryfa -vk pass.txt $in > ENCRYPTED
#./cryfa -vdk pass.txt ENCRYPTED > $out
./cryfa -k pass.txt $in > ENCRYPTED
./cryfa -dk pass.txt ENCRYPTED > $out

#echo "Comparing original and decrypted files..."
cmp $in $out

cd scripts