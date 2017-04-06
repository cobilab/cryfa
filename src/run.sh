#!/bin/bash
g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
./cryfa -vk pass.txt $1 > ENCRYPTED
./cryfa -vdk pass.txt ENCRYPTED > $2
cmp $1 $2

