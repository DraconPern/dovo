#!/bin/sh
#TYPE=Release
TYPE=Debug

# a top level directory for all PACS related code
DEVSPACE=`pwd`

SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
MACOSX_DEPLOYMENT_TARGET=10.9

cd $DEVSPACE/wxWidgets
mkdir build$TYPE
cd build$TYPE
COMMONwxWidgetsFlag="--disable-shared --with-macosx-version-min=$MACOSX_DEPLOYMENT_TARGET --with-macosx-sdk=$SYSROOT"
if [ "$TYPE" = "Release" ] ; then 
  ../configure $COMMONwxWidgetsFlag
elif [ "$TYPE" = "Debug" ] ; then
  ../configure $COMMONwxWidgetsFlag --enable-debug
fi
make -j8
cd ../..

cd $DEVSPACE/boost
./bootstrap.sh 
if [ "$TYPE" = "Release" ] ; then
  ./b2 toolset=clang link=static variant=release -j 4 macosx-version-min=$MACOSX_DEPLOYMENT_TARGET stage
elif [ "$TYPE" = "Debug" ] ; then
  ./b2 link=static variant=debug -j 4 runtime-link=static macosx-version-min=10.9 stage 
fi
cd ..

cd $DEVSPACE/dcmtk
mkdir build-$TYPE
cd build-$TYPE
cmake .. -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DCMAKE_INSTALL_PREFIX=$DEVSPACE/dcmtk/build-$TYPE -DCMAKE_OSX_SYSROOT=$SYSROOT -DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET
make -j8 install 
cd ../..

cd $DEVSPACE
mkdir build-$TYPE
cd build-$TYPE
cmake .. -DwxWidgets_CONFIG_EXECUTABLE=$DEVSPACE/wxWidgets/build$TYPE/wx-config -DBOOST_ROOT=$DEVSPACE/boost -DDCMTK_DIR=$DEVSPACE/dcmtk/build-$TYPE -DCMAKE_OSX_SYSROOT=$SYSROOT -DCMAKE_OSX_DEPLOYMENT_TARGET=$CMAKE_OSX_DEPLOYMENT_TARGET
make -j8
cd ..

echo "If you are getting a fchmodat error, please modify boost/libs/filessystem/src/operations.cpp.  Find the call to fchmodat and disable the #if using '#if 0 &&"
echo "If you are getting macosx-version-min error, in boost/tools/build/src/tools/darwin.jam after feature macosx-version-min add a new line"
echo "feature.extend macosx-version-min : 10.9 ;"