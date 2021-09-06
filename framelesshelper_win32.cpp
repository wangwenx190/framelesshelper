/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "framelesshelper_win32.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindow.h>
#include "utilities.h"
#include "framelesshelper_windows.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

[[nodiscard]] static inline bool shouldHaveWindowFrame()
{
    if (Utilities::shouldUseNativeTitleBar()) {
        // We have to use the original window frame unconditionally if we
        // want to use the native title bar.
        return true;
    }
    const bool should = qEnvironmentVariableIsSet(Constants::kPreserveNativeFrameFlag);
    const bool force = qEnvironmentVariableIsSet(Constants::kForcePreserveNativeFrameFlag);
    if (should || force) {
        if (force) {
            return true;
        }
        if (should) {
            // If you preserve the window frame on Win7~8.1,
            // the window will have a terrible appearance.
            return Utilities::isWin10OrGreater();
        }
    }
    return false;
}

struct FramelessHelperWinData
{
    [[nodiscard]] bool create() {
        if (!m_instance.isNull()) {
            return false;
        }
        m_instance.reset(new FramelessHelperWin);
        return !m_instance.isNull();
    }

    [[nodiscard]] bool release() {
        if (!m_instance.isNull()) {
            m_instance.reset();
        }
        return m_instance.isNull();
    }

    [[nodiscard]] bool isNull() const {
        return m_instance.isNull();
    }

    [[nodiscard]] bool install() {
        if (isInstalled()) {
            return true;
        }
        if (isNull()) {
            if (!create()) {
                return false;
            }
        }
        QCoreApplication::instance()->installNativeEventFilter(m_instance.data());
        m_installed = true;
        return true;
    }

    [[nodiscard]] bool uninstall() {
        if (!isInstalled()) {
            return true;
        }
        if (isNull()) {
            return false;
        }
        QCoreApplication::instance()->removeNativeEventFilter(m_instance.data());
        m_installed = false;
        return true;
    }

    [[nodiscard]] bool isInstalled() const {
        return m_installed;
    }

private:
    QScopedPointer<FramelessHelperWin> m_instance;
    bool m_installed = false;
};

Q_GLOBAL_STATIC(FramelessHelperWinData, g_framelessHelperWinData)

static inline void installHelper(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    Utilities::updateQtFrameMargins(window, enable);
    const WId winId = window->winId();
    Utilities::updateFrameMargins(winId, !enable);
    Utilities::triggerFrameChange(winId);
    window->setProperty(Constants::kFramelessModeFlag, enable);
}

FramelessHelperWin::FramelessHelperWin() = default;

FramelessHelperWin::~FramelessHelperWin() = default;

void FramelessHelperWin::addFramelessWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (g_framelessHelperWinData()->install()) {
        installHelper(window, true);
    } else {
        qCritical() << "Failed to install native event filter.";
    }
}

void FramelessHelperWin::removeFramelessWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    installHelper(window, false);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    if ((eventType != QByteArrayLiteral("windows_generic_MSG")) || !message || !result) {
        return false;
    }
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    // Work-around a bug caused by typo which only exists in Qt 5.11.1
    const auto msg = *reinterpret_cast<MSG **>(message);
#else
    const auto msg = static_cast<LPMSG>(message);
