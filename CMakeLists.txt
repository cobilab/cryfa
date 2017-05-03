cmake_minimum_required(VERSION 2.8.12)
project(cryfa)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES src/cryfa.cpp src/EnDecrypto.cpp)
add_executable(cryfa ${SOURCE_FILES})
target_link_libraries(cryfa ${CMAKE_SOURCE_DIR}/src/libcryptopp.a)