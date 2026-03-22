@echo off
chcp 65001 >nul
title Tron Address Hunter - Benchmark
cd /d "%~dp0"

echo ============================================================
echo   Tron Address Hunter - Worksize Benchmark
echo ============================================================
echo.
echo   Each config runs for ~10 found addresses (suffix-count=5).
echo   Results are saved to bench_results.txt
echo.
echo   Parameters tested:
echo     work (local worksize):  64, 128, 256
echo     inverse-size:           64, 128, 256
echo     inverse-multiple:       16384, 32768, 65536
echo ============================================================
echo.

:: Ensure we have a matching file
if not exist "dist\profanity.txt" (
    echo Error: dist\profanity.txt not found!
    pause
    exit /b 1
)

:: Clean previous results
if exist bench_results.txt del bench_results.txt

echo work,inverse-size,inverse-multiple,speed> bench_results.txt

set COUNT=0

for %%w in (64 128 256) do (
    for %%i in (64 128 256) do (
        for %%I in (16384 32768 65536) do (
            set /a COUNT+=1
            echo [Test !COUNT!] work=%%w  inverse-size=%%i  inverse-multiple=%%I

            :: Run with quit-count=10 suffix=5 for quick benchmark
            for /f "tokens=2 delims= " %%s in ('dist\profanity.exe --matching dist\profanity.txt --suffix-count 5 --quit-count 10 --no-cache -w %%w -i %%i -I %%I 2^>nul ^| findstr /C:"Speed:"') do (
                echo   Speed: %%s MH/s
                echo %%w,%%i,%%I,%%s>> bench_results.txt
            )
        )
    )
)

echo.
echo ============================================================
echo   Benchmark complete! Results:
echo ============================================================
echo.

:: Display results sorted by speed (descending)
echo work,inverse-size,inverse-multiple,speed
echo --------------------------------------------
type bench_results.txt | findstr /V "work,"

echo.
echo Results saved to bench_results.txt
pause