#endif
    if (!msg->hwnd) {
        // Why sometimes the window handle is null? Is it designed to be?
        // Anyway, we should skip it in this case.
        return false;
    }
    const QWindow *window = Utilities::findWindow(reinterpret_cast<WId>(msg->hwnd));
    if (!window || !window->property(Constants::kFramelessModeFlag).toBool()) {
        return false;
    }
    switch (msg->message) {
    case WM_NCCALCSIZE: {
        // Windows是根据这个消息的返回值来设置窗口的客户区（窗口中真正显示的内容）
        // 和非客户区（标题栏、窗口边框、菜单栏和状态栏等Windows系统自行提供的部分
        // ，不过对于Qt来说，除了标题栏和窗口边框，非客户区基本也都是自绘的）的范
        // 围的，lParam里存放的就是新客户区的几何区域，默认是整个窗口的大小，正常
        // 的程序需要修改这个参数，告知系统窗口的客户区和非客户区的范围（一般来说可
        // 以完全交给Windows，让其自行处理，使用默认的客户区和非客户区），因此如果
        // 我们不修改lParam，就可以使客户区充满整个窗口，从而去掉标题栏和窗口边框
        // （因为这些东西都被客户区给盖住了。但边框阴影也会因此而丢失，不过我们会使
        // 用其他方式将其带回，请参考其他消息的处理，此处不过多提及）。但有个情况要
        // 特别注意，那就是窗口最大化后，窗口的实际尺寸会比屏幕的尺寸大一点，从而使
        // 用户看不到窗口的边界，这样用户就不能在窗口最大化后调整窗口的大小了（虽然
        // 这个做法听起来特别奇怪，但Windows确实就是这样做的），因此如果我们要自行
        // 处理窗口的非客户区，就要在窗口最大化后，将窗口边框的宽度和高度（一般是相
        // 等的）从客户区裁剪掉，否则我们窗口所显示的内容就会超出屏幕边界，显示不全。
        // 如果用户开启了任务栏自动隐藏，在窗口最大化后，还要考虑任务栏的位置。因为
        // 如果窗口最大化后，其尺寸和屏幕尺寸相等（因为任务栏隐藏了，所以窗口最大化
        // 后其实是充满了整个屏幕，变相的全屏了），Windows会认为窗口已经进入全屏的
        // 状态，从而导致自动隐藏的任务栏无法弹出。要避免这个状况，就要使窗口的尺寸
        // 小于屏幕尺寸。我下面的做法参考了火狐、Chromium和Windows Terminal
        // 如果没有开启任务栏自动隐藏，是不存在这个问题的，所以要先进行判断。
        // 一般情况下，*result设置为0（相当于DefWindowProc的返回值为0）就可以了，
        // 根据MSDN的说法，返回0意为此消息已经被程序自行处理了，让Windows跳过此消
        // 息，否则Windows会添加对此消息的默认处理，对于当前这个消息而言，就意味着
        // 标题栏和窗口边框又会回来，这当然不是我们想要的结果。根据MSDN，当wParam
        // 为FALSE时，只能返回0，但当其为TRUE时，可以返回0，也可以返回一个WVR_常
        // 量。根据Chromium的注释，当存在非客户区时，如果返回WVR_REDRAW会导致子
        // 窗口/子控件出现奇怪的bug（自绘控件错位），并且Lucas在Windows 10
        // 上成功复现，说明这个bug至今都没有解决。我查阅了大量资料，发现唯一的解决
        // 方案就是返回0。但如果不存在非客户区，且wParam为TRUE，最好返回
        // WVR_REDRAW，否则窗口在调整大小可能会产生严重的闪烁现象。
        // 虽然对大多数消息来说，返回0都代表让Windows忽略此消息，但实际上不同消息
        // 能接受的返回值是不一样的，请注意自行查阅MSDN。

        // Sent when the size and position of a window's client area must be
        // calculated. By processing this message, an application can
        // control the content of the window's client area when the size or
        // position of the window changes. If wParam is TRUE, lParam points
        // to an NCCALCSIZE_PARAMS structure that contains information an
        // application can use to calculate the new size and position of the
        // client rectangle. If wParam is FALSE, lParam points to a RECT
        // structure. On entry, the structure contains the proposed window
        // rectangle for the window. On exit, the structure should contain
        // the screen coordinates of the corresponding window client area.
        // The client area is the window's content area, the non-client area
        // is the area which is provided by the system, such as the title
        // bar, the four window borders, the frame shadow, the menu bar, the
        // status bar, the scroll bar, etc. But for Qt, it draws most of the
        // window area (client + non-client) itself. We now know that the
        // title bar and the window frame is in the non-client area and we
        // can set the scope of the client area in this message, so we can
        // remove the title bar and the window frame by let the non-client
        // area be covered by the client area (because we can't really get
        // rid of the non-client area, it will always be there, all we can
        // do is to hide it) , which means we should let the client area's
        // size the same with the whole window's size. So there is no room
        // for the non-client area and then the user won't be able to see it
        // again. But how to achieve this? Very easy, just leave lParam (the
        // re-calculated client area) untouched. But of course you can
        // modify lParam, then the non-client area will be seen and the
        // window borders and the window frame will show up. However, things
        // are quite different when you try to modify the top margin of the
        // client area. DWM will always draw the whole title bar no matter
        // what margin value you set for the top, unless you don't modify it
        // and remove the whole top area (the title bar + the one pixel
        // height window border). This can be confirmed in Windows
        // Terminal's source code, you can also try yourself to verify
        // it. So things will become quite complicated if you want to
        // preserve the four window borders. So we just remove the whole
        // window frame, otherwise the code will become much more complex.

        if (Utilities::shouldUseNativeTitleBar()) {
            break;
        }

        if (msg->wParam == FALSE) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam)->rgrc[0]);
        if (shouldHaveWindowFrame()) {
            // Store the original top before the default window proc
            // applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcW(msg->hwnd, WM_NCCALCSIZE, TRUE, msg->lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the
            // default frame was applied.
            clientRect->top = originalTop;
        }
        bool nonClientAreaExists = false;
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (IsMaximized(msg->hwnd) && (window->windowState() != Qt::WindowFullScreen)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int resizeBorderThickness = Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, true);
            clientRect->top += resizeBorderThickness;
            if (!shouldHaveWindowFrame()) {
                clientRect->bottom -= resizeBorderThickness;
                clientRect->left += resizeBorderThickness;
                clientRect->right -= resizeBorderThickness;
            }
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (IsMaximized(msg->hwnd) || (window->windowState() == Qt::WindowFullScreen)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = SHAppBarMessage(ABM_GETSTATE, &abd);
            // First, check if we have an auto-hide taskbar at all:
            if (taskbarState & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to ABM_GETAUTOHIDEBAREX only exists from Win8.1,
                // we have to use another way to judge this if we are
                // running on Windows 7 or Windows 8.
                if (Utilities::isWin8Point1OrGreater()) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    const HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                    if (!monitor) {
                        qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("MonitorFromWindow"));
                        break;
                    }
                    if (GetMonitorInfoW(monitor, &monitorInfo) == FALSE) {
                        qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("GetMonitorInfoW"));
                        break;
                    }
                    // This helper can be used to determine if there's a
                    // auto-hide taskbar on the given edge of the monitor
                    // we're currently on.
                    const auto hasAutohideTaskbar = [&monitorInfo](const UINT edge) -> bool {
                        APPBARDATA _abd;
                        SecureZeroMemory(&_abd, sizeof(_abd));
                        _abd.cbSize = sizeof(_abd);
                        _abd.uEdge = edge;
                        _abd.rc = monitorInfo.rcMonitor;
                        const auto hTaskbar = reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &_abd));
                        return (hTaskbar != nullptr);
                    };
                    top = hasAutohideTaskbar(ABE_TOP);
                    bottom = hasAutohideTaskbar(ABE_BOTTOM);
                    left = hasAutohideTaskbar(ABE_LEFT);
                    right = hasAutohideTaskbar(ABE_RIGHT);
                } else {
                    // The following code is copied from Mozilla Firefox,
                    // with some modifications.
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                        if (!windowMonitor) {
                            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("MonitorFromWindow"));
                            break;
                        }
                        const HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (!taskbarMonitor) {
                            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("MonitorFromWindow"));
                            break;
                        }
                        if (taskbarMonitor == windowMonitor) {
                            SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    } else {
                        qWarning() << "Failed to retrieve the task bar window handle.";
                        break;
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }
                // If there's a taskbar on any side of the monitor, reduce
                // our size a little bit on that edge.
                // Note to future code archeologists:
                // This doesn't seem to work for fullscreen on the primary
                // display. However, testing a bunch of other apps with
                // fullscreen modes and an auto-hiding taskbar has
                // shown that _none_ of them reveal the taskbar from
                // fullscreen mode. This includes Edge, Firefox, Chrome,
                // Sublime Text, PowerPoint - none seemed to support this.
                // This does however work fine for maximized.
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += kAutoHideTaskbarThickness;
                    nonClientAreaExists = true;
                } else if (bottom) {
                    clientRect->bottom -= kAutoHideTaskbarThickness;
                    nonClientAreaExists = true;
                } else if (left) {
                    clientRect->left += kAutoHideTaskbarThickness;
                    nonClientAreaExists = true;
                } else if (right) {
                    clientRect->right -= kAutoHideTaskbarThickness;
                    nonClientAreaExists = true;
                }
            }
        }
