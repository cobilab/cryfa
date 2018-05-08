cmake_minimum_required(VERSION 3.3.2)
#project(cryfa)
#SET(CMAKE_BUILD_TYPE Release)  #todo. uncomment for final release

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB SOURCE_FILES src/cryfa.cpp src/EnDecrypto.cpp src/FASTA.cpp
                       src/FASTQ.cpp src/Security.cpp src/def.hpp
                       "src/cryptopp/*.cpp")

add_executable(cryfa ${SOURCE_FILES})