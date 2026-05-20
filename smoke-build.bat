@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /EHsc /std:c++17 /FoD:\CodeProj\cad-model-analyzer\build\x64-release\smoke.obj /Fe:D:\CodeProj\cad-model-analyzer\build\x64-release\smoke.exe D:\CodeProj\cad-model-analyzer\smoke.cpp
D:\CodeProj\cad-model-analyzer\build\x64-release\smoke.exe
