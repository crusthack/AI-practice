@echo off
setlocal

if "%~1"=="" (
    echo Usage: %~nx0 SOURCE_FOLDER DESTINATION_FOLDER
    exit /b 2
)

if "%~2"=="" (
    echo Usage: %~nx0 SOURCE_FOLDER DESTINATION_FOLDER
    exit /b 2
)

set "SOURCE=%~1"
set "DEST=%~2"

if not exist "%SOURCE%\" (
    echo Source folder does not exist: "%SOURCE%"
    exit /b 1
)

if not exist "%DEST%\" (
    mkdir "%DEST%"
    if errorlevel 1 (
        echo Failed to create destination: "%DEST%"
        exit /b 1
    )
)

echo Copying "%SOURCE%" to "%DEST%"
xcopy "%SOURCE%" "%DEST%" /E /I /Y
if errorlevel 1 (
    echo Backup failed.
    exit /b 1
)

echo Backup complete.
endlocal

