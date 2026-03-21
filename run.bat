@echo off
chcp 65001 >nul
title Tron Address Hunter

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

:: Timestamp filename
set TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%-%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
set OUTFILE=result-%TIMESTAMP%.txt

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
