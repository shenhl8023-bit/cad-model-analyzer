@echo off
setlocal
set "OCCT_ROOT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64"
set "OCCT_3RDPARTY=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\3rdparty-vc14-64"
set "PATH=%OCCT_ROOT%\win64\vc14\bin;%OCCT_3RDPARTY%\freetype-2.13.3-x64\bin;%OCCT_3RDPARTY%\tbb-2021.13.0-x64\bin;%OCCT_3RDPARTY%\jemalloc-vc14-64\bin;%PATH%"
echo PATH=%PATH%
echo EXE=%~dp0build\x64-release\cad_model_analyzer.exe
where tbb12.dll
where jemalloc.dll
where TKDESTEP.dll
"%~dp0build\x64-release\cad_model_analyzer.exe" %*
echo EXIT=%ERRORLEVEL%
