SET TYPE=Release
SET TYPE=Debug

REM a top level directory for all PACS related code
SET DEVSPACE="%CD%"

set WXWIN=%DEVSPACE%\wxWidgets
cd %WXWIN%\build\msw
IF "%TYPE%" == "Release" nmake /f makefile.vc BUILD=release RUNTIME_LIBS=static
IF "%TYPE%" == "Debug"   nmake /f makefile.vc BUILD=debug RUNTIME_LIBS=static

cd %DEVSPACE%\boost	
call bootstrap toolset=vc11 
IF "%TYPE%" == "Release" b2 toolset=msvc-11.0 runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage release
IF "%TYPE%" == "Debug"   b2 toolset=msvc-11.0 runtime-link=static define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage debug
cd ..

cd %DEVSPACE%\zlib
mkdir build-%TYPE%
cd build-%TYPE%
cmake.exe .. -G "Visual Studio 11" -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /D NDEBUG" -DCMAKE_C_FLAGS_DEBUG="/D_DEBUG /MTd /Od" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\zlib\build-%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
cd ..\..
IF "%TYPE%" == "Release" copy /Y %DEVSPACE%\zlib\build-Release\lib\zlibstatic.lib %DEVSPACE%\zlib\build-Release\lib\zlib_o.lib
IF "%TYPE%" == "Debug"   copy /Y %DEVSPACE%\zlib\build-Debug\lib\zlibstaticd.lib %DEVSPACE%\zlib\build-Debug\lib\zlib_d.lib

cd %DEVSPACE%\dcmtk
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1 -DDCMTK_WITH_ZLIB=1 -DWITH_ZLIBINC=%DEVSPACE%\zlib\build-%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk\build-%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj 
cd ..\..

cd %DEVSPACE%\ImageMagick\VisualMagick
msbuild /P:Configuration=%TYPE% /T:CORE_JP2 VisualStaticMT.sln
cd ..\..

cd %DEVSPACE%\fmjpeg2k
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DIMAGEMAGICK=%DEVSPACE%\ImageMagick -DDCMTK_WITH_ZLIB=1 -DWITH_ZLIBINC=%DEVSPACE%\zlib\build-%TYPE% -DDCMTK_DIR=%DEVSPACE%\dcmtk\build-%TYPE% -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\fmjpeg2k\%TYPE%
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj
cd ..\..

cd %DEVSPACE%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DwxWidgets_ROOT_DIR=%WXWIN% -DIMAGEMAGICK=%DEVSPACE%\ImageMagick -DBOOST_ROOT=%DEVSPACE%\boost -DDCMTK_DIR=%DEVSPACE%\dcmtk\build-%TYPE% -DFMJPEG2K=%DEVSPACE%\fmjpeg2k\%TYPE%
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
cd ..

cd %DEVSPACE%
