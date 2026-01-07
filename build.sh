#!/bin/bash
set -e

BUILD_DIR=./scriptbuild
QT_DIR=""

STARTED_AT=$(date +"%I:%M:%S.%N")

# --- Parse arguments ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        --qt-dir)
            QT_DIR="$2"
            shift 2
            ;;
        --qt-dir=*)
            QT_DIR="${1#*=}"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--qt-dir <path>]"
            exit 1
            ;;
    esac
done

# --- Configure ---
echo "Build directory: $BUILD_DIR"

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE=Release
)

if [ -n "$QT_DIR" ]; then
    echo "Qt directory:    $QT_DIR"
    CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$QT_DIR")
else
    echo "Qt directory:    (system default)"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. "${CMAKE_ARGS[@]}"

if [ $? -ne 0 ]; then
    echo "Could not execute cmake"
    exit 1
fi

# --- Build ---
make -j"$(nproc)"

if [ $? -ne 0 ]; then
    echo "Could not execute make"
    exit 2
fi

FINISHED_AT=$(date +"%I:%M:%S.%N")

echo ""
echo -e "Started:\t$STARTED_AT"
echo -e "Finished:\t$FINISHED_AT"
echo ""
echo "App binary: $(realpath ./src/yangl)"
