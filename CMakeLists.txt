cmake_minimum_required(VERSION 3.3.2)
SET(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -O3")
file(GLOB SOURCE_FILES "src/cryptopp/*.cpp")

add_executable(cryfa   ${SOURCE_FILES}
                       src/assert.hpp
                       src/cryfa.cpp
                       src/def.hpp
                       src/endecrypto.cpp
                       src/fasta.cpp
                       src/fastq.cpp
                       src/fn.hpp
                       src/parser.hpp
                       src/security.cpp)

add_executable(keygen  src/keygen.cpp)