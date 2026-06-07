@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
pushd "%ROOT%" >nul

if not exist "sandbox\cmd" mkdir "sandbox\cmd"

set "OUT=sandbox\cmd\workspace-map.txt"
set "READMES=sandbox\cmd\readme-files.txt"

echo Workspace Map > "%OUT%"
echo Generated: %DATE% %TIME% >> "%OUT%"
echo Root: %CD% >> "%OUT%"
echo. >> "%OUT%"

echo [Command Locations] >> "%OUT%"
where cmd >> "%OUT%" 2>&1
where powershell >> "%OUT%" 2>&1
where winget >> "%OUT%" 2>&1
echo. >> "%OUT%"

echo [Environment] >> "%OUT%"
echo USERPROFILE=%USERPROFILE% >> "%OUT%"
echo PATH=%PATH% >> "%OUT%"
echo. >> "%OUT%"

echo [Directory Tree] >> "%OUT%"
dir /s /b >> "%OUT%"

dir /s /b | findstr /i "readme" > "%READMES%"

echo Wrote %OUT%
echo Wrote %READMES%

popd >nul
endlocal