#if 0
        // Fix the flickering issue while resizing.
        // "clientRect->right += 1;" also works.
        // This small technique is known to have two draw backs:
        // (1) Qt's coordinate system will be confused because the canvas size
        // doesn't match the client area size so you will get some warnings
        // from Qt and you should also be careful when you try to draw something
        // manually through QPainter or in Qt Quick, be aware of the coordinate
        // mismatch issue when you calculate position yourself.
        // (2) Qt's window system will take some wrong actions when the window
        // is being resized. For example, the window size will become 1px smaller
        // or bigger everytime when resize() is called because the client area size
        // is not correct. It confuses QPA's internal logic.
        clientRect->bottom += 1;
#endif
        // If the window bounds change, we're going to relayout and repaint
        // anyway. Returning WVR_REDRAW avoids an extra paint before that of
        // the old client pixels in the (now wrong) location, and thus makes
        // actions like resizing a window from the left edge look slightly
        // less broken.
        //
        // We cannot return WVR_REDRAW when there is nonclient area, or
        // Windows exhibits bugs where client pixels and child HWNDs are
        // mispositioned by the width/height of the upper-left nonclient
        // area.
        *result = nonClientAreaExists ? 0 : WVR_REDRAW;
        return true;
    }
    // These undocumented messages are sent to draw themed window
    // borders. Block them to prevent drawing borders over the client
    // area.
    case WM_NCUAHDRAWCAPTION:
    case WM_NCUAHDRAWFRAME: {
        if (shouldHaveWindowFrame()) {
            break;
        } else {
            *result = 0;
            return true;
        }
    }
    case WM_NCPAINT: {
        // 边框阴影处于非客户区的范围，因此如果直接阻止非客户区的绘制，会导致边框阴影丢失

        if (!Utilities::isDwmCompositionAvailable() && !shouldHaveWindowFrame()) {
            // Only block WM_NCPAINT when DWM composition is disabled. If
            // it's blocked when DWM composition is enabled, the frame
            // shadow won't be drawn.
            *result = 0;
            return true;
        } else {
            break;
        }
    }
    case WM_NCACTIVATE: {
        if (shouldHaveWindowFrame()) {
            break;
        } else {
            if (Utilities::isDwmCompositionAvailable()) {
                // DefWindowProc won't repaint the window border if lParam
                // (normally a HRGN) is -1. See the following link's "lParam"
                // section:
                // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
                // Don't use "*result = 0" otherwise the window won't respond
                // to the window active state change.
                *result = DefWindowProcW(msg->hwnd, WM_NCACTIVATE, msg->wParam, -1);
            } else {
                if (msg->wParam == FALSE) {
                    *result = TRUE;
                } else {
                    *result = FALSE;
                }
            }
            return true;
        }
    }
    case WM_NCHITTEST: {
        // 原生Win32窗口只有顶边是在窗口内部resize的，其余三边都是在窗口
        // 外部进行resize的，其原理是，WS_THICKFRAME这个窗口样式会在窗
        // 口的左、右和底边添加三个透明的resize区域，这三个区域在正常状态
        // 下是完全不可见的，它们由DWM负责绘制和控制。这些区域的宽度等于
        // (SM_CXSIZEFRAME + SM_CXPADDEDBORDER)，高度等于
        // (SM_CYSIZEFRAME + SM_CXPADDEDBORDER)，在100%缩放时，均等
        // 于8像素。它们属于窗口区域的一部分，但不属于客户区，而是属于非客
        // 户区，因此GetWindowRect获取的区域中是包含这三个resize区域的，
        // 而GetClientRect获取的区域是不包含它们的。当把
        // DWMWA_EXTENDED_FRAME_BOUNDS作为参数调用
        // DwmGetWindowAttribute时，也能获取到一个窗口大小，这个大小介
        // 于前面两者之间，暂时不知道这个数据的意义及其作用。我们在
        // WM_NCCALCSIZE消息的处理中，已经把整个窗口都设置为客户区了，也
        // 就是说，我们的窗口已经没有非客户区了，因此那三个透明的resize区
        // 域，此刻也已经成为窗口客户区的一部分了，从而变得不透明了。所以
        // 现在的resize，看起来像是在窗口内部resize，是因为原本透明的地方
        // 现在变得不透明了，实际上，单纯从范围上来看，现在我们resize的地方，
        // 就是普通窗口的边框外部，那三个透明区域的范围。
        // 因此，如果我们把边框完全去掉（就是我们正在做的事情），resize就
        // 会看起来是在内部进行，这个问题通过常规方法非常难以解决。我测试过
        // QQ和钉钉的窗口，它们的窗口就是在外部resize，但实际上它们是通过
        // 把窗口实际的内容，嵌入到一个完全透明的但尺寸要大一圈的窗口中实现
        // 的，虽然看起来效果还行，但在我看来不是正途。而且我之所以能发现，
        // 也是由于这种方法在很多情况下会露馅，比如窗口未响应卡住或贴边的时
        // 候，能明显看到窗口周围多出来一圈边界。我曾经尝试再把那三个区域弄
        // 透明，但无一例外都会破坏DWM绘制的边框阴影，因此只好作罢。

        // As you may have found, if you use this code, the resize areas
        // will be inside the frameless window, however, a normal Win32
        // window can be resized outside of it. Here is the reason: the
        // WS_THICKFRAME window style will cause a window has three
        // transparent areas beside the window's left, right and bottom
        // edge. Their width or height is eight pixels if the window is not
        // scaled. In most cases, they are totally invisible. It's DWM's
        // responsibility to draw and control them. They exist to let the
        // user resize the window, visually outside of it. They are in the
        // window area, but not the client area, so they are in the
        // non-client area actually. But we have turned the whole window
        // area into client area in WM_NCCALCSIZE, so the three transparent
        // resize areas also become a part of the client area and thus they
        // become visible. When we resize the window, it looks like we are
        // resizing inside of it, however, that's because the transparent
        // resize areas are visible now, we ARE resizing outside of the
        // window actually. But I don't know how to make them become
        // transparent again without breaking the frame shadow drawn by DWM.
        // If you really want to solve it, you can try to embed your window
        // into a larger transparent window and draw the frame shadow
        // yourself. As what we have said in WM_NCCALCSIZE, you can only
        // remove the top area of the window, this will let us be able to
        // resize outside of the window and don't need much process in this
        // message, it looks like a perfect plan, however, the top border is
        // missing due to the whole top area is removed, and it's very hard
        // to bring it back because we have to use a trick in WM_PAINT
        // (learned from Windows Terminal), but no matter what we do in
        // WM_PAINT, it will always break the backing store mechanism of Qt,
        // so actually we can't do it. And it's very difficult to do such
        // things in NativeEventFilters as well. What's worse, if we really
        // do this, the four window borders will become white and they look
        // horrible in dark mode. This solution only supports Windows 10
        // because the border width on Win10 is only one pixel, however it's
        // eight pixels on Windows 7 so preserving the three window borders
        // looks terrible on old systems. I'm testing this solution in
        // another branch, if you are interested in it, you can give it a
        // try.

        if (Utilities::shouldUseNativeTitleBar()) {
            break;
        }

        POINT winLocalMouse = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
        if (ScreenToClient(msg->hwnd, &winLocalMouse) == FALSE) {
            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("ScreenToClient"));
            break;
        }
        const QPointF localMouse = {static_cast<qreal>(winLocalMouse.x), static_cast<qreal>(winLocalMouse.y)};
        RECT clientRect = {0, 0, 0, 0};
        if (GetClientRect(msg->hwnd, &clientRect) == FALSE) {
            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("GetClientRect"));
            break;
        }
        const LONG windowWidth = clientRect.right;
        const int resizeBorderThickness = Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, true);
        const int titleBarHeight = Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, true);
        bool isTitleBar = false;
        if (IsMaximized(msg->hwnd) || (window->windowState() == Qt::WindowFullScreen)) {
            isTitleBar = (localMouse.y() >= 0) && (localMouse.y() <= titleBarHeight)
                    && (localMouse.x() >= 0) && (localMouse.x() <= windowWidth)
                    && !Utilities::isHitTestVisibleInChrome(window);
        }
        if (window->windowState() == Qt::WindowNoState) {
            isTitleBar = (localMouse.y() > resizeBorderThickness) && (localMouse.y() <= titleBarHeight)
                    && (localMouse.x() > resizeBorderThickness) && (localMouse.x() < (windowWidth - resizeBorderThickness))
                    && !Utilities::isHitTestVisibleInChrome(window);
        }
        const bool isTop = localMouse.y() <= resizeBorderThickness;
        if (shouldHaveWindowFrame()) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(msg->hwnd, WM_NCHITTEST, 0, msg->lParam);
            if (originalRet != HTCLIENT) {
                *result = originalRet;
                return true;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (!IsMaximized(msg->hwnd) && isTop) {
                *result = HTTOP;
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        } else {
            const LRESULT hitTestResult = [clientRect, msg, isTitleBar, &localMouse, resizeBorderThickness, windowWidth, isTop, window]{
                if (IsMaximized(msg->hwnd)) {
                    if (isTitleBar) {
                        return HTCAPTION;
                    }
                    return HTCLIENT;
                }
                const LONG windowHeight = clientRect.bottom;
                const bool isBottom = (localMouse.y() >= (windowHeight - resizeBorderThickness));
                // Make the border a little wider to let the user easy to resize on corners.
                const qreal factor = (isTop || isBottom) ? 2.0 : 1.0;
                const bool isLeft = (localMouse.x() <= qRound(static_cast<qreal>(resizeBorderThickness) * factor));
                const bool isRight = (localMouse.x() >= (windowWidth - qRound(static_cast<qreal>(resizeBorderThickness) * factor)));
                const bool fixedSize = Utilities::isWindowFixedSize(window);
                const auto getBorderValue = [fixedSize](int value) -> int {
                    return fixedSize ? HTCLIENT : value;
                };
                if (isTop) {
                    if (isLeft) {
                        return getBorderValue(HTTOPLEFT);
                    }
                    if (isRight) {
                        return getBorderValue(HTTOPRIGHT);
                    }
                    return getBorderValue(HTTOP);
                }
                if (isBottom) {
                    if (isLeft) {
                        return getBorderValue(HTBOTTOMLEFT);
                    }
                    if (isRight) {
                        return getBorderValue(HTBOTTOMRIGHT);
                    }
                    return getBorderValue(HTBOTTOM);
                }
                if (isLeft) {
                    return getBorderValue(HTLEFT);
                }
                if (isRight) {
                    return getBorderValue(HTRIGHT);
                }
                if (isTitleBar) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            }();
            *result = hitTestResult;
            return true;
        }
    }
    case WM_SETICON:
    case WM_SETTEXT: {
        if (Utilities::shouldUseNativeTitleBar()) {
            break;
        }

        // Disable painting while these messages are handled to prevent them
        // from drawing a window caption over the client area.
        const LONG_PTR oldStyle = GetWindowLongPtrW(msg->hwnd, GWL_STYLE);
        // Prevent Windows from drawing the default title bar by temporarily
        // toggling the WS_VISIBLE style.
        if (SetWindowLongPtrW(msg->hwnd, GWL_STYLE, static_cast<LONG_PTR>(oldStyle & ~WS_VISIBLE)) == 0) {
            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("SetWindowLongPtrW"));
            break;
        }
        const auto winId = reinterpret_cast<WId>(msg->hwnd);
        Utilities::triggerFrameChange(winId);
        const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        if (SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle) == 0) {
            qWarning() << Utilities::getSystemErrorMessage(QStringLiteral("SetWindowLongPtrW"));
            break;
        }
        Utilities::triggerFrameChange(winId);
        *result = ret;
        return true;
    }
    case WM_SIZE: {
        const bool normal = (msg->wParam == SIZE_RESTORED);
        const bool max = (msg->wParam == SIZE_MAXIMIZED);
        const bool full = (window->windowState() == Qt::WindowFullScreen);
        if (normal || max || full) {
            Utilities::updateFrameMargins(reinterpret_cast<WId>(msg->hwnd), (max || full));
            Utilities::updateQtFrameMargins(const_cast<QWindow *>(window), true);
        }
    } break;
    default:
        break;
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
