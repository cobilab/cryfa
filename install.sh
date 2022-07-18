#!/bin/bash

# Parameters
BUILD_TYPE=Release
BUILD=build
SOURCE=src
PARALLEL=8

# Configure CMake
cmake -B $BUILD -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
cmake --build $BUILD --parallel $PARALLEL --config $BUILD_TYPE

# Move executables to the main directory
mv $BUILD/cryfa .
mv $BUILD/keygen .
