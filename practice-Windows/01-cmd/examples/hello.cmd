@echo off
setlocal

echo Hello from a Windows batch file.
echo.
echo Script path: %~f0
echo Script folder: %~dp0
echo Current folder: %CD%
echo First argument: %~1
echo User profile: %USERPROFILE%
echo Computer name: %COMPUTERNAME%

endlocal

