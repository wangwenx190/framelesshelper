# FramelessHelper

If you are using part of or all the code from this repository in your own projects, it's my pleasure and I'm happy that it can help you. But I hope you can tell me the URL of the homepage or repository of your projects, whether your projects are close-sourced or commercial products do not matter. I'll link to your homepage or repository in this README file. It would be much better if you can provide me some screenshots of your software for demonstration.

如果您正在使用此项目的部分或全部代码，这是我的荣幸，我很高兴能帮到您，但我同时也希望，您能将您项目的首页或仓库的网址告诉我（闭源或收费都没关系），我将在这个自述文件中链接到您所提供的网址，以供展示。如果您能一并提供一些软件运行时的截图，那就更好了。

## Screenshots

![Quick_Acrylic_Win7](/screenshots/quick_acrylic_win7.png)

## Features

- Support Windows, X11, Wayland and macOS.
- Frameless but have frame shadow.
- Draggable and resizable.
- Automatically high DPI scaling.
- Multi-monitor support (different resolution and DPI).
- Have animations when minimizing, maximizing and restoring.
- Support tiled and stack windows by DWM (Win32 only).
- Won't cover the task bar when maximized (Win32 only).
- Won't block the auto-hide task bar when maximized (Win32 only).
- Support cross-platform blur effect inspired by [Microsoft's Acrylic blur](https://docs.microsoft.com/en-us/windows/uwp/design/style/acrylic).

## Usage

```cpp
#include "framelesswindowsmanager.h"
#include <QWidget>

int main(int argc, char *argv[]) {
    QWidget widget;
    // Qt's internal function. Make sure it's a top level window.
    widget.createWinId();
    // Do this before the widget is shown.
    FramelessWindowsManager::addWindow(widget.windowHandle());
    widget.show();
}
```

Please refer to [the QWidget example](/examples/QWidget/main.cpp) for more detailed information.

### Ignore areas and etc

```cpp
// Only **TOP LEVEL** QWidgets and QWindows are supported.
QMainWindow *mainWindow = new QMainWindow;
// Qt's internal function. Make sure it's a top level window.
mainWindow->createWinId();
QWindow *win = mainWindow->windowHandle();
// Disable resizing of the given window. Resizing is enabled by default.
FramelessWindowsManager::setResizable(win, false);
// All the following values should not be DPI-aware, just use the
// original numbers, assuming the scale factor is 1.0, don't scale
// them yourself, this code will do the scaling according to DPI
// internally and automatically.
// Maximum window size
win->setMaximumSize(1280, 720);
// Minimum window size
win->setMinimumSize(800, 540);
// The **POINTER** of a QWidget or QQuickItem
FramelessWindowsManager::addIgnoreObject(win, ui->pushButton_minimize);
```

## Supported Platforms

### Win32

Windows 7 ~ 10, 32 bit & 64 bit.

The code itself should be able to work on Windows Vista in theory, but Qt5 has dropped Vista support long time ago. And Qt6 will only support Win10.

### UNIX

A not too old version of Linux and macOS, 32 bit & 64 bit.

## Requirements

### Win32

| Component | Requirement | Additional Information |
| --- | --- | --- |
| Qt | >= 5.6 | Only the `core` and `gui` modules are required |
| Compiler | >= C++11 | MSVC, MinGW, Clang-CL, Intel-CL or cross compile from Linux/macOS are all supported |

### UNIX

| Component | Requirement | Additional Information |
| --- | --- | --- |
| Qt | >= 5.15 | This code uses two functions, [`startSystemMove`](https://doc.qt.io/qt-5/qwindow.html#startSystemMove) and [`startSystemResize`](https://doc.qt.io/qt-5/qwindow.html#startSystemResize), which are introduced in Qt 5.15 |
| Compiler | >= C++11 | MSVC, MinGW, Clang-CL, Intel-CL / GCC, Clang, ICC are all supported |

## Known Bugs

Please refer to <https://github.com/wangwenx190/framelesshelper/issues> for more information.

## License

```text
MIT License

Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
