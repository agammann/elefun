@echo off
cd /d "%~dp0"
if not exist "Elefun.exe" (
  echo Please build Elefun.exe first using build.ps1.
  pause
  exit /b 1
)
start "" "%~dp0Elefun.exe"
