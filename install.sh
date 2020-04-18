#!/bin/bash

mkdir -p build
cd build
rm CMakeCache.txt
cmake ..
make -j4
mv cryfa ..
mv keygen ..
cd ..
