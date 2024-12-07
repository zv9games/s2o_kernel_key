@echo off
setlocal

:: Ensure environment variables are set correctly
echo %INCLUDE%
echo %LIB%

:: Add exact WDF header path to INCLUDE
set INCLUDE=E:\Program Files\Windows Kits\10\Include\wdf\kmdf\1.15;%INCLUDE%

:: Set essential include paths
set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\km
set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared
set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um
set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\wdf\kmdf
set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt
set INCLUDE=%INCLUDE%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.42.34433\include
set INCLUDE=%INCLUDE%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\include

:: Set essential library paths
set LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\km\x64
set LIB=%LIB%;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64
set LIB=%LIB%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.42.34433\lib\x64
set LIB=%LIB%;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64

:: Navigate to project directory and confirm
cd /d C:\S2O\s2o_kernel_key
if not "%cd%" == "C:\S2O\s2o_kernel_key" (
    echo "Failed to navigate to project directory"
    pause
    exit /b 1
)

:: Run nmake to build the project
nmake /f Makefile
if errorlevel 1 (
    echo "Error running nmake"
    pause
    exit /b 1
)

:: Keep the command prompt open
cmd /k
endlocal
