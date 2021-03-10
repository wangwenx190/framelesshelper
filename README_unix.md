# FramelessHelper (UNIX version)

## Screenshots

TODO

## Features

- Frameless (of course!).
- Draggable.
- Resizable.
- Cross-platform: Windows, X11, Wayland, macOS (however, it doesn't look good on Windows, so you may want to try [`FramelessHelperWin`](/framelesshelper_win32.h) instead).
- Support blur effect inspired by [Microsoft's Acrylic blur](https://docs.microsoft.com/en-us/windows/uwp/design/style/acrylic).

As for the frame shadow and other window features, they mainly depend on your window manager.

## Usage

```cpp
QWidget widget;
FramelessHelper helper;
helper.removeWindowFrame(&widget);
widget.show();
```

### Ignore areas and etc

```cpp
helper.setIgnoreAreas(&widget, {{0, 0, 30, 40}, {40, 0, 30, 40}});
```

## Requirements

| Component | Requirement | Additional Information |
| --- | --- | --- |
| Qt | >= 5.15 | This code uses two functions, [`startSystemMove`](https://doc.qt.io/qt-5/qwindow.html#startSystemMove) and [`startSystemResize`](https://doc.qt.io/qt-5/qwindow.html#startSystemResize), which are introduced in Qt 5.15 |
| Compiler | >= C++11 | MSVC, MinGW, Clang-CL, Intel-CL / GCC, Clang, ICC are all supported |

## References for developers

- <https://doc.qt.io/qt-5/qobject.html#installEventFilter>
- <https://doc.qt.io/qt-5/qobject.html#removeEventFilter>
- <https://doc.qt.io/qt-5/qwindow.html#startSystemMove>
- <https://doc.qt.io/qt-5/qwindow.html#startSystemResize>

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
