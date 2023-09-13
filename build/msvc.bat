@echo off
title Building FramelessHelper ...
setlocal
cls
set __vs_bat=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%__vs_bat%" set __vs_bat=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%__vs_bat%" set __vs_bat=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%__vs_bat%" set __vs_bat=%ProgramFiles(x86)%\Microsoft Visual Studio\2015\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%__vs_bat%" (
    echo Cannot find a valid Visual Studio toolchain!
    echo Please install at least Visual Studio 2015 to the default location!
    goto fin
)
call "%__vs_bat%"
cmake --version
echo ninja build
ninja --version
cd /d "%~dp0"
if exist "%~dp0build.user.bat" call "%~dp0build.user.bat"
if not defined QTDIR set QTDIR=C:\Qt\6.6.0\msvc2019_64
echo QTDIR=%QTDIR%
if exist cmake rd /s /q cmake
md cmake
cd cmake
md build
cd build
cmake -DCMAKE_PREFIX_PATH="%QTDIR%" -DCMAKE_INSTALL_PREFIX="%~dp0cmake\install" -DCMAKE_CONFIGURATION_TYPES=Release;Debug -G"Ninja Multi-Config" -DFRAMELESSHELPER_ENABLE_VCLTL=ON -DFRAMELESSHELPER_ENABLE_YYTHUNKS=ON -DFRAMELESSHELPER_ENABLE_SPECTRE=ON -DFRAMELESSHELPER_ENABLE_EHCONTGUARD=ON -DFRAMELESSHELPER_ENABLE_INTELCET=ON -DFRAMELESSHELPER_ENABLE_INTELJCC=ON -DFRAMELESSHELPER_ENABLE_CFGUARD=ON -DFRAMELESSHELPER_FORCE_LTO=ON "%~dp0.."
cmake --build . --target all --config Release --parallel
cmake --build . --target all --config Debug --parallel
cmake --install . --config Release --strip
cmake --install . --config Debug
goto fin
:fin
endlocal
cd /d "%~dp0"
pause
exit /b 0
