@echo off
setlocal
set "VSDEV=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
set "OCCT_ROOT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64"
set "OCCT_3RDPARTY=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\3rdparty-vc14-64"
call "%VSDEV%" -arch=x64 -host_arch=x64 >nul
set "PATH=%~dp0build\x64-release;%OCCT_ROOT%\win64\vc14\bin;%OCCT_3RDPARTY%\freetype-2.13.3-x64\bin;%OCCT_3RDPARTY%\tbb-2021.13.0-x64\bin;%OCCT_3RDPARTY%\jemalloc-vc14-64\bin;%PATH%"
for %%F in ("%~dp0build\x64-release\*.dll") do (
  echo === %%~nxF ===
  dumpbin /dependents "%%F" | findstr /i /r "^[ ]*[A-Za-z0-9_\.-]*\.dll"
)
