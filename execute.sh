#!/bin/bash

cmake .
make

in="in.fa"
out="out.fa"

#./cryfa -vk pass.txt $in
./cryfa -vk pass.txt $in > ENCRYPTED
./cryfa -vdk pass.txt ENCRYPTED > $out
#cmp $in $out