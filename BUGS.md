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

## Widgets or Qt Quick elements are in wrong position when DPI changes / DPI改变后控件错位

When the DPI changes and the application is still running or the window of the application was moved to a different monitor which has a different DPI, the widgets (Qt Widgets application) or Qt Quick elements (Qt Quick application) are in wrong position.

This is a Qt bug and hasn't been fixed till Qt 5.15. Its root cause is the window is not repainted, the widgets or Qt Quick elements are in right position actually. The temporary solution is either ignore this issue (all Qt applications have this issue and most of them don't handle this) or resize the window manually for one pixel (so the user won't see a visual change) to trigger a repaint. Don't use platform-specific native APIs such as `RedrawWindow` to do this because Qt has take over the painting of the window so you have to let Qt to do the repaint.

当DPI改变或窗口被移动到一个DPI不同的显示器后，控件位置不正确（看起来或许像是花屏了）

这是Qt自身的bug，并且直到5.15都还没有被修复。这个bug是窗口没有重绘导致的，控件的位置实际上是正确的，只是画面没有刷新，看起来像是错位了。临时的解决方案有两个，一个是直接忽略这个问题（所有Qt程序都存在这个问题，但绝大多数都没有对这个问题进行处理，我怀疑大多数开发者也不知道这个地方有问题），另一个是手动调整窗口的尺寸来触发重绘（最好只调整一个像素，这样的话能触发重绘，但用户基本看不到这个过程）。不要使用诸如`RedrawWindow`这样的平台原生API来重绘，因为Qt程序的界面全都是自绘的，窗口的绘制已经完全被Qt接管了，你这样手动调用API是无效的，必须要Qt自己去重绘才能生效。
