#!/bin/bash

cmake .
make

in="in.fa"
#in="example.fa"
out="out.fa"

./cryfa -vk pass.txt $in > ENCRYPTED
./cryfa -vdk pass.txt ENCRYPTED > $out
cmp $in $out