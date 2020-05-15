# Known Bugs

## Repaint not in time / 重绘不及时

The white background of the window can be seen if it's being resized using the right-bottom corner.

Most frameless applications have this issue. The root cause and it's solution is unknown.

用右下角调整窗口大小时，窗口的白底（白色背景）能被看到。

大多数自定义边框的程序（VS、VSCode、Adobe全家桶、Office全家桶、火狐、Chrome等）都存在这个问题（不过它们被看到的不一定是白色的背景，但能看到窗口自身的背景是肯定的），暂时不知道原因，解决方案也未知。

## Window flickers / 窗口抖动或闪烁

The window flickers when it's being resized using the top or left edge.

Most frameless applications have this issue. The root cause and it's solution is unknown.

用顶边或左边调整窗口大小时，窗口剧烈抖动或闪烁。

大多数自定义边框的程序都存在这个问题，暂时不知道原因，解决方案也未知。

## Unusual memory usage / 内存用量异常

The application uses much more memory after using this repo.

Turn off layered window will solve this. But the window may flicker (see the previous bug).

使用这个项目后，程序的内存用量突然明显增加。

关闭分层窗口可以解决这个问题，但这可能会导致窗口出现抖动或闪烁的问题（见上一个bug）。

## Window has white borders / 窗口有白边

The frameless window has white border on the right and bottom edges on Windows 7.

Qt 5.6 series have this issue. Update your Qt version.

在 Windows 7 上，窗口右边和底边有白边。

Qt 5.6 系列存在这个问题。请升级您的 Qt 版本。
