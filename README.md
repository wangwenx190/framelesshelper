# FramelessHelper 2.x

## Highlights compared to 2.1 (TODO list)

- Common: Added cross-platform customizable system menu for both Qt Widgets and Qt Quick. Also supports both light and dark theme.
- Common: More configurable options from environment variables and settings file.
- Common: Migrate to categorized logging output.
- Examples: Added QtWebEngine based demo projects for both Qt Widgets and Qt Quick.

## Highlights compared to 2.0

- Windows: Added support for the snap layout feature introduced in Windows 11.
- Widgets: Redesigned the public interface, the use of FramelessHelper is now more elegant.
- Quick: Redesigned the public interface, the use of FramelessHelper is now more elegant.
- Common: Redesigned the standard title bar interface, it's now possible to customize it from outside. Previously there's no standard title bar in the widgets module, now it's added and exported.
- Misc: Removed bundled Qt internal classes that are licensed under Commercial/GPL/LGPL. This library is now pure MIT licensed.
- Bug fixes and internal refactorings.

## Highlights compared to 1.x

- Windows: Gained the ability to only remove the title bar but preserve the window frame at the same time.
- Windows: The flicker and jitter during window resizing is completely gone.
- Windows: The system menu will be opened if you right-click on your custom title bar.
- Windows: Replaced Qt's original system menu with FramelessHelper's homemade one, which looks a lot better than the original one.
- Linux: Removed the limitation of the Qt version. The minimum supported version is lowered to Qt 5.6 now (previously was 5.15).
- macOS: Removed the limitation of the Qt version. The minimum supported version is lowered to Qt 5.6 now (previously was 5.15).
- macOS: The frameless window now supports native resizing.
- Common: Almost completely rewritten of the whole library, it's now a lot more easier to setup your own custom title bar than before.
- Common: Added many more helper functions to allow creating your own custom window easier.
- Misc: Reorganized the project structure to be more like a modern library, it's now a lot more friendly to the library users.
- Misc: Many bugs from the 1.x times are fixed (they were not fixable in 1.x due to technical reasons).

## Screenshots

### Windows

![Light](./doc/win_light.png)

![Dark](./doc/win_dark.png)

### Linux

![Light](./doc/linux_light.png)

![Dark](./doc/linux_dark.png)

### macOS

![Light](./doc/mac_light.png)

![Dark](./doc/mac_dark.png)

## Requiredments

- Compiler: a modern compiler which supports C++17 at least.
- Qt version: using the latest stable version of Qt is highly recommended, the minimum supported version is Qt 5.6.
- Qt modules: QtCore and QtGui for the core module, QtWidgets for the widgets module, QtQml QtQuick QtQuickControls2 QtQuickTemplates2 for the quick module.
- CMake & ninja: the newer, the better.

## Build

```bash
git clone https://github.com/wangwenx190/framelesshelper.git
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=<YOUR_QT_SDK_DIR_PATH> -DCMAKE_BUILD_TYPE=Release -GNinja ../framelesshelper
cmake --build . --config Release --target all --parallel
```

**Note**: On Linux you need to install the _GTK3_ and _X11_ development packages first.

## Use

### Qt Widgets

