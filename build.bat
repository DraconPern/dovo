IF "%1"=="Release" (
SET TYPE=Release
) ELSE (
SET TYPE=Debug
)

IF "%2"=="localgit" (
SET GITHUBURL=http://gitcache:8080/github.com
) ELSE (
SET GITHUBURL=https://github.com
)

SET BUILD_DIR=%CD%
SET DEVSPACE=%CD%
SET CL=/MP

SET GENERATOR="Visual Studio 17 2022" -A x64

cd %DEVSPACE%
git clone --branch=master --single-branch --depth=1 %GITHUBURL%/madler/zlib.git
cd zlib
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. -G %GENERATOR% -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\zlib\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\zlib\Release\lib\zs.lib %DEVSPACE%\zlib\Release\lib\zlib_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\zlib\Debug\lib\zsd.lib %DEVSPACE%\zlib\Debug\lib\zlib_d.lib

cd %DEVSPACE%
git clone %GITHUBURL%/DraconPern/libiconv-cmake.git
cd libiconv-cmake
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. -G %GENERATOR% -DBUILD_SHARED_LIBS=0 -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_C_FLAGS_DEBUG="/MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\libiconv\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\libiconv\Debug\lib\libiconv.lib %DEVSPACE%\libiconv\Debug\lib\libiconv_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\libiconv\Debug\lib\libiconv.lib %DEVSPACE%\libiconv\Debug\lib\libiconv_d.lib
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\libiconv\Release\lib\libiconv.lib %DEVSPACE%\libiconv\Release\lib\libiconv_o.lib

cd %DEVSPACE%
git clone --branch=DCMTK-3.7.0 %GITHUBURL%/DCMTK/dcmtk.git
cd dcmtk
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DCMAKE_CXX_FLAGS_RELEASE="/Zi" -DDCMTK_WITH_ZLIB=1 -DDCMTK_WITH_OPENSSL=0 -DWITH_ZLIBINC=%DEVSPACE%\zlib\%TYPE% -DDCMTK_WITH_ICONV=1 -DWITH_LIBICONVINC=%DEVSPACE%\libiconv\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=v2.4.0 --single-branch --depth 1 %GITHUBURL%/uclouvain/openjpeg.git
cd openjpeg
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DBUILD_THIRDPARTY=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_C_FLAGS_DEBUG="/MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\openjpeg\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=master %GITHUBURL%/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2" -DCMAKE_CXX_FLAGS_DEBUG="/MTd /Od" -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE% -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2koj\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=boost-1.88.0 --recurse-submodules %GITHUBURL%/boostorg/boost.git
set Boost_DIR=%DEVSPACE%\boost\stage\lib\cmake\Boost-1.88.0
cd boost
call bootstrap
SET BOOSTTOOLSET=toolset=msvc
SET BOOSTADDRESSMODEL=address-model=64
SET COMMONb2Flag=%BOOSTTOOLSET% %BOOSTADDRESSMODEL% runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage
SET BOOSTmodules=--with-locale --with-atomic --with-thread --with-filesystem --with-system --with-date_time --with-regex
b2 headers
IF "%TYPE%" == "Release" b2 %COMMONb2Flag% %BOOSTmodules% release
IF "%TYPE%" == "Debug"   b2 %COMMONb2Flag% %BOOSTmodules% debug

cd %DEVSPACE%
git clone --branch=v3.3.2 --recurse-submodule %GITHUBURL%/wxWidgets/wxWidgets.git
cd wxWidgets
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DwxBUILD_SHARED=FALSE -DwxBUILD_USE_STATIC_RUNTIME=TRUE -DwxUSE_ZLIB=OFF -DwxUSE_LIBTIFF=OFF -DwxUSE_LIBPNG=OFF -DwxUSE_ARTPROVIDER_TANGO=OFF -DwxUSE_SVG=OFF -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\wxWidgets\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone https://github.com/laudrup/boost-wintls.git

cd %BUILD_DIR%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DwxWidgets_DIR=%DEVSPACE%\wxWidgets\%TYPE%\lib\cmake\wxWidgets-3.3 -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DZLIB_ROOT=%DEVSPACE%\zlib\%TYPE% -Dfmjpeg2k_ROOT=%DEVSPACE%\fmjpeg2koj\%TYPE% -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE%
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
