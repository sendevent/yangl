#!/bin/sh

BUILD_DIR=./build

STARTED_AT=`date +"%I:%M:%S.%N"`

echo $BUILD_DIR

if [ ! -d $BUILD_DIR ] 
then
    mkdir ./build
fi

if [ ! -d $BUILD_DIR ] 
then
    echo "can't create out dir"
    exit 1
fi

cd $BUILD_DIR

qmake -r ../yangl.pro "CONFIG+=release"

if [ $? -ne 0 ]
then
  echo "Could not exec qmake"
  exit 2
fi

make -j`nproc`

if [ $? -ne 0 ]
then
  echo "Could not exec make"
  exit 3
fi

FINISHED_AT=`date +"%I:%M:%S.%N"`

make clean

mv ./app/yangl yangl

find . -name Makefile|xargs rm
rm -rf ./app ./tests ./test_fake_status 

echo "\nStarted:\t" $STARTED_AT
echo "Finished:\t" $FINISHED_AT
echo `realpath yangl`
