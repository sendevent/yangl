#!/bin/bash
set -e

BUILD_DIR=./debbuild
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

# --- Check for dpkg-deb ---
if ! command -v dpkg-deb &> /dev/null; then
    echo "Error: dpkg-deb is not installed."
    echo "Install it with: sudo apt install dpkg  (Debian/Ubuntu)"
    exit 1
fi

# --- Configure ---
echo "Build directory: $BUILD_DIR"

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=/usr
)

if [ -n "$QT_DIR" ]; then
    echo "Qt directory:    $QT_DIR"
    CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$QT_DIR")
else
    echo "Qt directory:    (system default)"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. "${CMAKE_ARGS[@]}" || { echo "Could not execute cmake"; exit 1; }

# --- Build ---
make -j"$(nproc)" || { echo "Could not execute make"; exit 2; }

# --- Package ---
cpack -G DEB || { echo "Could not create .deb package"; exit 3; }

FINISHED_AT=$(date +"%I:%M:%S.%N")

echo ""
echo -e "Started:\t$STARTED_AT"
echo -e "Finished:\t$FINISHED_AT"
echo ""
echo "Package created:"
ls -lh *.deb
