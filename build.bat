@echo off
title Build Tron Address Hunter

:: Auto-detect Visual Studio environment
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo cl.exe not found, searching for Visual Studio...
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo Error: Visual Studio not found. Please run from "x64 Native Tools Command Prompt".
        pause
        exit /b 1
    )
)

echo.
echo Compiling...
cl /O2 /EHsc /std:c++17 /I "OpenCL/include" Dispatcher.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp /link /OUT:profanity.exe "OpenCL/lib/OpenCL.lib" ws2_32.lib advapi32.lib

if %errorlevel% equ 0 (
    echo.
    echo Build successful: profanity.exe
) else (
    echo.
    echo Build failed!
)
pause
