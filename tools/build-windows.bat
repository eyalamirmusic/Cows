@echo off
rem Usage: [set COWS_EACP=<eacp checkout>] tools\build-windows.bat [Release^|Debug]
rem Configures and builds build-windows\ (Release) or build-windows-debug\ (Debug)
rem with the newest Visual Studio's x64 tools and Ninja. COWS_EACP is optional
rem (CPM fetches eacp otherwise).
setlocal
set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release
set ROOT=%~dp0..
set BUILD=%ROOT%\build-windows
if /i "%CONFIG%"=="Debug" set BUILD=%ROOT%\build-windows-debug

if not defined VSCMD_VER (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`) do set VSPATH=%%i
)
if not defined VSCMD_VER (
    call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
)

set EACP=
if defined COWS_EACP set EACP=-DCPM_eacp_SOURCE=%COWS_EACP%

rem Release links the C runtime statically, so the exe needs no VC++ redist.
set CRT=
if /i "%CONFIG%"=="Release" set CRT=-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCOWS_BUILD_TESTS=OFF

cmake -G Ninja -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=%CONFIG% %EACP% %CRT% || exit /b 1
cmake --build "%BUILD%" || exit /b 1
