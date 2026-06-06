@echo off
setlocal

if "%~1"=="" (
    echo Usage: %~nx0 SEARCH_TEXT [FOLDER]
    exit /b 2
)

set "TEXT=%~1"
set "FOLDER=%~2"

if "%FOLDER%"=="" set "FOLDER=."

if not exist "%FOLDER%\" (
    echo Folder does not exist: "%FOLDER%"
    exit /b 1
)

echo Searching for "%TEXT%" under "%FOLDER%"
findstr /S /I /N /C:"%TEXT%" "%FOLDER%\*.*"

if errorlevel 1 (
    echo No matches found or findstr failed.
    exit /b 1
)

endlocal

