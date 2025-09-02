@echo off
REM ===========================================
REM Compile.bat for Windows (Visual Studio + CMake) - Debug Mode
REM ===========================================

REM Set build folder
set BUILD_DIR=build

REM Remove previous build folder if exists
if exist %BUILD_DIR% (
    echo Cleaning previous build...
    rmdir /s /q %BUILD_DIR%
)

REM Create build folder
mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure CMake for Visual Studio 2022 (Debug)
echo Configuring project with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..

REM Build Debug configuration
echo Building Debug version...
cmake --build . --config Debug

echo ===========================================
echo Build finished! Executables are in:
echo %CD%\Debug
pause
