cmake_minimum_required(VERSION 3.3.2)

project(cryfa)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

include_directories(${CMAKE_SOURCE_DIR}/src/include)

find_package(Threads REQUIRED)

file(GLOB CRYPTOPP_SOURCE "src/cryptopp/*.cpp")
add_library(libCryptopp OBJECT 
    ${CRYPTOPP_SOURCE}
)

add_library(libCryfaCommon OBJECT
    src/endecrypto.cpp
    src/fasta.cpp
    src/fastq.cpp
    src/security.cpp
)

add_executable(cryfa
    src/cryfa.cpp
)

target_link_libraries(cryfa
    Threads::Threads
    libCryfaCommon
    libCryptopp
)

add_executable(keygen
    src/keygen.cpp
)