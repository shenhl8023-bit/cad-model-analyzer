@echo off
setlocal

set "ROOT=%~dp0"
set "INPUT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step"
set "OUTPUT=%ROOT%output\test-screw-report.json"
set "OUTPUT_O=%ROOT%output\test-screw-report-o.json"
set "EXE=%ROOT%build\x64-release\cad_model_analyzer.exe"

if not exist "%ROOT%output" mkdir "%ROOT%output"

"%EXE%" --help >nul
if errorlevel 1 exit /b %errorlevel%

"%EXE%" --version >nul
if errorlevel 1 exit /b %errorlevel%

call "%ROOT%run-analyzer.bat" "%INPUT%" -o "%OUTPUT_O%" >nul
if errorlevel 1 exit /b %errorlevel%

call "%ROOT%run-analyzer.bat" "%INPUT%" "%OUTPUT%" >nul
if errorlevel 1 exit /b %errorlevel%

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$r = Get-Content '%OUTPUT%' -Raw | ConvertFrom-Json;" ^
  "$ro = Get-Content '%OUTPUT_O%' -Raw | ConvertFrom-Json;" ^
  "if ($r.topology.solid -ne 1) { throw 'solid count mismatch' };" ^
  "if ($r.topology.face -ne 10) { throw 'face count mismatch' };" ^
  "if ($r.curves.circle -ne 20) { throw 'circle count mismatch' };" ^
  "if ($r.surfaces.torus -ne 3) { throw 'torus count mismatch' };" ^
  "if ($null -eq $r.metrics) { throw 'missing metrics' };" ^
  "if ($null -eq $r.metrics.bounding_box) { throw 'missing bounding_box' };" ^
  "if ($r.metrics.bounding_box.dx -le 0) { throw 'invalid bounding box dx' };" ^
  "if ($r.metrics.bounding_box.dy -le 0) { throw 'invalid bounding box dy' };" ^
  "if ($r.metrics.bounding_box.dz -le 0) { throw 'invalid bounding box dz' };" ^
  "if ($null -eq $r.metrics.bounding_box.center) { throw 'missing bounding_box center' };" ^
  "if ($r.metrics.bounding_box.diagonal -le 0) { throw 'invalid bounding box diagonal' };" ^
  "if ($r.metrics.surface_area -le 0) { throw 'invalid surface area' };" ^
  "if ($r.metrics.volume -le 0) { throw 'invalid volume' };" ^
  "if ($null -eq $r.metrics.center_of_mass) { throw 'missing center_of_mass' };" ^
  "if ($ro.topology.face -ne $r.topology.face) { throw '-o output mismatch' };" ^
  "Write-Output 'OK: screw.step report matches expected counts, metrics, and CLI options'"
