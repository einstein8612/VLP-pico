#!/bin/sh

DEBUG_LED=0
CLEAN_BUILD=0
BOARD="pico"  # default board

# Parse all arguments
for arg in "$@"; do
    case "$arg" in
        --debug)
            echo "🔧 Debug mode enabled"
            DEBUG_LED=1
            ;;
        --clean)
            CLEAN_BUILD=1
            ;;
        --board=*)
            BOARD="${arg#*=}"
            echo "🛠️  Target board set to '$BOARD'"
            ;;
        *)
            echo "⚠️  Unknown argument: $arg"
            ;;
    esac
done

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "🧹 Cleaning build directory"
    rm -rf build
fi

mkdir -p build
cd build || exit 1

cmake -G Ninja -DPICO_BOARD=${BOARD} -DDEBUG_LED=${DEBUG_LED} ..
ninja
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi
echo "Build succeeded"

# Optionally, you can run tests if you have them set up
ctest --output-on-failure