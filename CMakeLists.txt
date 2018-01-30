cmake_minimum_required(VERSION 3.3.2)
project(cryfa)
SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB SOURCE_FILES "src/*.h" "src/*.hpp" "src/*.cpp" "src/cryptopp/*.cpp")
add_executable(cryfa ${SOURCE_FILES})