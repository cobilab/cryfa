cmake_minimum_required(VERSION 4.0.0)

project(cryfa)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

# ── Cryptopp ──────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    cryptopp
    GIT_REPOSITORY https://github.com/weidai11/cryptopp.git
    GIT_TAG        CRYPTOPP_8_9_0
    GIT_SHALLOW    TRUE
    DOWNLOAD_ONLY  YES
)
FetchContent_MakeAvailable(cryptopp)

# Mirror the installed-layout: copy headers to <build>/cryptopp_include/cryptopp/
# so that #include "cryptopp/aes.h" resolves correctly without relying on installation.
set(CRYPTOPP_INCLUDE_DIR "${CMAKE_BINARY_DIR}/cryptopp_include")
file(GLOB CRYPTOPP_HEADERS "${cryptopp_SOURCE_DIR}/*.h")
file(COPY ${CRYPTOPP_HEADERS} DESTINATION "${CRYPTOPP_INCLUDE_DIR}/cryptopp")

# Build cryptopp as a static library from upstream sources
file(GLOB CRYPTOPP_SOURCES "${cryptopp_SOURCE_DIR}/*.cpp")
add_library(cryptopp-static STATIC ${CRYPTOPP_SOURCES})
target_include_directories(cryptopp-static
    PRIVATE "${cryptopp_SOURCE_DIR}"   # internal: #include "aes.h"
    PUBLIC  "${CRYPTOPP_INCLUDE_DIR}"  # external: #include "cryptopp/aes.h"
)
set_target_properties(cryptopp-static PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# On GCC/Clang (x86/x86_64), cryptopp's SIMD source files need explicit
# ISA flags that its own GNUmakefile supplies per-file. Add them to the
# whole target; on non-x86 (ARM/Apple Silicon) these flags don't exist and
# the check_cxx_compiler_flag guard keeps things safe.
if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-msse4.1" COMPILER_SUPPORTS_SSE41)
    if(COMPILER_SUPPORTS_SSE41)
        target_compile_options(cryptopp-static PRIVATE
            -msse4.1 -mssse3 -maes -mpclmul)
    endif()
endif()

find_package(Threads REQUIRED)

add_library(libCryfaCommon OBJECT
    src/application.cpp
    src/endecrypto.cpp
    src/fasta.cpp
    src/fastq.cpp
    src/security.cpp
)
target_include_directories(libCryfaCommon PRIVATE src/include)
target_link_libraries(libCryfaCommon PRIVATE cryptopp-static)

add_executable(cryfa
    src/cryfa.cpp
)
target_include_directories(cryfa PRIVATE src/include)
target_link_libraries(cryfa PRIVATE
    Threads::Threads
    libCryfaCommon
    cryptopp-static
)

add_executable(keygen
    src/keygen.cpp
)
target_include_directories(keygen PRIVATE src/include)