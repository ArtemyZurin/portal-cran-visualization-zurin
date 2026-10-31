@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\Run.ps1"
if errorlevel 1 pause
