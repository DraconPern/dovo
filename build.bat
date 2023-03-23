IF "%1"=="Release" (
SET TYPE=Release
) ELSE (
SET TYPE=Debug
)

SET BUILD_DIR=%CD%
SET DEVSPACE=%CD%
SET CL=/MP
SET BOOSTTOOLSET=toolset=msvc

SET GENERATOR=-G "Visual Studio 17 2022" -A x64
SET OPENSSLFLAG=VC-WIN64A
SET BOOSTADDRESSMODEL=address-model=64

cd %DEVSPACE%
git clone --branch=master --single-branch --depth=1 https://github.com/madler/zlib.git
cd zlib
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. %GENERATOR% -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_C_FLAGS_DEBUG="/MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\zlib\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\zlib\Release\lib\zlibstatic.lib %DEVSPACE%\zlib\Release\lib\zlib_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\zlib\Debug\lib\zlibstaticd.lib %DEVSPACE%\zlib\Debug\lib\zlib_d.lib

cd %DEVSPACE%
git clone https://github.com/DraconPern/libiconv-cmake.git
cd libiconv-cmake
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. %GENERATOR% -DBUILD_SHARED_LIBS=0 -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_C_FLAGS_DEBUG="/MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\libiconv\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\libiconv\Debug\lib\libiconv.lib %DEVSPACE%\libiconv\Debug\lib\libiconv_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\libiconv\Debug\lib\libiconv.lib %DEVSPACE%\libiconv\Debug\lib\libiconv_d.lib
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\libiconv\Release\lib\libiconv.lib %DEVSPACE%\libiconv\Release\lib\libiconv_o.lib

if DEFINED FORCEBUILD goto buildssl
if EXIST "%DEVSPACE%\openssl\%TYPE%\lib\libcrypto.lib" goto dontbuildssl
:buildssl
cd %DEVSPACE%
git clone https://github.com/openssl/openssl.git --branch OpenSSL_1_1_1-stable --single-branch --depth 1
cd openssl
SET OLDPATH=%PATH%
IF "%TYPE%" == "Release" perl Configure -D_CRT_SECURE_NO_WARNINGS=1 no-asm no-shared --openssldir=%DEVSPACE%\openssl\Release --prefix=%DEVSPACE%\openssl\Release %OPENSSLFLAG%
IF "%TYPE%" == "Debug"   perl Configure -D_CRT_SECURE_NO_WARNINGS=1 no-asm no-shared --openssldir=%DEVSPACE%\openssl\Debug --prefix=%DEVSPACE%\openssl\Debug %OPENSSLFLAG%
nmake install
SET PATH=%OLDPATH%
:dontbuildssl
SET OPENSSL_ROOT_DIR=%DEVSPACE%\openssl\%TYPE%

cd %DEVSPACE%
git clone --branch=DCMTK-3.6.5 https://github.com/DCMTK/dcmtk.git
cd dcmtk
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. %GENERATOR% -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DCMAKE_CXX_FLAGS_RELEASE="/Zi" -DDCMTK_WITH_ZLIB=1 -DDCMTK_WITH_OPENSSL=0 -DWITH_ZLIBINC=%DEVSPACE%\zlib\%TYPE% -DDCMTK_WITH_ICONV=1 -DWITH_LIBICONVINC=%DEVSPACE%\libiconv\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=v2.4.0 --single-branch --depth 1 https://github.com/uclouvain/openjpeg.git
cd openjpeg
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. %GENERATOR% -DBUILD_THIRDPARTY=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_C_FLAGS_DEBUG="/MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\openjpeg\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=master https://github.com/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. %GENERATOR% -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2" -DCMAKE_CXX_FLAGS_DEBUG="/MTd /Od" -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE% -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2koj\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=boost-1.81.0 --recurse-submodules https://github.com/boostorg/boost.git
cd boost
call bootstrap
SET COMMONb2Flag=%BOOSTTOOLSET% %BOOSTADDRESSMODEL% runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage
SET BOOSTmodules=--with-locale --with-atomic --with-thread --with-filesystem --with-system --with-date_time --with-regex
IF "%TYPE%" == "Release" b2 %COMMONb2Flag% %BOOSTmodules% release
IF "%TYPE%" == "Debug"   b2 %COMMONb2Flag% %BOOSTmodules% debug

cd %DEVSPACE%
git clone --branch=v3.1.6 --recurse-submodule https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. %GENERATOR% -DwxBUILD_SHARED=FALSE -DwxUSE_ZLIB=OFF -DwxUSE_LIBTIFF=OFF -DwxUSE_LIBPNG=OFF -DwxUSE_ARTPROVIDER_TANGO=OFF -DwxUSE_SVG=OFF -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\wxWidgets\%TYPE%
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_.FullName | ForEach {$_ -replace 'MultiThreadedDebugDLL', 'MultiThreadedDebug'}) | Set-Content $_.FullName }"
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_.FullName | ForEach {$_ -replace 'MultiThreadedDLL', 'MultiThreaded'}) | Set-Content $_.FullName }"
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. %GENERATOR% -DwxWidgets_ROOT_DIR=%DEVSPACE%\wxWidgets\%TYPE% -DBoost_ROOT=%DEVSPACE%\boost -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DZLIB_ROOT=%DEVSPACE%\zlib\%TYPE% -Dfmjpeg2k_ROOT=%DEVSPACE%\fmjpeg2koj\%TYPE% -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE% -DOPENSSL_ROOT_DIR=%OPENSSL_ROOT_DIR% -DVLD="C:\Program Files (x86)\Visual Leak Detector"
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
