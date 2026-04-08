@echo off
setlocal enabledelayedexpansion

set "PROJECT_ROOT=%~dp0.."

set "C2_HOST=example.com"
set "C2_PORT=443"
set "UPLOAD_ENDPOINT=/api/upload"
set "DOWNLOAD_ENDPOINT=/api/download"

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
echo      - Anti-VM detection
echo      - Persistence (registry)
echo.
echo   2) Medium
echo      - Enhanced keylogger
echo      - Encrypted C2 communication
echo      - Polymorphic encoding
echo      - Unique build ID per compilation
echo      - Anti-VM + Persistence
echo.
echo   3) Complex
echo      - Advanced keylogger
echo      - Multi-layer obfuscation (PRNG, polymorphic)
echo      - HTTPS C2 with upload/download
echo      - System info exfiltration
echo      - Session-based identification
echo      - Anti-VM + Persistence
echo.
echo COMPILATION:
echo   4) Compile ALL (Linux)
echo   5) Compile ALL (Windows)
echo   6) Configure C2 URLs
echo.
echo   0) Exit
echo.
set /p choice="Select option: "

if "%choice%"=="1" goto :simple
if "%choice%"=="2" goto :medium
if "%choice%"=="3" goto :complex
if "%choice%"=="4" goto :compile_linux
if "%choice%"=="5" goto :compile_windows
if "%choice%"=="6" goto :configure_urls
if "%choice%"=="0" goto :exit

echo [!] Invalid option
goto :menu

:configure_urls
echo.
echo === C2 Configuration ===
echo.
echo Press ENTER to use defaults or enter custom values:
echo.
set /p C2_HOST="C2 Host (e.g., c2.example.com) [example.com]: "
if "!C2_HOST!"=="" set "C2_HOST=example.com"
set /p C2_PORT="C2 Port [443]: "
if "!C2_PORT!"=="" set "C2_PORT=443"
echo.
echo Advanced endpoints (for medium/complex variants):
set /p UPLOAD_ENDPOINT="Upload endpoint [/api/upload]: "
if "!UPLOAD_ENDPOINT!"=="" set "UPLOAD_ENDPOINT=/api/upload"
set /p DOWNLOAD_ENDPOINT="Download endpoint [/api/download]: "
if "!DOWNLOAD_ENDPOINT!"=="" set "DOWNLOAD_ENDPOINT=/api/download"
echo.
echo [*] Using:
echo     C2 Host: !C2_HOST!
echo     C2 Port: !C2_PORT!
echo     Upload: !UPLOAD_ENDPOINT!
echo     Download: !DOWNLOAD_ENDPOINT!
echo.
goto :menu

:simple
echo.
echo [*] Simple Variant
echo     Location: %PROJECT_ROOT%\malw\simple\simple_malw.c
echo.
echo     Features:
echo     - Console keylogger capture
echo     - HTTPS C2 to !C2_HOST!
echo     - XOR obfuscated strings
echo     - Random delays via PRNG
echo     - Anti-VM detection
echo     - Persistence mechanism
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
echo     - Anti-VM detection
echo     - Persistence mechanism
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
echo     - Anti-VM detection
echo     - Persistence mechanism
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
