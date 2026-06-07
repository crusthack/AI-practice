@echo off
setlocal

set "ROOT=%~dp0..\.."
pushd "%ROOT%" >nul

if not exist "sandbox\cmd" mkdir "sandbox\cmd"

set "OUT=sandbox\cmd\system-report.txt"

echo Windows System Report > "%OUT%"
echo Generated: %DATE% %TIME% >> "%OUT%"
echo Current folder: %CD% >> "%OUT%"
echo. >> "%OUT%"

echo [whoami] >> "%OUT%"
whoami >> "%OUT%" 2>&1
echo. >> "%OUT%"

echo [ver] >> "%OUT%"
ver >> "%OUT%" 2>&1
echo. >> "%OUT%"

echo [where commands] >> "%OUT%"
where cmd >> "%OUT%" 2>&1
where powershell >> "%OUT%" 2>&1
where winget >> "%OUT%" 2>&1
echo. >> "%OUT%"

echo [ipconfig] >> "%OUT%"
ipconfig >> "%OUT%" 2>&1
echo. >> "%OUT%"

echo [tasklist first lines] >> "%OUT%"
tasklist >> "%OUT%" 2>&1

echo Wrote "%OUT%"

popd >nul
endlocal

