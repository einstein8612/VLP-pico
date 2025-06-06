#!/bin/sh

# Clean up any previous build artifacts if clean is specified
if [ "$1" = "--clean" ]; then
    echo "Cleaning previous build artifacts..."
    rm -rf build
    echo "Clean complete"
fi

mkdir -p build
cd build
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi
echo "Build succeeded"

# Optionally, you can run tests if you have them set up
ctest --output-on-failure