#!/bin/bash
set -xe

if [ "$1" == "Release" ]
then
  TYPE=Release
else
  TYPE=Debug
fi

mkdir -p container/DEBIAN
cp control container/DEBIAN/
mkdir -p container/usr/bin
cp ../../build-$TYPE/dovo container/usr/bin/
dpkg-deb --build container
