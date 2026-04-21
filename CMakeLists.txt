cmake_minimum_required(VERSION 4.0.0)

project(cryfa)

# On MSVC x64 the cryptopp runtime uses hand-coded MASM routines
# (CPUID64, XGETBV64, Baseline_Add/Sub, SHA256/GCM/Rijndael_*_SSE2).
# Enable the MASM language so CMake can assemble x64dll.asm.
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    enable_language(ASM_MASM)
endif()

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
# Prefer an already-installed Crypto++ (for example from conda-forge/Bioconda)
# and fall back to building from source for local/dev and release builds.
find_path(CRYPTOPP_SYSTEM_INCLUDE_DIR NAMES cryptopp/aes.h)
find_library(CRYPTOPP_SYSTEM_LIBRARY NAMES cryptopp)

if(CRYPTOPP_SYSTEM_INCLUDE_DIR AND CRYPTOPP_SYSTEM_LIBRARY)
    add_library(cryptopp-dep UNKNOWN IMPORTED)
    set_target_properties(cryptopp-dep PROPERTIES
        IMPORTED_LOCATION "${CRYPTOPP_SYSTEM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${CRYPTOPP_SYSTEM_INCLUDE_DIR}"
    )
else()
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
    add_library(cryptopp-dep STATIC ${CRYPTOPP_SOURCES})
    target_include_directories(cryptopp-dep
        PRIVATE "${cryptopp_SOURCE_DIR}"   # internal: #include "aes.h"
        PUBLIC  "${CRYPTOPP_INCLUDE_DIR}"  # external: #include "cryptopp/aes.h"
    )
    set_target_properties(cryptopp-dep PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    # README.txt: include -DNDEBUG for release builds to prevent assert-based
    # information leaks to platform error-reporting tools (Apport, CrashReporter).
    target_compile_definitions(cryptopp-dep PRIVATE
        $<$<CONFIG:Release>:NDEBUG>)
    # On MSVC x64 add x64dll.asm which provides the hand-coded implementations of
    # CPUID64, XGETBV64, Baseline_Add, Baseline_Sub, SHA256_HashMultipleBlocks_SSE2,
    # GCM_AuthenticateBlocks_{2K,64K}_SSE2, Rijndael_Enc_AdvancedProcessBlocks_SSE2.
    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_sources(cryptopp-dep PRIVATE "${cryptopp_SOURCE_DIR}/x64dll.asm")
    endif()

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
        cryptopp_simd_flags(keccak_simd.cpp   -mssse3)
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
        cryptopp_simd_flags(shacal2_simd.cpp  -mssse3 -msse4.1 -msha)
        # SSSE3 (LSH SSE variants)
        cryptopp_simd_flags(lsh256_sse.cpp    -mssse3)
        cryptopp_simd_flags(lsh512_sse.cpp    -mssse3)
        # AVX2 (lsh*_avx files use AVX2 intrinsics, not just AVX)
        cryptopp_simd_flags(lsh256_avx.cpp    -mavx2)
        cryptopp_simd_flags(lsh512_avx.cpp    -mavx2)
        # AVX2
        cryptopp_simd_flags(chacha_avx.cpp    -mavx2)
        cryptopp_simd_flags(lsh224_avx2.cpp   -mavx2)
        cryptopp_simd_flags(lsh256_avx2.cpp   -mavx2)
        cryptopp_simd_flags(lsh384_avx2.cpp   -mavx2)
        cryptopp_simd_flags(lsh512_avx2.cpp   -mavx2)

        # ARM64 / AArch64 extensions (check_cxx_compiler_flag is a no-op on x86)
        cryptopp_simd_flags(crc_simd.cpp      -march=armv8-a+crc)
        cryptopp_simd_flags(gcm_simd.cpp      -march=armv8-a+crypto)
        cryptopp_simd_flags(gf2n_simd.cpp     -march=armv8-a+crypto)
        cryptopp_simd_flags(rijndael_simd.cpp -march=armv8-a+crypto)
        cryptopp_simd_flags(sha_simd.cpp      -march=armv8-a+crypto)
        cryptopp_simd_flags(shacal2_simd.cpp  -march=armv8-a+crypto)
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
target_link_libraries(libCryfaCommon PRIVATE cryptopp-dep)

add_executable(cryfa
    src/cryfa.cpp
)
target_include_directories(cryfa PRIVATE src/include)
target_link_libraries(cryfa PRIVATE
    Threads::Threads
    libCryfaCommon
    cryptopp-dep
)

add_executable(keygen
    src/keygen.cpp
)
target_include_directories(keygen PRIVATE src/include)

# ── CTest round-trip integration test ────────────────────────────────────────
enable_testing()
add_test(
    NAME roundtrip
    COMMAND ${CMAKE_COMMAND}
        -DCRYFA=$<TARGET_FILE:cryfa>
        -DPASS=${CMAKE_SOURCE_DIR}/pass.txt
        -DINPUT=${CMAKE_SOURCE_DIR}/example/in.fq
        -DWORKDIR=${CMAKE_BINARY_DIR}/test_roundtrip
        -P ${CMAKE_SOURCE_DIR}/cmake/roundtrip.cmake
)
