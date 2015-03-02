SET TYPE=Release
SET TYPE=Debug

REM a top level directory for all PACS related code
SET DEVSPACE="%CD%"

cd %WXWIN%\build\msw
nmake /f makefile.vc BUILD=%TYPE%
set WXWIN=%DEVSPACE%\wxWidgets

cd %DEVSPACE%\boost	
call bootstrap toolset=vc11 
IF "%TYPE%" == "Release" b2 toolset=msvc-11.0 runtime-link=shared define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage release
IF "%TYPE%" == "Debug"   b2 toolset=msvc-11.0 runtime-link=shared define=_BIND_TO_CURRENT_VCLIBS_VERSION=1 -j 4 stage debug
cd ..

cd %DEVSPACE%
mkdir build-%TYPE%
cd build-%TYPE%
cmake .. -G "Visual Studio 11" -DwxWidgets_ROOT_DIR=%WXWIN% -DBOOST_ROOT=%DEVSPACE%\boost
msbuild /P:Configuration=%TYPE% ALL_BUILD.vcxproj
cd ..

cd %DEVSPACE%
