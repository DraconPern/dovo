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

cd $DEVSPACE
[[ -d dcmtk ]] || git clone git://git.dcmtk.org/dcmtk.git
cd dcmtk
git checkout -f 5371e1d84526e7544ab7e70fb47e3cdb4e9231b2
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DDCMTK_WITH_TIFF=OFF -DDCMTK_WITH_PNG=OFF -DDCMTK_WITH_OPENSSL=OFF -DDCMTK_WITH_XML=OFF -DDCMTK_WITH_ZLIB=ON -DDCMTK_WITH_SNDFILE=OFF -DDCMTK_WITH_ICONV=OFF -DDCMTK_WITH_WRAP=OFF -DCMAKE_INSTALL_PREFIX=$DEVSPACE/dcmtk/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d openjpeg ]] || git clone --branch=openjpeg-2.1 https://github.com/uclouvain/openjpeg.git
cd openjpeg
git pull
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE -DBUILD_THIRDPARTY=ON -DCMAKE_INSTALL_PREFIX=$DEVSPACE/openjpeg/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d fmjpeg2koj ]] || git clone --branch=master https://github.com/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
git pull
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE -DOPENJPEG=$DEVSPACE/openjpeg/$TYPE -DDCMTK_DIR=$DEVSPACE/dcmtk/$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/fmjpeg2koj/$TYPE
make -j8 install

cd $DEVSPACE
wget -c http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.zip
unzip -n boost_1_60_0.zip
cd boost_1_60_0
./bootstrap.sh
COMMONb2Flag="-j 4 link=static runtime-link=static stage"
BOOSTModule="--with-locale --with-thread --with-filesystem --with-system --with-date_time --with-regex"
if [ "$TYPE" = "Release" ] ; then
  ./b2 $COMMONb2Flag $BOOSTModule variant=release
elif [ "$TYPE" = "Debug" ] ; then
  ./b2 $COMMONb2Flag $BOOSTModule variant=debug
fi

cd $DEVSPACE
[[ -d wxWidgets ]] || git clone --branch=master https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets
git checkout v3.1.0
mkdir -p build$TYPE
cd build$TYPE
unamestr=`uname`
if [ "$unamestr" == 'Darwin' ] ; then
  COMMONwxWidgetsFlag=(--disable-shared CXXFLAGS="-std=c++11 -stdlib=libc++" CPPFLAGS=-stdlib=libc++ LIBS=-lc++ -with-macosx-version-min=10.9)
elif [ "$unamestr" = 'Linux'] ; then
  COMMONwxWidgetsFlag=(--disable-shared CXXFLAGS="-std=c++11")
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
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DwxWidgets_CONFIG_EXECUTABLE=$DEVSPACE/wxWidgets/build$TYPE/wx-config -DBOOST_ROOT=$DEVSPACE/boost_1_60_0 -DDCMTK_DIR=$DEVSPACE/dcmtk/$TYPE -DFMJPEG2K=$DEVSPACE/fmjpeg2koj/$TYPE -DOPENJPEG=$DEVSPACE/openjpeg/$TYPE
make -j8
