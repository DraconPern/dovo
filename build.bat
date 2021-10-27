IF "%1"=="Release" (
SET TYPE=Release
) ELSE (
SET TYPE=Debug
)

SET BUILD_DIR=%CD%
SET DEVSPACE=%CD%
SET CL=/MP
SET BOOSTTOOLSET=toolset=msvc-14.1

IF "%2"=="32" (
SET GENERATOR="Visual Studio 15 2017"
SET OPENSSLFLAG=VC-WIN32
) ELSE (
SET GENERATOR="Visual Studio 15 2017 Win64"
SET OPENSSLFLAG=VC-WIN64A
SET BOOSTADDRESSMODEL=address-model=64
)

cd %DEVSPACE%
git clone --branch=master --single-branch --depth=1 https://github.com/madler/zlib.git
cd zlib
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. -G %GENERATOR% -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\zlib\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\zlib\Release\lib\zlibstatic.lib %DEVSPACE%\zlib\Release\lib\zlib_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\zlib\Debug\lib\zlibstaticd.lib %DEVSPACE%\zlib\Debug\lib\zlib_d.lib

cd %DEVSPACE%
git clone https://github.com/DraconPern/libiconv-cmake.git
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
git clone https://github.com/DCMTK/dcmtk.git
cd dcmtk
git fetch
git checkout -f DCMTK-3.6.5
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DCMAKE_CXX_FLAGS_RELEASE="/Zi" -DDCMTK_WITH_ZLIB=1 -DWITH_ZLIBINC=%DEVSPACE%\zlib\%TYPE% -DDCMTK_WITH_ICONV=1 -DWITH_LIBICONVINC=%DEVSPACE%\libiconv\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk\%TYPE%
msbuild /maxcpucount:8 /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=openjpeg-2.1 https://github.com/uclouvain/openjpeg.git
cd openjpeg
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DBUILD_THIRDPARTY=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\openjpeg\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=master https://github.com/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_CXX_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE% -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2koj\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
if NOT EXIST boost_1_73_0.zip wget -c --no-check-certificate http://downloads.sourceforge.net/project/boost/boost/1.73.0/boost_1_73_0.zip
if NOT EXIST boost_1_73_0 unzip -o -q boost_1_73_0.zip
cd boost_1_73_0
call bootstrap
SET COMMONb2Flag=%BOOSTTOOLSET% %BOOSTADDRESSMODEL% runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage
SET BOOSTmodules=--with-locale --with-atomic --with-thread --with-filesystem --with-system --with-date_time --with-regex
IF "%TYPE%" == "Release" b2 %COMMONb2Flag% %BOOSTmodules% release
IF "%TYPE%" == "Debug"   b2 %COMMONb2Flag% %BOOSTmodules% debug

cd %DEVSPACE%
git clone --branch=master --recurse-submodule https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets
git checkout v3.1.4
git submodule update --init
set WXWIN=%DEVSPACE%\wxWidgets
cd %WXWIN%\build\msw
copy /Y %WXWIN%\include\wx\msw\setup0.h %WXWIN%\include\wx\msw\setup.h
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_ | ForEach {$_ -replace 'MultiThreadedDebugDLL', 'MultiThreadedDebug'}) | Set-Content $_ }"
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_ | ForEach {$_ -replace 'MultiThreadedDLL', 'MultiThreaded'}) | Set-Content $_ }"
msbuild /maxcpucount:5 /P:Configuration=%TYPE% wx_vc15.sln
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G %GENERATOR% -DwxWidgets_ROOT_DIR=%WXWIN% -DBoost_ROOT=%DEVSPACE%\boost_1_73_0 -DDCMTK_ROOT=%DEVSPACE%\dcmtk\%TYPE% -DZLIB_ROOT=%DEVSPACE%\zlib\%TYPE% -Dfmjpeg2k_ROOT=%DEVSPACE%\fmjpeg2koj\%TYPE% -DOpenJPEG_ROOT=%DEVSPACE%\openjpeg\%TYPE% -DVLD="C:\Program Files (x86)\Visual Leak Detector"
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
