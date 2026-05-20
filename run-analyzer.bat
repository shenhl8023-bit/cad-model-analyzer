@echo off
setlocal

set "OCCT_ROOT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64"
set "OCCT_3RDPARTY=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\3rdparty-vc14-64"
set "PATH=%~dp0build\x64-release;%OCCT_ROOT%\win64\vc14\bin;%OCCT_3RDPARTY%\openvr-1.14.15-64\bin\win64;%OCCT_3RDPARTY%\msvc-vc14-64;%OCCT_3RDPARTY%\freetype-2.13.3-x64\bin;%OCCT_3RDPARTY%\tbb-2021.13.0-x64\bin;%OCCT_3RDPARTY%\jemalloc-vc14-64\bin;%OCCT_3RDPARTY%\tcltk-8.6.15-x64\bin;%OCCT_3RDPARTY%\freeimage-3.18.0-x64\bin;%OCCT_3RDPARTY%\ffmpeg-3.3.4-64\bin;%PATH%"

if "%~1"=="" (
  "%~dp0build\x64-release\cad_model_analyzer.exe"
) else (
  "%~dp0build\x64-release\cad_model_analyzer.exe" %*
)
