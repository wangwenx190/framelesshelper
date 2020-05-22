@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
call "%SystemDrive%\Qt\5.15.0\msvc2019_64\bin\qtenv2.bat"
cd /d "%~dp0"
if exist build rd /s /q build
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=bin -GNinja ..
cmake --build .
windeployqt --force --no-translations --no-system-d3d-compiler --no-virtualkeyboard --no-compiler-runtime --no-angle --no-opengl-sw --dir "%~dp0build\bin\qml" --libdir "%~dp0build\bin" --plugindir "%~dp0build\bin\plugins" --qmldir "%~dp0resources\qml" "%~dp0build\bin\QQPlayer.exe"
cd /d "%~dp0"
start explorer /select,"%~dp0build\bin\QQPlayer.exe"
exit /b
