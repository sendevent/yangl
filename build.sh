#!/bin/bash

BUILD_DIR=./scriptbuild

STARTED_AT=`date +"%I:%M:%S.%N"`

echo $BUILD_DIR

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

if [ ! -d $BUILD_DIR ]; then
    echo "Can't create build directory"
    exit 1
fi

cd $BUILD_DIR

cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "Could not execute cmake"
    exit 2
fi

make -j`nproc`

if [ $? -ne 0 ]; then
    echo "Could not execute make"
    exit 3
fi

FINISHED_AT=`date +"%I:%M:%S.%N"`

mv ./src/yangl yangl

make clean

find . -name Makefile | xargs rm
rm -rf ./app ./tests ./test_fake_status

echo -e "\nStarted:\t$STARTED_AT"
echo -e "Finished:\t$FINISHED_AT"
echo "App binary:" $(realpath yangl)


