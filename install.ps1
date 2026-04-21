# Parameters
$BUILD_TYPE = "Release"
$BUILD = "build"
$PARALLEL = 8

# Configure CMake
cmake -B $BUILD -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
cmake --build $BUILD --parallel $PARALLEL --config $BUILD_TYPE

# Move executables to the main directory
Move-Item "$BUILD\$BUILD_TYPE\cryfa.exe"  -Destination .
Move-Item "$BUILD\$BUILD_TYPE\keygen.exe" -Destination .
