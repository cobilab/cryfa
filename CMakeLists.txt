cmake_minimum_required(VERSION 2.8.12)
project(cryfa)
SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB SOURCE_FILES "src/*.h" "src/*.cpp" "src/cryptopp/*.cpp")
#file(GLOB SOURCE_FILES "src/cryfa.cpp" "src/EnDecrypto.cpp" "src/FASTA.cpp" "src/cryptopp/*.cpp")
add_executable(cryfa ${SOURCE_FILES})