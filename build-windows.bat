@echo off
setlocal

set "VSDEV=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
set "CMAKE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "NINJA=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
set "OCCT_ROOT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64"
set "OCCT_3RDPARTY=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\3rdparty-vc14-64"

if not exist "%VSDEV%" (
  echo VsDevCmd not found: %VSDEV%
  exit /b 1
)

call "%VSDEV%" -arch=x64 -host_arch=x64
set "PATH=%NINJA%;%OCCT_ROOT%\win64\vc14\bin;%OCCT_3RDPARTY%\openvr-1.14.15-64\bin\win64;%OCCT_3RDPARTY%\msvc-vc14-64;%OCCT_3RDPARTY%\freetype-2.13.3-x64\bin;%OCCT_3RDPARTY%\tbb-2021.13.0-x64\bin;%OCCT_3RDPARTY%\jemalloc-vc14-64\bin;%OCCT_3RDPARTY%\tcltk-8.6.15-x64\bin;%OCCT_3RDPARTY%\freeimage-3.18.0-x64\bin;%OCCT_3RDPARTY%\ffmpeg-3.3.4-64\bin;%PATH%"

"%CMAKE%" -S "%~dp0." -B "%~dp0build\x64-release" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOpenCASCADE_DIR="%OCCT_ROOT%\cmake"
if errorlevel 1 exit /b %errorlevel%

"%CMAKE%" --build "%~dp0build\x64-release"
if errorlevel 1 exit /b %errorlevel%

echo.
echo Build completed:
echo %~dp0build\x64-release\cad_model_analyzer.exe
