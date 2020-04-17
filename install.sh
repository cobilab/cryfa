#!/bin/bash

if [ ! -d build ]; then mkdir -p build; fi
cd build
rm CMakeCache.txt
cmake ..
make -j4
mv cryfa ..
mv keygen ..
cd ..
