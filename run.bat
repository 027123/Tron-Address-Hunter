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
set /p PREFIX="Match prefix count (0=disabled, 0-12) [default: 0]: "
if "%PREFIX%"=="" set PREFIX=0

:: Suffix count
set SUFFIX=6
set /p SUFFIX="Match suffix count (4-12) [default: 6]: "
if "%SUFFIX%"=="" set SUFFIX=6

:: Quit count
set QUIT=1
set /p QUIT="How many addresses to find (1-10000) [default: 1]: "
if "%QUIT%"=="" set QUIT=1

:: Timestamp filename (locale-safe)
for /f "tokens=2 delims==" %%i in ('wmic os get localdatetime /value') do set DT=%%i
set TIMESTAMP=%DT:~0,8%-%DT:~8,6%
set OUTFILE=result\result-%TIMESTAMP%.txt

echo.
echo ============================================
echo  Prefix: %PREFIX%
echo  Suffix: %SUFFIX%
echo  Find:   %QUIT% address(es)
echo  Output: %OUTFILE%
echo ============================================
echo.

if "%PREFIX%"=="0" (
    profanity.exe --matching profanity.txt --suffix-count %SUFFIX% --quit-count %QUIT% --skip 1 -o %OUTFILE%
) else (
    profanity.exe --matching profanity.txt --prefix-count %PREFIX% --suffix-count %SUFFIX% --quit-count %QUIT% --skip 1 -o %OUTFILE%
)

echo.
echo Done! Results saved to %OUTFILE%
pause
