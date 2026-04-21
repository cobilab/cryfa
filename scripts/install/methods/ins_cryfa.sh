#!/usr/bin/env bash

# Install Cryfa

rm -f cryfa

cmake .
make

if [[ ! -d $progs/cryfa ]]; then
  mkdir -p $progs/cryfa
fi
cp cryfa pass.txt $progs/cryfa/
