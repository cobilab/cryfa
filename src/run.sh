#!/bin/bash
g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
in="in.fa"
#in="example.fa"
out="out.fa"
./cryfa -vk pass.txt $in > ENCRYPTED
#./cryfa -vdk pass.txt ENCRYPTED
./cryfa -vdk pass.txt ENCRYPTED > $out
cmp $in $out

