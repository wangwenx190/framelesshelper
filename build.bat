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
    echo Please install at least Visual Studio 2015 to the default location
    echo and install the English language pack at the same time.
    echo If you want to use clang-cl or MinGW to build this project, please
    echo make sure you have added their directory to your PATH environment
    echo variable.
    echo Press the ENTER key to continue, or close this window directly.
    pause
) else (
    call "%__vs_bat%"
)
cmake --version
echo ninja build
ninja --version
cd /d "%~dp0"
if exist build.user.bat call build.user.bat
if not defined CC set CC=cl.exe
if not defined CXX set CXX=cl.exe
if not defined QTDIR set QTDIR=%SystemDrive%\Qt\6.6.0\msvc2019_64
echo CC=%CC%
echo CXX=%CXX%
echo QTDIR=%QTDIR%
if exist build rd /s /q build
md build
cd build
md cmake
cd cmake
cmake -DCMAKE_C_COMPILER="%CC%" -DCMAKE_CXX_COMPILER="%CXX%" -DCMAKE_PREFIX_PATH="%QTDIR%" -DCMAKE_INSTALL_PREFIX="%~dp0build\install" -DCMAKE_CONFIGURATION_TYPES=Release;Debug -G"Ninja Multi-Config" -DFRAMELESSHELPER_ENABLE_VCLTL=ON -DFRAMELESSHELPER_ENABLE_YYTHUNKS=ON -DFRAMELESSHELPER_ENABLE_SPECTRE=ON -DFRAMELESSHELPER_ENABLE_EHCONTGUARD=ON -DFRAMELESSHELPER_ENABLE_INTELCET=ON -DFRAMELESSHELPER_ENABLE_INTELJCC=ON -DFRAMELESSHELPER_ENABLE_CFGUARD=ON -DFRAMELESSHELPER_FORCE_LTO=ON "%~dp0"
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
