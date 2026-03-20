#!/bin/bash
set -xe

if [ "$1" == "Release" ]
then
  TYPE=Release
else
  TYPE=Debug
fi


if [ "$2" == "localgit" ]
then
  githuburl="http://gitcache:8080/github.com"
else
  githuburl="https://github.com"
fi

BUILD_DIR=`pwd`
DEVSPACE=`pwd`
unamestr=`uname`

if [ "$unamestr" == 'Darwin' ] ; then
cd $DEVSPACE
[[ -d openssl ]] || git clone $githuburl/openssl/openssl.git --branch OpenSSL_1_1_1-stable --single-branch --depth 1
cd openssl
git pull
./Configure darwin64-x86_64-cc  --prefix=$DEVSPACE/openssl/$TYPE --openssldir=$DEVSPACE/openssl/$TYPE/openssl no-shared
make install
export OPENSSL_ROOT_DIR=$DEVSPACE/openssl/$TYPE
# [ "$unamestr" == 'Darwin' ]
fi

cd $DEVSPACE
[[ -d dcmtk ]] || git clone --branch=DCMTK-3.7.0 $githuburl/DCMTK/dcmtk.git
cd dcmtk
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DDCMTK_ENABLE_CXX11=ON -DDCMTK_ENABLE_STL=ON -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DDCMTK_ENABLE_BUILTIN_DICTIONARY=1 -DDCMTK_WITH_TIFF=OFF -DDCMTK_WITH_PNG=OFF -DDCMTK_WITH_OPENSSL=OFF -DDCMTK_WITH_XML=OFF -DDCMTK_WITH_ZLIB=ON -DDCMTK_WITH_SNDFILE=OFF -DDCMTK_WITH_ICONV=ON -DDCMTK_WITH_WRAP=OFF -DCMAKE_INSTALL_PREFIX=$DEVSPACE/dcmtk/$TYPE
make -j8 install

#if [ "$unamestr" == 'Darwin' ] ; then
cd $DEVSPACE
[[ -d openjpeg ]] || git clone --branch=v2.4.0 --single-branch --depth 1 $githuburl/uclouvain/openjpeg.git
cd openjpeg
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/openjpeg/$TYPE
make -j8 install
export OPENJPEG_CMAKEARG=-DOpenJPEG_ROOT=$DEVSPACE/openjpeg/$TYPE
# [ "$unamestr" == 'Darwin' ]
#else
#export OPENJPEG_CMAKEARG=-DOPENJPEG_LIBRARY=/usr/lib/x86_64-linux-gnu/openjpeg.so
#fi

cd $DEVSPACE
[[ -d fmjpeg2koj ]] || git clone --branch=master $githuburl/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$TYPE $OPENJPEG_CMAKEARG -DDCMTK_ROOT=$DEVSPACE/dcmtk/$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/fmjpeg2koj/$TYPE
make -j8 install

cd $DEVSPACE
[[ -d boost ]] || git clone --branch=boost-1.88.0 --recurse-submodules $githuburl/boostorg/boost.git
export Boost_DIR=$DEVSPACE/boost/stage/lib/cmake/Boost-1.88.0
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
[[ -d wxWidgets ]] || git clone --branch=v3.2.10 --recurse-submodules $githuburl/wxWidgets/wxWidgets.git
cd wxWidgets
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DwxBUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=$TYPE -DCMAKE_INSTALL_PREFIX=$DEVSPACE/wxWidgets/$TYPE
make -j8 install

cd $BUILD_DIR
mkdir -p build-$TYPE
cd build-$TYPE
cmake .. -DCMAKE_BUILD_TYPE=$TYPE -DwxWidgets_DIR=$DEVSPACE/wxWidgets/$TYPE/lib64/cmake/wxWidgets-3.3 -DDCMTK_DIR=$DEVSPACE/dcmtk/$TYPE/lib64/cmake/dcmtk -Dfmjpeg2k_ROOT=$DEVSPACE/fmjpeg2koj/$TYPE $OPENJPEG_CMAKEARG -DZLIB_LIBRARY=/usr/lib/x86_64-linux-gnu/libz.so
make -j8
