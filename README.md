# FramelessHelper

## Usage

```cpp
// include other files ...
#include "framelesshelper.h"
// include other files ...

// Anywhere you like, we use the main function here.
int main(int argc, char *argv[]) {
    // ...
    QWidget widget;
    FramelessHelper helper;
    // Do this before the widget is shown.
    // Only QWidget and QWindow are supported.
    helper.setFramelessWindows({&widget});
    widget.show();
    // ...
}
```

Notes

- The `setFramelessWindows` function must not be called after the widget or window is shown.
- I use `startSystemMove` and `startSystemResize` which are only available since Qt 5.15 for moving and resizing frameless windows on UNIX platforms, so if your Qt version is below that, you can't compile this code. I'm sorry for it but using the two functions is the easiest way to achieve this.
- Any widgets (or Qt Quick elements) in the titlebar area will not be resposible because the mouse events are intercepted. Try if `setIgnoreAreas` and `setDraggableAreas` help.
- Only top level windows can be frameless. Applying this code to child windows or widgets will result in unexpected behavior.
- If you want to use your own border width, border height, titlebar height or minimum window size, just use the original numbers, no need to scale them according to DPI, this code will do the scaling automatically.

## Features

- Frameless but have frame shadow.
- Drag and resize.
- High DPI scaling.
- Multi-monitor support (different resolution and DPI).
- Windows: act like a normal window, such as have animations when minimizing and maximizing, support tile windows, etc ...

## Tested Platforms

- Windows 7 ~ 10
- Should work on X11, Wayland and macOS, but not tested.

## For Windows developers

- Don't change the window flags (for example, enable the Qt::FramelessWindowHint flag) because it will break the functionality of this code. I'll get rid of the window frame (including the titlebar of course) in Win32 native events.
- All traditional Win32 APIs are replaced by their DPI-aware ones, if there is one.
- Start from Windows 10, normal windows usually have a one pixel width border line, I don't add it because not everyone like it. You can draw one manually if you really need it.
- The frame shadow will get lost if the window is totally transparent. It can't be solved unless you draw the frame shadow manually.
- On Windows 7, if you disabled the Windows Aero, the frame shadow will be disabled as well because it's DWM's resposibility to draw the frame shadow.
- The border width (8 if not scaled), border height (8 if not scaled) and titlebar height (30 if not scaled) are acquired by Win32 APIs and are the same with other standard windows, and thus you should not modify them.
- You can also copy all the code to `[virtual protected] bool QWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)` or `[virtual protected] bool QWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)`, it's the same with install a native event filter to the application.

## References

- <https://github.com/rossy/borderless-window>
- <https://github.com/Bringer-of-Light/Qt-Nice-Frameless-Window>
- <https://github.com/dfct/TrueFramelessWindow>
- <https://github.com/qtdevs/FramelessHelper>
- <https://docs.microsoft.com/en-us/archive/blogs/wpfsdk/custom-window-chrome-in-wpf>

## Special Thanks

Thanks **Lucas** for testing this code in many various conditions.
