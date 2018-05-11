cmake_minimum_required(VERSION 3.3.2)
#project(cryfa)
#SET(CMAKE_BUILD_TYPE Release)  #todo. uncomment for final release

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

#file(GLOB SOURCE_FILES src/cryfa.cpp src/endecrypto.cpp src/fasta.cpp
#                       src/fastq.cpp src/security.cpp src/def.hpp src/fn.hpp
#                       "src/cryptopp/*.cpp")
#
#add_executable(cryfa ${SOURCE_FILES})

add_executable(keygen  src/keygen.cpp)