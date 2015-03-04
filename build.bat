SET TYPE=Release
REM SET TYPE=Debug

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

cd %DEVSPACE%\dcmtk
cmake -G "Visual Studio 11" -DCMAKE_INSTALL_PREFIX=%DEVSPACE%\dcmtk
msbuild /P:Configuration=%TYPE% INSTALL.vcxproj 
cd ..\..

cd %DEVSPACE%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DwxWidgets_ROOT_DIR=%WXWIN% -DBOOST_ROOT=%DEVSPACE%\boost -DDCMTK_DIR=%DEVSPACE%\dcmtk
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
cd ..

cd %DEVSPACE%
