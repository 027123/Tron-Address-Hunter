@echo off
chcp 65001 >nul
title Tron Address Hunter

:: Auto-detect script directory, works anywhere
cd /d "%~dp0"

:: Create result directory
if not exist "result" mkdir result

echo ============================================
echo        Tron Address Hunter
echo ============================================
echo.

:: Prefix count
set PREFIX=0
set /p PREFIX="Match prefix count (0=disabled, 0-10) [default: 0]: "
if "%PREFIX%"=="" set PREFIX=0

:: Suffix count
set SUFFIX=6
set /p SUFFIX="Match suffix count (0-10) [default: 6]: "
if "%SUFFIX%"=="" set SUFFIX=6

:: Quit count
set QUIT=1
set /p QUIT="How many addresses to find (1-10000) [default: 1]: "
if "%QUIT%"=="" set QUIT=1

:: Timestamp filename (locale-safe)
for /f %%i in ('powershell -noprofile -command "Get-Date -Format yyyyMMdd-HHmmss"') do set TIMESTAMP=%%i
set OUTFILE=result\result-%TIMESTAMP%.txt

:: Count valid patterns (non-empty, non-comment lines) in profanity.txt
set PATCOUNT=0
if exist profanity.txt (
    for /f "usebackq eol=# delims=" %%a in ("profanity.txt") do set /a PATCOUNT+=1
)

echo.
echo ============================================
echo  Prefix:   %PREFIX%
echo  Suffix:   %SUFFIX%
echo  Find:     %QUIT% address(es)
echo  Patterns: %PATCOUNT%
echo  Output:   %OUTFILE%
echo ============================================
echo.

if "%PREFIX%"=="0" (
    profanity.exe --matching profanity.txt --suffix-count %SUFFIX% --quit-count %QUIT% -o %OUTFILE%
) else (
    profanity.exe --matching profanity.txt --prefix-count %PREFIX% --suffix-count %SUFFIX% --quit-count %QUIT% -o %OUTFILE%
)

echo.
echo Done! Results saved to %OUTFILE%
pause
