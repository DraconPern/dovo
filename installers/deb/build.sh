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

mkdir -p container/DEBIAN
cp control container/DEBIAN/

mkdir -p container/usr/share/dovo
cp ../../build-$TYPE/dovo container/usr/share/dovo/

mkdir -p container/usr/bin
cd container/usr/bin
ln -r -s -f ../share/dovo/dovo dovo

cd $BUILD_DIR
mkdir -p container/usr/share/applications
cp dovo.desktop container/usr/share/applications/

mkdir -p container/usr/share/pixmaps
cp dovo.png container/usr/share/pixmaps/

cd $BUILD_DIR
dpkg-deb --build container

mv container.deb dovo.deb
