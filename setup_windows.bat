@echo off
REM ============================================================
REM  poop3D Compiler — Windows Dependency Setup
REM  Run this ONCE before opening in Visual Studio.
REM ============================================================

echo.
echo ==========================================
echo   poop3D Compiler ^— Windows Setup
echo ==========================================
echo.

SET SCRIPT_DIR=%~dp0
SET SRC=%SCRIPT_DIR%src

REM ─── Step 1: Download ImGui + GLAD ───────────────────────────────────────────
echo [1/2] Downloading ImGui and GLAD...
echo.

REM Try python first, then python3
where python >nul 2>&1
IF NOT ERRORLEVEL 1 (
    python "%SCRIPT_DIR%fetch_deps.py"
    goto :check_python_result
)
where python3 >nul 2>&1
IF NOT ERRORLEVEL 1 (
    python3 "%SCRIPT_DIR%fetch_deps.py"
    goto :check_python_result
)

echo WARNING: Python not found. Trying curl fallback...
goto :curl_fallback

:check_python_result
IF ERRORLEVEL 1 (
    echo.
    echo Python download failed. Trying curl fallback...
    goto :curl_fallback
)
goto :vcpkg_step

:curl_fallback
REM Fallback: download with curl (built into Windows 10+)
where curl >nul 2>&1
IF ERRORLEVEL 1 (
    echo ERROR: Neither Python nor curl found.
    echo.
    echo Please install Python from https://python.org/downloads
    echo Then run:  python fetch_deps.py
    pause
    exit /b 1
)

echo Downloading ImGui via curl...
SET IMGUI_BASE=https://raw.githubusercontent.com/ocornut/imgui/v1.90.5
mkdir "%SRC%\imgui\backends" 2>nul

FOR %%F IN (imgui.h imgui.cpp imgui_internal.h imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imconfig.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h) DO (
    IF NOT EXIST "%SRC%\imgui\%%F" (
        echo   Downloading %%F...
        curl -sL --retry 3 "%IMGUI_BASE%/%%F" -o "%SRC%\imgui\%%F"
    )
)
FOR %%F IN (imgui_impl_glfw.h imgui_impl_glfw.cpp imgui_impl_opengl3.h imgui_impl_opengl3.cpp imgui_impl_opengl3_loader.h) DO (
    IF NOT EXIST "%SRC%\imgui\backends\%%F" (
        echo   Downloading backends/%%F...
        curl -sL --retry 3 "%IMGUI_BASE%/backends/%%F" -o "%SRC%\imgui\backends\%%F"
    )
)

echo   ImGui done.
echo.

REM GLAD is already bundled — skip download
echo   GLAD already bundled in src/glad/

:vcpkg_step
REM ─── Step 2: vcpkg (GLFW + Assimp) ──────────────────────────────────────────
echo.
echo [2/2] Checking vcpkg for GLFW and Assimp...

SET VCPKG_ROOT=%USERPROFILE%\vcpkg
IF NOT EXIST "%VCPKG_ROOT%\vcpkg.exe" (
    where git >nul 2>&1
    IF ERRORLEVEL 1 (
        echo ERROR: git not found. Install from https://git-scm.com
        echo Then re-run this script.
        pause
        exit /b 1
    )
    echo   vcpkg not found. Cloning and bootstrapping...
    git clone https://github.com/Microsoft/vcpkg.git "%VCPKG_ROOT%"
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat" -disableMetrics
)

echo   Installing glfw3 and assimp (x64-windows)...
"%VCPKG_ROOT%\vcpkg.exe" install glfw3:x64-windows assimp:x64-windows

IF ERRORLEVEL 1 (
    echo.
    echo WARNING: vcpkg install failed. You may still be able to build
    echo without Assimp (OBJ import only).
)

echo.
echo ==========================================
echo   Setup complete!
echo.
echo   Now either:
echo     1. Open this folder in Visual Studio 2022
echo        (it will detect CMakeLists.txt automatically)
echo     2. Or run:  build_windows.bat
echo ==========================================
pause
