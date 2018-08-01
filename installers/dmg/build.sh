#!/bin/bash
set -xe

if [ "$1" == "Release" ]
then
  TYPE=Release
else
  TYPE=Debug
fi

BUILD_DIR=`pwd`

hdiutil create -volname dovo -srcfolder ../../build-$TYPE/dovo.app -ov -format UDZO dovo.dmg
