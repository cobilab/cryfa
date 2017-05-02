cmake_minimum_required(VERSION 2.8.0)
project(cryfa)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -I src/cryptopp -pthread")

set(SOURCE_FILES src/cryfa.cpp src/EnDecrypto.cpp src/cryptopp )
add_executable(cryfa src/cryptopp ${SOURCE_FILES})
