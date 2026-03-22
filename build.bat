@echo off
title Build Tron Address Hunter

:: Auto-detect Visual Studio environment
where cl >nul 2>&1
if %errorlevel% equ 0 goto :compile

echo cl.exe not found, searching for Visual Studio...

:: Use vswhere.exe to find any installed VS (Community, Professional, Enterprise, BuildTools)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" (
            echo Found Visual Studio at: %%i
            call "%%i\VC\Auxiliary\Build\vcvars64.bat"
            goto :compile
        )
    )
)

echo Error: Visual Studio with C++ tools not found.
echo Please run from "x64 Native Tools Command Prompt" or install Visual Studio with C++ workload.
pause
exit /b 1

:compile
echo.
echo Compiling...
cl /O2 /EHsc /std:c++17 /I "OpenCL/include" /I "third_party" src\Dispatcher.cpp src\Mode.cpp src\precomp.cpp src\profanity.cpp src\SpeedSample.cpp third_party\uECC.c /link /OUT:profanity.exe "OpenCL/lib/OpenCL.lib" ws2_32.lib advapi32.lib

if %errorlevel% equ 0 (
    echo.
    echo Build successful: profanity.exe
) else (
    echo.
    echo Build failed!
)
pause
