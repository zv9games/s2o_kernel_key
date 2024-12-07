@echo off
setlocal

:: Call Visual Studio environment setup
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 (
    echo "Error initializing Visual Studio environment"
    pause
    exit /b 1
)

:: Call additional environment setup script and check for errors
call "E:\LaunchBuildEnv.cmd"
if errorlevel 1 (
    echo "Error running LaunchBuildEnv.cmd"
    pause
    exit /b 1
)

:: Add exact WDF header path to INCLUDE
set INCLUDE=E:\Program Files\Windows Kits\10\Include\wdf\kmdf\1.15;%INCLUDE%

:: Output the INCLUDE and LIB paths for verification
echo %INCLUDE%
echo %LIB%

pause
endlocal
