IF "%1"=="Release" (
SET TYPE=Release
) ELSE (
SET TYPE=Debug
)

SET BUILD_DIR=%CD%
SET DEVSPACE=%CD%
SET CL=/MP

cd %DEVSPACE%
git clone --branch=master https://github.com/madler/zlib.git
cd zlib
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. -G "Visual Studio 11" -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\zlib\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\zlib\Release\lib\zlibstatic.lib %DEVSPACE%\zlib\Release\lib\zlib_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\zlib\Debug\lib\zlibstaticd.lib %DEVSPACE%\zlib\Debug\lib\zlib_d.lib

cd %DEVSPACE%
git clone git://git.dcmtk.org/dcmtk.git
cd dcmtk
git pull
git checkout -f 5371e1d84526e7544ab7e70fb47e3cdb4e9231b2
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DDCMTK_WITH_ZLIB=1 -DWITH_ZLIBINC=%DEVSPACE%\zlib\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk\%TYPE%
msbuild /maxcpucount:8 /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=openjpeg-2.1 https://github.com/uclouvain/openjpeg.git
cd openjpeg
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DBUILD_THIRDPARTY=1 -DBUILD_SHARED_LIBS=0 -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\openjpeg\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=master https://github.com/DraconPern/fmjpeg2koj.git
cd fmjpeg2koj
git pull
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DBUILD_SHARED_LIBS=OFF -DBUILD_THIRDPARTY=ON -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_CXX_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DOPENJPEG=%DEVSPACE%\openjpeg\%TYPE% -DDCMTK_DIR=%DEVSPACE%\dcmtk\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2koj\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
git clone --branch=master https://github.com/DraconPern/fmjpeg2kjasper.git
cd fmjpeg2kjasper
git pull
wget -c https://www.ece.uvic.ca/~frodo/jasper/software/jasper-1.900.1.zip
unzip -n jasper-1.900.1.zip
cd jasper
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DBUILD_SHARED_LIBS=0 -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DJASPERDIR=%DEVSPACE%\fmjpeg2kjasper\jasper-1.900.1 -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\jasper\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%
cd %DEVSPACE%
cd fmjpeg2kjasper
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DBUILD_SHARED_LIBS=OFF -DBUILD_THIRDPARTY=ON -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_CXX_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DJASPER=%DEVSPACE%\jasper\%TYPE% -DDCMTK_DIR=%DEVSPACE%\dcmtk\%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2kjasper\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %DEVSPACE%
wget -c http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.zip
unzip -n boost_1_60_0.zip
cd boost_1_60_0
call bootstrap
SET COMMONb2Flag=toolset=msvc-11.0 runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage
SET BOOSTmodules=--with-locale --with-atomic --with-thread --with-filesystem --with-system --with-date_time --with-regex
IF "%TYPE%" == "Release" b2 %COMMONb2Flag% %BOOSTmodules% release
IF "%TYPE%" == "Debug"   b2 %COMMONb2Flag% %BOOSTmodules% debug

cd %DEVSPACE%
git clone --branch=master https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets
git checkout v3.1.0
git pull
set WXWIN=%DEVSPACE%\wxWidgets
cd %WXWIN%\build\msw
copy /Y %WXWIN%\include\wx\msw\setup0.h %WXWIN%\include\wx\msw\setup.h
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_ | ForEach {$_ -replace 'MultiThreadedDebugDLL', 'MultiThreadedDebug'}) | Set-Content $_ }"
powershell "gci . *.vcxproj -recurse | ForEach { (Get-Content $_ | ForEach {$_ -replace 'MultiThreadedDLL', 'MultiThreaded'}) | Set-Content $_ }"
msbuild /maxcpucount:5 /P:Configuration=%TYPE% /p:Platform="Win32" wx_vc11.sln
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DwxWidgets_ROOT_DIR=%WXWIN% -DBOOST_ROOT=%DEVSPACE%\boost_1_60_0 -DDCMTK_DIR=%DEVSPACE%\dcmtk\%TYPE% -DZLIB_ROOT=%DEVSPACE%\zlib\%TYPE% -DFMJPEG2K=%DEVSPACE%\fmjpeg2koj\%TYPE% -DOPENJPEG=%DEVSPACE%\openjpeg\%TYPE% -DFMJP2K=%DEVSPACE%\fmjpeg2kjasper\%TYPE% -DJASPER=%DEVSPACE%\jasper\%TYPE% -DVLD="C:\Program Files (x86)\Visual Leak Detector"
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

cd %BUILD_DIR%
