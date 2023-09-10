@echo off
title Building FramelessHelper ...
setlocal
cls
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0"
if exist cmake rd /s /q cmake
md cmake
cd cmake
md build
cd build
cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64" -DCMAKE_INSTALL_PREFIX="%~dp0cmake\install" -DCMAKE_CONFIGURATION_TYPES=Release;Debug -G"Ninja Multi-Config" -DFRAMELESSHELPER_ENABLE_VCLTL=ON -DFRAMELESSHELPER_ENABLE_YYTHUNKS=ON -DFRAMELESSHELPER_ENABLE_SPECTRE=ON -DFRAMELESSHELPER_ENABLE_EHCONTGUARD=ON -DFRAMELESSHELPER_ENABLE_INTELCET=ON -DFRAMELESSHELPER_ENABLE_INTELJCC=ON -DFRAMELESSHELPER_ENABLE_CFGUARD=ON -DFRAMELESSHELPER_FORCE_LTO=ON "%~dp0.."
cmake --build . --target all --config Release --parallel
cmake --build . --target all --config Debug --parallel
cmake --install . --config Release --strip
cmake --install . --config Debug
endlocal
cd /d "%~dp0"
pause
exit /b 0
