#!/bin/bash
set -xe

if [ "$1" == "Release" ]
then
  TYPE=Release
else
  TYPE=Debug
fi

BUILD_DIR=`pwd`
DEVSPACE=`pwd`

./build.sh $1

cd $BUILD_DIR/build-$TYPE
hdiutil create -volname dovo -srcfolder $BUILD_DIR/build-$TYPE/dovo.app -ov -format UDZO dovo.dmg
