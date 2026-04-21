#!/usr/bin/env bash

# Install DSRC -- FASTQ

rm -fr dsrc/

git clone https://github.com/lrog/dsrc.git
mv dsrc/ $progs/

cd $progs/dsrc/
make
cp bin/dsrc .

cd ../..
