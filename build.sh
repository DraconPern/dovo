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

unamestr=`uname`

cd $DEVSPACE
[[ -d dcmtk ]] || git clone https://github.com/DCMTK/dcmtk.git
cd dcmtk
git fetch
git checkout -f DCMTK-3.6.5
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DDCMTK_ENABLE_CXX11=ON -DDCMTK_ENABLE_STL=ON -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DDCMTK_ENABLE_BUILTIN_DICTIONARY=1 -DDCMTK_WITH_TIFF=OFF -DDCMTK_WITH_PNG=OFF -DDCMTK_WITH_OPENSSL=OFF -DDCMTK_WITH_XML=OFF -DDCMTK_WITH_ZLIB=ON -DDCMTK_WITH_SNDFILE=OFF -DDCMTK_WITH_ICONV=ON -DDCMTK_WITH_WRAP=OFF -DCMAKE_INSTALL_PREFIX=$DEVSPACE/dcmtk/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d openjpeg ]] || git clone --branch=v2.4.0 --single-branch --depth 1 https://github.com/uclouvain/openjpeg.git
cd openjpeg
git pull
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/openjpeg/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d fmjpeg2koj ]] || git clone --branch=master https://github.com/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
git pull
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE -DOpenJPEG_ROOT=$DEVSPACE/openjpeg/$TYPE -DDCMTK_ROOT=$DEVSPACE/dcmtk/$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/fmjpeg2koj/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d boost ]] || git clone --branch=boost-1.73.0 --recurse-submodules https://github.com/boostorg/boost.git
cd boost
./bootstrap.sh
COMMONb2Flag="-j 4 link=static stage"
BOOSTModule="--with-locale --with-thread --with-filesystem --with-system --with-date_time --with-regex"
if [ "$TYPE" = "Release" ] ; then
  ./b2 $COMMONb2Flag $BOOSTModule variant=release
elif [ "$TYPE" = "Debug" ] ; then
  ./b2 $COMMONb2Flag $BOOSTModule variant=debug
fi

cd $DEVSPACE
[[ -d wxWidgets ]] || git clone --branch=master --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets
mkdir -p build$TYPE
cd build$TYPE
if [ "$unamestr" = "Darwin" ] ; then
  COMMONwxWidgetsFlag=(--disable-shared --enable-utf8)
elif [ "$unamestr" = "Linux" ] ; then
  COMMONwxWidgetsFlag=(--disable-shared --enable-utf8)
fi
if [ "$TYPE" = "Release" ] ; then
  ../configure "${COMMONwxWidgetsFlag[@]}"
elif [ "$TYPE" = "Debug" ] ; then
  ../configure "${COMMONwxWidgetsFlag[@]}" --enable-debug
fi
make -j8

cd $BUILD_DIR
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DwxWidgets_CONFIG_EXECUTABLE=$DEVSPACE/wxWidgets/build$TYPE/wx-config -DBoost_ROOT=$DEVSPACE/boost -DDCMTK_ROOT=$DEVSPACE/dcmtk/$TYPE -Dfmjpeg2k_ROOT=$DEVSPACE/fmjpeg2koj/$TYPE -DOpenJPEG_ROOT=$DEVSPACE/openjpeg/$TYPE
make -j8
