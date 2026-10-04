@echo off
rem Usage: [set COWS_EACP=<eacp checkout>] [set COWS_ARCH=arm64^|x64] tools\build-windows.bat [Release^|Debug]
rem Release: the x64 exe the stores ship, in build-windows\ (static C runtime,
rem no tests). Debug: build\, for this machine's own architecture (arm64 or x64;
rem COWS_ARCH overrides), with the tests. Both configure with the newest Visual
rem Studio's tools (native on Arm machines, cross-compiling x64 there) and Ninja.
rem COWS_EACP is optional (CPM fetches eacp otherwise).
setlocal
set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release
set ROOT=%~dp0..

set HOST=x64
if not "%ProgramFiles(Arm)%"=="" set HOST=arm64

if /i "%CONFIG%"=="Debug" (
    set BUILD=%ROOT%\build
    set ARCH=%HOST%
) else (
    set BUILD=%ROOT%\build-windows
    set ARCH=x64
)
if not "%COWS_ARCH%"=="" set ARCH=%COWS_ARCH%

set VCARCH=%ARCH%
if /i not "%HOST%"=="%ARCH%" set VCARCH=%HOST%_%ARCH%

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`) do set VSPATH=%%i
if not defined VSPATH echo vswhere found no Visual Studio & exit /b 1
call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" %VCARCH% >nul 2>nul || (
    echo vcvarsall.bat %VCARCH% failed: are the %ARCH% build tools installed?
    exit /b 1
)

set EACP=
if not "%COWS_EACP%"=="" set EACP=-DCPM_eacp_SOURCE=%COWS_EACP%

rem Release links the C runtime statically, so the exe needs no VC++ redist.
set CRT=
if /i "%CONFIG%"=="Release" set CRT=-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCOWS_BUILD_TESTS=OFF

cmake -G Ninja -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=%CONFIG% %EACP% %CRT% || exit /b 1
cmake --build "%BUILD%" || exit /b 1
