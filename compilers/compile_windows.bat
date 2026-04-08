@echo off
setlocal enabledelayedexpansion

set "PROJECT_ROOT=%~dp0.."
set "OUTPUT_DIR=%PROJECT_ROOT%\output\windows"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo ========================================
echo Malw Generator - Windows Compiler
echo ========================================
echo.

where cl >nul 2>nul
if errorlevel 1 (
    echo [ERROR] MSVC compiler ^(cl.exe^) not found.
    echo         Ensure Visual Studio is installed and PATH is set.
    exit /b 1
)
echo [OK] MSVC compiler found

where upx >nul 2>nul
if errorlevel 1 (
    set "UPX_AVAILABLE=0"
    echo [--] UPX not found ^(optional - binaries will not be packed^)
) else (
    set "UPX_AVAILABLE=1"
    for /f "tokens=*" %%i in ('upx --version 2^>^&1 ^| findstr /i "upx"') do echo [OK] UPX found: %%i
)
echo.

set "CFLAGS=/O2 /GS- /D_UNICODE /DUNICODE /I%PROJECT_ROOT%"
set "LDFLAGS=winhttp.lib ws2_32.lib"

echo --- Compiling ---
echo.

echo [1/3] Building simple_malw...
cl %CFLAGS% ^
    "%PROJECT_ROOT%\malw\simple\simple_malw.c" ^
    /Fe"%OUTPUT_DIR%\simple_malw.exe" ^
    %LDFLAGS%
echo   -^> simple_malw.exe built

if "!UPX_AVAILABLE!"=="1" (
    copy "%OUTPUT_DIR%\simple_malw.exe" "%OUTPUT_DIR%\simple_malw.packed.exe" >nul
    upx --best "%OUTPUT_DIR%\simple_malw.packed.exe" >nul 2>nul
    echo   -^> simple_malw.packed.exe built
)

echo.
echo [2/3] Building medium_malw...
cl %CFLAGS% ^
    "%PROJECT_ROOT%\malw\medium\medium_malw.c" ^
    /Fe"%OUTPUT_DIR%\medium_malw.exe" ^
    %LDFLAGS%
echo   -^> medium_malw.exe built

if "!UPX_AVAILABLE!"=="1" (
    copy "%OUTPUT_DIR%\medium_malw.exe" "%OUTPUT_DIR%\medium_malw.packed.exe" >nul
    upx --best "%OUTPUT_DIR%\medium_malw.packed.exe" >nul 2>nul
    echo   -^> medium_malw.packed.exe built
)

echo.
echo [3/3] Building complex_malw...
cl %CFLAGS% ^
    "%PROJECT_ROOT%\malw\complex\complex_malw.c" ^
    /Fe"%OUTPUT_DIR%\complex_malw.exe" ^
    %LDFLAGS%
echo   -^> complex_malw.exe built

if "!UPX_AVAILABLE!"=="1" (
    copy "%OUTPUT_DIR%\complex_malw.exe" "%OUTPUT_DIR%\complex_malw.packed.exe" >nul
    upx --best "%OUTPUT_DIR%\complex_malw.packed.exe" >nul 2>nul
    echo   -^> complex_malw.packed.exe built
)

echo.
echo ========================================
echo Build Complete
echo ========================================
dir /b "%OUTPUT_DIR%\*.exe"

endlocal
