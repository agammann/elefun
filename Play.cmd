@echo off
cd /d "%~dp0"
if not exist "build\Elefun.exe" (
  echo Build the source first: see BUILD.md, or download the Windows ZIP from Releases.
  pause
  exit /b 1
)
start "Elefun 1.0.0" "%~dp0build\Elefun.exe"
