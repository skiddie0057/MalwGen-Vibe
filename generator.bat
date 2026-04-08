@echo off
setlocal enabledelayedexpansion

set "PROJECT_ROOT=%~dp0.."

echo ========================================
echo         AV Testing Malw Generator
echo ========================================
echo.
echo  FOR TESTING PURPOSES ONLY
echo  Use in isolated environments only
echo ========================================
echo.

:menu
echo.
echo === Malw Generator Menu ===
echo.
echo MALWARE VARIANTS:
echo   1) Simple
echo      - Keylogger (console capture)
echo      - HTTPS C2 connection
echo      - XOR string obfuscation
echo      - PRNG-based delays
echo.
echo   2) Medium
echo      - Enhanced keylogger
echo      - Encrypted C2 communication
echo      - Polymorphic encoding
echo      - Unique build ID per compilation
echo.
echo   3) Complex
echo      - Advanced keylogger
echo      - Multi-layer obfuscation (PRNG, polymorphic)
echo      - HTTPS C2 with upload/download
echo      - System info exfiltration
echo      - Session-based identification
echo.
echo COMPILATION:
echo   4) Compile ALL (Linux)
echo   5) Compile ALL (Windows)
echo.
echo   0) Exit
echo.
set /p choice="Select option: "

if "%choice%"=="1" goto :simple
if "%choice%"=="2" goto :medium
if "%choice%"=="3" goto :complex
if "%choice%"=="4" goto :compile_linux
if "%choice%"=="5" goto :compile_windows
if "%choice%"=="0" goto :exit

echo [!] Invalid option
goto :menu

:simple
echo.
echo [*] Simple Variant
echo     Location: %PROJECT_ROOT%\malw\simple\simple_malw.c
echo.
echo     Features:
echo     - Console keylogger capture
echo     - HTTPS C2 to example.com
echo     - XOR obfuscated strings
echo     - Random delays via PRNG
goto :menu

:medium
echo.
echo [*] Medium Variant
echo     Location: %PROJECT_ROOT%\malw\medium\medium_malw.c
echo.
echo     Features:
echo     - Enhanced keylogger
echo     - Multi-layer packet encoding
echo     - Unique build ID per compilation
echo     - Randomized encryption keys
goto :menu

:complex
echo.
echo [*] Complex Variant
echo     Location: %PROJECT_ROOT%\malw\complex\complex_malw.c
echo.
echo     Features:
echo     - Advanced keylogger with evasion
echo     - Xorshift96 PRNG anti-analysis
echo     - HTTPS C2 with full upload/download
echo     - System reconnaissance
echo     - Polymorphic transformations
goto :menu

:compile_linux
echo.
echo [*] Linux compilation script ready at:
echo     %PROJECT_ROOT%\compilers\compile_linux.sh
echo.
echo     Run this on a Linux machine with:
echo     chmod +x compilers^/compile_linux.sh
echo     .^/compilers^/compile_linux.sh
goto :menu

:compile_windows
echo.
echo [*] Running Windows compilation...
echo.
call "%PROJECT_ROOT%\compilers\compile_windows.bat"
goto :menu

:exit
echo.
echo [*] Goodbye!
endlocal
exit /b 0
