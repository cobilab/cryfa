cmake_minimum_required(VERSION 2.8.12)
project(cryfa)
#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(SOURCE_FILES src/cryfa.cpp src/EnDecrypto.cpp
        src/cryptopp/algparam.cpp
        src/cryptopp/asn.cpp
        src/cryptopp/base64.cpp
        src/cryptopp/basecode.cpp
        src/cryptopp/channels.cpp
        src/cryptopp/cpu.cpp
        src/cryptopp/cryptlib.cpp
        src/cryptopp/des.cpp
        src/cryptopp/dessp.cpp
#
        src/cryptopp_copy/dll.cpp
        src/cryptopp/ec2n.cpp
        src/cryptopp/ecp.cpp
        src/cryptopp/emsa2.cpp
        src/cryptopp/files.cpp
#
        src/cryptopp_copy/filters.cpp
        src/cryptopp/fips140.cpp
#
        src/cryptopp_copy/fipstest.cpp
#
        src/cryptopp_copy/gf2n.cpp
#
        src/cryptopp_copy/gfpcrypt.cpp
        src/cryptopp/hex.cpp
        src/cryptopp/hmac.cpp
#
        src/cryptopp_copy/hrtimer.cpp
#
        src/cryptopp_copy/integer.cpp
        src/cryptopp/iterhash.cpp
#
        src/cryptopp_copy/misc.cpp
        src/cryptopp/modes.cpp
#
        src/cryptopp_copy/mqueue.cpp
        src/cryptopp/nbtheory.cpp
        src/cryptopp/oaep.cpp
        src/cryptopp/osrng.cpp
        src/cryptopp/pkcspad.cpp
        src/cryptopp/pssr.cpp
        src/cryptopp/pubkey.cpp
        src/cryptopp/queue.cpp
#
        src/cryptopp_copy/randpool.cpp
        src/cryptopp/rdtables.cpp
        src/cryptopp/rijndael.cpp
        src/cryptopp/rng.cpp
#
        src/cryptopp_copy/rsa.cpp
        src/cryptopp/sha.cpp
        src/cryptopp/skipjack.cpp
#
        src/cryptopp_copy/wait.cpp
#
        src/cryptopp_copy/zdeflate.cpp
#
        src/cryptopp_copy/zinflate.cpp


#        src/cryptopp/3way.cpp
#        src/cryptopp/adler32.cpp
#        src/cryptopp/algebra.cpp
#        src/cryptopp/arc4.cpp
#        src/cryptopp/authenc.cpp
#        src/cryptopp/base32.cpp
#        src/cryptopp/bench1.cpp
#        src/cryptopp/bench2.cpp
#        src/cryptopp/bfinit.cpp
#        src/cryptopp/blake2.cpp
#        src/cryptopp/blowfish.cpp
#        src/cryptopp/blumshub.cpp
#        src/cryptopp/camellia.cpp
#        src/cryptopp/cast.cpp
#        src/cryptopp/casts.cpp
#        src/cryptopp/cbcmac.cpp
#        src/cryptopp/ccm.cpp
#        src/cryptopp/chacha.cpp
#        src/cryptopp/cmac.cpp
#        src/cryptopp/crc.cpp
#        src/cryptopp/datatest.cpp
#        src/cryptopp/default.cpp
#        src/cryptopp/dh2.cpp
#        src/cryptopp/dh.cpp
#        src/cryptopp/dlltest.cpp
#        src/cryptopp/dsa.cpp
#        src/cryptopp/eax.cpp
#        src/cryptopp/eccrypto.cpp
#        src/cryptopp/elgamal.cpp
#        src/cryptopp/eprecomp.cpp
#        src/cryptopp/esign.cpp
#        src/cryptopp/fipsalgt.cpp
#        src/cryptopp/gcm.cpp
#        src/cryptopp/gf2_32.cpp
#        src/cryptopp/gf256.cpp
#        src/cryptopp/gost.cpp
#        src/cryptopp/gzip.cpp
#        src/cryptopp/ida.cpp
#        src/cryptopp/idea.cpp
#        src/cryptopp/keccak.cpp
#        src/cryptopp/luc.cpp
#        src/cryptopp/mars.cpp
#        src/cryptopp/marss.cpp
#        src/cryptopp/md2.cpp
#        src/cryptopp/md4.cpp
#        src/cryptopp/md5.cpp
#        src/cryptopp/mqv.cpp
#        src/cryptopp/network.cpp
#        src/cryptopp/panama.cpp
#        src/cryptopp/pch.cpp
#        src/cryptopp/polynomi.cpp
#        src/cryptopp/rabin.cpp
#        src/cryptopp/rc2.cpp
#        src/cryptopp/rc5.cpp
#        src/cryptopp/rc6.cpp
#        src/cryptopp/rdrand.cpp
#        src/cryptopp/regtest1.cpp
#        src/cryptopp/regtest2.cpp
#        src/cryptopp/regtest3.cpp
#        src/cryptopp/ripemd.cpp
#        src/cryptopp/rw.cpp
#        src/cryptopp/safer.cpp
#        src/cryptopp/salsa.cpp
#        src/cryptopp/seal.cpp
#        src/cryptopp/seed.cpp
#        src/cryptopp/serpent.cpp
#        src/cryptopp/sha3.cpp
#        src/cryptopp/shacal2.cpp
#        src/cryptopp/sharkbox.cpp
#        src/cryptopp/shark.cpp
#        src/cryptopp/simple.cpp
#        src/cryptopp/socketft.cpp
#        src/cryptopp/sosemanuk.cpp
#        src/cryptopp/square.cpp
#        src/cryptopp/squaretb.cpp
#        src/cryptopp/strciphr.cpp
#        src/cryptopp/tea.cpp
#        src/cryptopp/test.cpp
#        src/cryptopp/tftables.cpp
#        src/cryptopp/tiger.cpp
#        src/cryptopp/tigertab.cpp
#        src/cryptopp/trdlocal.cpp
#        src/cryptopp/ttmac.cpp
#        src/cryptopp/twofish.cpp
#        src/cryptopp/validat1.cpp
#        src/cryptopp/validat2.cpp
#        src/cryptopp/validat3.cpp
#        src/cryptopp/vmac.cpp
#        src/cryptopp/wake.cpp
#        src/cryptopp/whrlpool.cpp
#        src/cryptopp/winpipes.cpp
#        src/cryptopp/xtr.cpp
#        src/cryptopp/xtrcrypt.cpp
#        src/cryptopp/zlib.cpp
        )
add_executable(cryfa ${SOURCE_FILES})
#target_link_libraries(cryfa ${CMAKE_SOURCE_DIR}/src/libcryptopp.a)
#target_link_libraries(cryfa ${CMAKE_SOURCE_DIR}/src/libcryptopp.so.5.6.5)