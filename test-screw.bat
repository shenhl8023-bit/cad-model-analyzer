@echo off
setlocal

set "ROOT=%~dp0"
set "INPUT=D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step"
set "OUTPUT=%ROOT%output\test-screw-report.json"
set "OUTPUT_O=%ROOT%output\test-screw-report-o.json"
set "BATCH_INPUT=%ROOT%output\batch-input"
set "BATCH_OUTPUT=%ROOT%output\batch-report"
set "EXE=%ROOT%build\x64-release\cad_model_analyzer.exe"

if not exist "%ROOT%output" mkdir "%ROOT%output"
if not exist "%BATCH_INPUT%" mkdir "%BATCH_INPUT%"
if exist "%BATCH_OUTPUT%" rmdir /s /q "%BATCH_OUTPUT%"
copy /y "%INPUT%" "%BATCH_INPUT%\screw.step" >nul

"%EXE%" --help >nul
if errorlevel 1 exit /b %errorlevel%

"%EXE%" --version >nul
if errorlevel 1 exit /b %errorlevel%

call "%ROOT%run-analyzer.bat" "%INPUT%" -o "%OUTPUT_O%" >nul
if errorlevel 1 exit /b %errorlevel%

call "%ROOT%run-analyzer.bat" "%INPUT%" "%OUTPUT%" >nul
if errorlevel 1 exit /b %errorlevel%

call "%ROOT%run-analyzer.bat" --batch "%BATCH_INPUT%" -o "%BATCH_OUTPUT%" >nul
if errorlevel 1 exit /b %errorlevel%

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$r = Get-Content '%OUTPUT%' -Raw | ConvertFrom-Json;" ^
  "$ro = Get-Content '%OUTPUT_O%' -Raw | ConvertFrom-Json;" ^
  "if (!(Test-Path '%BATCH_OUTPUT%\summary.csv')) { throw 'missing batch summary.csv' };" ^
  "if (!(Test-Path '%BATCH_OUTPUT%\screw.json')) { throw 'missing batch screw.json' };" ^
  "$rb = Get-Content '%BATCH_OUTPUT%\screw.json' -Raw | ConvertFrom-Json;" ^
  "$summary = Get-Content '%BATCH_OUTPUT%\summary.csv' -Raw;" ^
  "if ($summary -notmatch 'file,status,solid,face,edge,free_edge,non_manifold_edge,volume,surface_area') { throw 'summary header mismatch' };" ^
  "if ($summary -notmatch 'screw.step,ok,1,10,44,0,0') { throw 'summary row mismatch' };" ^
  "if ($null -eq $r.metadata) { throw 'missing metadata' };" ^
  "if ($r.metadata.version -ne '0.3.0') { throw 'version mismatch' };" ^
  "if ($r.metadata.analysis_time_ms -lt 0) { throw 'invalid analysis time' };" ^
  "if ($r.topology.solid -ne 1) { throw 'solid count mismatch' };" ^
  "if ($r.topology.face -ne 10) { throw 'face count mismatch' };" ^
  "if ($r.topology.free_edge -ne 0) { throw 'free edge count mismatch' };" ^
  "if ($r.topology.non_manifold_edge -ne 0) { throw 'non-manifold edge count mismatch' };" ^
  "if ($r.topology.euler_characteristic -ne 54) { throw 'euler characteristic mismatch' };" ^
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
  "if ($null -eq $r.quality) { throw 'missing quality' };" ^
  "if ($r.quality.closed_solid_candidate -ne $true) { throw 'closed solid candidate mismatch' };" ^
  "if ($r.quality.has_free_edges -ne $false) { throw 'free edge quality mismatch' };" ^
  "if ($r.quality.has_non_manifold_edges -ne $false) { throw 'non-manifold quality mismatch' };" ^
  "if ($r.quality.has_positive_volume -ne $true) { throw 'positive volume quality mismatch' };" ^
  "if ($ro.topology.face -ne $r.topology.face) { throw '-o output mismatch' };" ^
  "if ($rb.topology.face -ne $r.topology.face) { throw 'batch json mismatch' };" ^
  "Write-Output 'OK: screw.step report matches expected counts, metrics, and CLI options'"
