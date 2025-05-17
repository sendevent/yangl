#!/bin/bash
set -e

BUILD_DIR=./debbuild
QT_DIR="$HOME/Qt/6.8.2/gcc_64"

STARTED_AT=$(date +"%I:%M:%S.%N")

echo "Build directory: $BUILD_DIR"
echo "Qt directory:    $QT_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr \
         -DCMAKE_PREFIX_PATH="$QT_DIR"

if [ $? -ne 0 ]; then
    echo "Could not execute cmake"
    exit 1
fi

make -j"$(nproc)"

if [ $? -ne 0 ]; then
    echo "Could not execute make"
    exit 2
fi

cpack -G DEB

if [ $? -ne 0 ]; then
    echo "Could not create .deb package"
    exit 3
fi

FINISHED_AT=$(date +"%I:%M:%S.%N")

echo ""
echo -e "Started:\t$STARTED_AT"
echo -e "Finished:\t$FINISHED_AT"
echo ""
echo "Package created:"
ls -lh *.deb