To customize the window frame of a QWidget, you need to instantiate a `FramelessWidgetsHelper` object and then attach it to the widget's top level parent for the widget.
`FramelessWidgetsHelper` will do all the work for you: the window frame will be removed automatically once you attach it to your top level widget. In theory you can instantiate
multiple `FramelessWidgetsHelper` objects for the same widget, in this case there will be only one object that keeps functional, all other objects of `FramelessWidgetsHelper` will
become a wrapper of that one. But to make sure everything goes smoothly and normally, you should not do that in any case. The simplest way to instantiate a `FramelessWidgetsHelper`
object is to call the static method `FramelessWidgetsHelper *FramelessWidgetsHelper::get(QObject *)`. It will return the previously instantiated object if any, or it will
instantiate a new object if it can't find one. It's safe to call it multiple times for a same widget, it won't instantiate any new object if there is one already. It also does
not matter where you call that function as long as the top level widget is the same. The internally created object will always be parented to the top level widget. Once you get
the `FramelessWidgetsHelper` object, you should call `void FramelessWidgetsHelper::attach()` to let it attach to the top level widget. The window frame
will be removed automatically once it has attached to the top level widget successfully. In order to make sure `FramelessWidgetsHelper` can find the correct top level widget,
you should call the `get` function on a widget which has a complete parent-chain. After these two steps, the window frame should be removed now. However, it can't be moved by
dragging because it doesn't have a title bar now. You should set a title bar widget to make the window be movable, the title bar doesn't need to be a rectangle, it also doesn't need to be on the top of the window. Call `void FramelessWidgetsHelper::setTitleBarWidget(QWidget *)` to do that. By default, all the widgets in the title bar area won't be responsible due to the mouse events are intercepted by FramelessHelper. To make them still work normally, you should make them visible to hit test. Call `void FramelessWidgetsHelper::setHitTestVisible(QWidget* )` to do that. You can of course call it on a widget that is not inside the title bar at all, but it won't have any effect. Due to Qt's own limitations, you need to make sure your widget has a complete parent-chain which the root parent is the top level widget.

### Qt Quick

TODO

Please refer to the demo applications to see more detailed usages: [examples](./examples/)

## Platform Notes

### Windows

- If DWM composition is disabled in some very rare cases (only possible on Windows 7), the top-left corner and top-right corner will appear in round shape. The round corners can be restored to square if you re-enable DWM composition.
- There's an OpenGL driver bug which will cause some frameless windows have a strange black bar right on top of your homemade title bar, and it also makes the controls in your windows shifted to the bottom-right corner for some pixels. It's a bug of your graphics card driver, specifically, your OpenGL driver, not FramelessHelper. There are some solutions provided by our users but some of them may not work in all conditions, you can pick one from them:
  - Upgrade your graphics card driver to the latest version.
  - Change your system theme to "Basic".
  - If you have multiple graphics cards, try to use another one instead.
  - Force your application use the ANGLE backend instead of the Desktop OpenGL.
  - Force your application use pure software rendering instead of rendering through OpenGL.
  - Or just don't use OpenGL at all, try to use Direct3D/Vulkan/Metal instead.
- Due to there are many sub-versions of Windows 10, it's highly recommended to use the latest version of Windows 10, at least no older than Windows 10 1809. If you try to use this framework on some very old Windows 10 versions such as 1507 or 1607, there may be some compatibility issues. Using this framework on Windows 7 is also supported but not recommended. To get the most stable behavior and the best appearance, you should use it on the latest version of Windows 10 or Windows 11.
- To make the snap layout work as expected, there are some additional requirements for your homemade system buttons to follow:
  - Make sure there are two public invokable functions (slot functions are always invokable): `void setHovered(bool)` and `void setPressed(bool)`. These two functions will be invoked by FramelessHelper when the button is being hovered or pressed. You should change the button's visual state inside these functions.
  - Make sure there's a public signal: `void clicked()`. When the button is being clicked, that signal will be triggered by FramelessHelper. You should connect your event handler to that signal.
  - Don't forget to call `setSystemButton()` for each button to let FramelessHelper know which is the minimize/maximize/close button.
  - System buttons will not be able to receive any actual mouse events so there's no need to handle any mouse events inside these buttons. That's also why we need to set the button's visual state manually.
  - I know this is making everything complicated but unfortunately we can't avoid this mess if we need to support the snap layout feature. Snap layout is really only designed for the original standard window frame, so if we want to forcely support it without a standard window frame, many black magic will be needed.

### Linux

- FramelessHelper will force your application to use the _XCB_ platform plugin when running on Wayland.
- Currently lacks runtime theme switching support due to Qt is missing the ability to detect theme change event on Linux.
- The resize area is inside of the window.

### macOS

- The frameless windows will appear in square corners instead of round corners.
- The resize area is inside of the window.

## License

```text
MIT License

Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

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
