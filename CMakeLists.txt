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

# Build cryptopp as a static library from upstream sources.
# README.txt: cryptlib.cpp, cpu.cpp, integer.cpp MUST be first for correct
# static-initialization order.
file(GLOB CRYPTOPP_SOURCES "${cryptopp_SOURCE_DIR}/*.cpp")
list(REMOVE_ITEM CRYPTOPP_SOURCES
    "${cryptopp_SOURCE_DIR}/cryptlib.cpp"
    "${cryptopp_SOURCE_DIR}/cpu.cpp"
    "${cryptopp_SOURCE_DIR}/integer.cpp")
list(INSERT CRYPTOPP_SOURCES 0
    "${cryptopp_SOURCE_DIR}/cryptlib.cpp"
    "${cryptopp_SOURCE_DIR}/cpu.cpp"
    "${cryptopp_SOURCE_DIR}/integer.cpp")
add_library(cryptopp-static STATIC ${CRYPTOPP_SOURCES})
target_include_directories(cryptopp-static
    PRIVATE "${cryptopp_SOURCE_DIR}"   # internal: #include "aes.h"
    PUBLIC  "${CRYPTOPP_INCLUDE_DIR}"  # external: #include "cryptopp/aes.h"
)
set_target_properties(cryptopp-static PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
# README.txt: include -DNDEBUG for release builds to prevent assert-based
# information leaks to platform error-reporting tools (Apport, CrashReporter).
target_compile_definitions(cryptopp-static PRIVATE
    $<$<CONFIG:Release>:NDEBUG>)

# Mirror cryptopp's GNUmakefile: apply per-file SIMD flags so that only the
# files that need a given ISA extension request it. check_cxx_compiler_flag
# makes every call a no-op on ARM/Apple-Silicon/MSVC/non-x86 hosts.
if(NOT MSVC)
    include(CheckCXXCompilerFlag)

    # cryptopp_simd_flags(<basename> <flag> [<flag>...])
    # Applies the given compiler flags to a single cryptopp source file,
    # but only when *all* flags are supported by the current compiler.
    function(cryptopp_simd_flags src)
        set(flags ${ARGN})
        set(all_ok TRUE)
        foreach(flag IN LISTS flags)
            string(MAKE_C_IDENTIFIER "CRYPTOPP_HAS${flag}" var)
            check_cxx_compiler_flag("${flag}" ${var})
            if(NOT ${var})
                set(all_ok FALSE)
            endif()
        endforeach()
        if(all_ok)
            set(path "${cryptopp_SOURCE_DIR}/${src}")
            if(EXISTS "${path}")
                set_source_files_properties("${path}"
                    PROPERTIES COMPILE_OPTIONS "${flags}")
            endif()
        endif()
    endfunction()

    # SSE2
    cryptopp_simd_flags(chacha_simd.cpp   -msse2)
    cryptopp_simd_flags(donna_sse.cpp     -msse2)
    cryptopp_simd_flags(lsh256_sse2.cpp   -msse2)
    cryptopp_simd_flags(lsh512_sse2.cpp   -msse2)
    # SSSE3 + SSE4.1
    cryptopp_simd_flags(aria_simd.cpp     -mssse3 -msse4.1)
    cryptopp_simd_flags(blake2b_simd.cpp  -mssse3 -msse4.1)
    cryptopp_simd_flags(blake2s_simd.cpp  -mssse3 -msse4.1)
    cryptopp_simd_flags(cham_simd.cpp     -mssse3 -msse4.1)
    cryptopp_simd_flags(hight_simd.cpp    -mssse3 -msse4.1)
    cryptopp_simd_flags(lea_simd.cpp      -mssse3 -msse4.1)
    cryptopp_simd_flags(rijndael_simd.cpp -mssse3 -msse4.1 -maes)
    cryptopp_simd_flags(sha3_simd.cpp     -mssse3 -msse4.1)
    cryptopp_simd_flags(simon128_simd.cpp -mssse3 -msse4.1)
    cryptopp_simd_flags(sm4_simd.cpp      -mssse3 -msse4.1 -maes)
    cryptopp_simd_flags(speck128_simd.cpp -mssse3 -msse4.1)
    # SSE4.2 + AES-NI + PCLMUL
    cryptopp_simd_flags(crc_simd.cpp      -msse4.2 -mpclmul)
    cryptopp_simd_flags(gcm_simd.cpp      -msse4.2 -mpclmul -maes)
    cryptopp_simd_flags(gf2n_simd.cpp     -mpclmul)
    # SHA extensions
    cryptopp_simd_flags(sha_simd.cpp      -msse4.1 -msha)
    # AVX
    cryptopp_simd_flags(lsh256_avx.cpp    -mavx)
    cryptopp_simd_flags(lsh512_avx.cpp    -mavx)
    # AVX2
    cryptopp_simd_flags(chacha_avx.cpp    -mavx2)
    cryptopp_simd_flags(lsh224_avx2.cpp   -mavx2)
    cryptopp_simd_flags(lsh256_avx2.cpp   -mavx2)
    cryptopp_simd_flags(lsh384_avx2.cpp   -mavx2)
    cryptopp_simd_flags(lsh512_avx2.cpp   -mavx2)
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