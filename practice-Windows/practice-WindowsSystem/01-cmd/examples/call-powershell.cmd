@echo off
setlocal

set "ROOT=%~dp0..\.."
set "SCRIPT=%ROOT%\02-powershell\Get-WorkspaceInventory.ps1"

if not exist "%SCRIPT%" (
    echo PowerShell script not found: "%SCRIPT%"
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%"
if errorlevel 1 (
    echo PowerShell script failed.
    exit /b 1
)

endlocal

