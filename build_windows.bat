@echo off
REM ============================================================
REM  poop3D Compiler — Windows Build Script
REM  Prerequisites: Visual Studio 2022, CMake, vcpkg
REM ============================================================

echo.
echo ==========================================
echo   poop3D Compiler ^— Windows Build
echo ==========================================
echo.

SET SCRIPT_DIR=%~dp0
SET BUILD_DIR=%SCRIPT_DIR%build_win

REM Check CMake
where cmake >nul 2>&1
IF ERRORLEVEL 1 (
    echo ERROR: cmake not found in PATH
    echo Install from https://cmake.org/download/
    pause
    exit /b 1
)

REM Check deps
IF NOT EXIST "%SCRIPT_DIR%src\imgui\imgui.h" (
    echo ERROR: Dependencies missing. Run setup.bat first!
    pause
    exit /b 1
)

IF NOT EXIST "%SCRIPT_DIR%src\glad\glad.c" (
    echo ERROR: GLAD missing. Run setup.bat first!
    pause
    exit /b 1
)

mkdir "%BUILD_DIR%" 2>nul
cd /d "%BUILD_DIR%"

REM Try vcpkg toolchain if available
SET VCPKG_ROOT=%USERPROFILE%\vcpkg
IF EXIST "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo Using vcpkg at %VCPKG_ROOT%
    cmake "%SCRIPT_DIR%" ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
        -DVCPKG_TARGET_TRIPLET=x64-windows
) ELSE (
    echo vcpkg not found at %VCPKG_ROOT%, trying without...
    cmake "%SCRIPT_DIR%" -DCMAKE_BUILD_TYPE=Release
)

IF ERRORLEVEL 1 (
    echo CMake configure FAILED
    pause
    exit /b 1
)

cmake --build . --config Release -- /m

IF ERRORLEVEL 1 (
    echo Build FAILED
    pause
    exit /b 1
)

echo.
echo ==========================================
echo   Build SUCCESS!
echo   Binary: %BUILD_DIR%\bin\Release\poop3D.exe
echo ==========================================
pause
