/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
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

#include "framelesshelper_win.h"
#include <QtCore/qdebug.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindow.h>
#include "framelesswindowsmanager.h"
#include "framelesswindowsmanager_p.h"
#include "utils.h"
#include "framelesshelper_windows.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct Win32HelperData
{
    UserSettings settings = {};
    SystemParameters params = {};
    WNDPROC originalWindowProc = nullptr;
};

struct Win32Helper
{
    QMutex mutex;
    QScopedPointer<FramelessHelperWin> nativeEventFilter;
    QHash<WId, Win32HelperData> data = {};
};

Q_GLOBAL_STATIC(Win32Helper, g_win32Helper)

FRAMELESSHELPER_BYTEARRAY_CONSTANT2(Win32MessageTypeName, "windows_generic_MSG")
static const QString qThemeSettingChangeEventName = QString::fromWCharArray(kThemeSettingChangeEventName);
FRAMELESSHELPER_STRING_CONSTANT(MonitorFromWindow)
FRAMELESSHELPER_STRING_CONSTANT(GetMonitorInfoW)
FRAMELESSHELPER_STRING_CONSTANT(ScreenToClient)
FRAMELESSHELPER_STRING_CONSTANT(ClientToScreen)
FRAMELESSHELPER_STRING_CONSTANT(GetClientRect)
#ifdef Q_PROCESSOR_X86_64
  FRAMELESSHELPER_STRING_CONSTANT(GetWindowLongPtrW)
  FRAMELESSHELPER_STRING_CONSTANT(SetWindowLongPtrW)
#else
  // WinUser.h defines G/SetClassLongPtr as G/SetClassLong due to the
  // "Ptr" suffixed APIs are not available on 32-bit platforms, so we
  // have to add the following workaround. Undefine the macros and then
  // redefine them is also an option but the following solution is more simple.
  FRAMELESSHELPER_STRING_CONSTANT2(GetWindowLongPtrW, "GetWindowLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetWindowLongPtrW, "SetWindowLongW")
#endif

[[nodiscard]] static inline Qt::MouseButtons keyStateToMouseButtons(const WPARAM wParam)
{
    if (wParam == 0) {
        return {};
    }
    Qt::MouseButtons result = {};
    if (wParam & MK_LBUTTON) {
        result |= Qt::LeftButton;
    }
    if (wParam & MK_MBUTTON) {
        result |= Qt::MiddleButton;
    }
    if (wParam & MK_RBUTTON) {
        result |= Qt::RightButton;
    }
    if (wParam & MK_XBUTTON1) {
        result |= Qt::XButton1;
    }
    if (wParam & MK_XBUTTON2) {
        result |= Qt::XButton2;
    }
    return result;
}

[[nodiscard]] static inline Qt::MouseButtons queryMouseButtons()
{
    Qt::MouseButtons result = {};
    const bool mouseSwapped = (GetSystemMetrics(SM_SWAPBUTTON) != FALSE);
    if (GetAsyncKeyState(VK_LBUTTON) < 0) {
        result |= (mouseSwapped ? Qt::RightButton: Qt::LeftButton);
    }
    if (GetAsyncKeyState(VK_MBUTTON) < 0) {
        result |= Qt::MiddleButton;
    }
    if (GetAsyncKeyState(VK_RBUTTON) < 0) {
        result |= (mouseSwapped ? Qt::LeftButton : Qt::RightButton);
    }
    if (GetAsyncKeyState(VK_XBUTTON1) < 0) {
        result |= Qt::XButton1;
    }
    if (GetAsyncKeyState(VK_XBUTTON2) < 0) {
        result |= Qt::XButton2;
    }
    return result;
}

[[nodiscard]] static inline LRESULT CALLBACK MaximizeDockingHookWindowProc
    (const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    Q_ASSERT(hWnd);
    if (!hWnd) {
        return 0;
    }
    const auto windowId = reinterpret_cast<WId>(hWnd);
    g_win32Helper()->mutex.lock();
    if (!g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    const Win32HelperData data = g_win32Helper()->data.value(windowId);
    g_win32Helper()->mutex.unlock();
    const bool isNonClientMouseEvent = (((uMsg >= WM_NCMOUSEMOVE) && (uMsg <= WM_NCMBUTTONDBLCLK))
                                        || (uMsg == WM_NCHITTEST));
    const bool isClientMouseEvent = (((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST))
                                     || ((uMsg >= WM_XBUTTONDOWN) && (uMsg <= WM_XBUTTONDBLCLK)));
    if (isNonClientMouseEvent || isClientMouseEvent) {
        const Qt::MouseButtons mouseButtons = (isNonClientMouseEvent ? queryMouseButtons() : keyStateToMouseButtons(wParam));
        const POINT nativePosExtractedFromLParam = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT nativeScreenPos = {};
        POINT nativeWindowPos = {};
        if (isNonClientMouseEvent) {
            nativeScreenPos = nativePosExtractedFromLParam;
            nativeWindowPos = nativeScreenPos;
            if (ScreenToClient(hWnd, &nativeWindowPos) == FALSE) {
                qWarning() << Utils::getSystemErrorMessage(kScreenToClient);
                return DefWindowProcW(hWnd, uMsg, wParam, lParam);
            }
        }
        if (isClientMouseEvent) {
            nativeWindowPos = nativePosExtractedFromLParam;
            nativeScreenPos = nativeWindowPos;
            if (ClientToScreen(hWnd, &nativeScreenPos) == FALSE) {
                qWarning() << Utils::getSystemErrorMessage(kClientToScreen);
                return DefWindowProcW(hWnd, uMsg, wParam, lParam);
            }
        }
        const qreal devicePixelRatio = data.params.getWindowDevicePixelRatio();
        const QPoint qtScenePos = QPointF(QPointF(qreal(nativeWindowPos.x), qreal(nativeWindowPos.y)) / devicePixelRatio).toPoint();
        SystemButtonType currentButtonType = SystemButtonType::Unknown;
        static constexpr const auto defaultButtonState = ButtonState::Unspecified;
        data.params.setSystemButtonState(SystemButtonType::WindowIcon, defaultButtonState);
        data.params.setSystemButtonState(SystemButtonType::Help, defaultButtonState);
        data.params.setSystemButtonState(SystemButtonType::Minimize, defaultButtonState);
        data.params.setSystemButtonState(SystemButtonType::Maximize, defaultButtonState);
        data.params.setSystemButtonState(SystemButtonType::Restore, defaultButtonState);
        data.params.setSystemButtonState(SystemButtonType::Close, defaultButtonState);
        if (data.params.isInsideSystemButtons(qtScenePos, &currentButtonType)) {
            Q_ASSERT(currentButtonType != SystemButtonType::Unknown);
            if (currentButtonType != SystemButtonType::Unknown) {
                const ButtonState currentButtonState = ((mouseButtons & Qt::LeftButton) ? ButtonState::Pressed : ButtonState::Hovered);
                data.params.setSystemButtonState(currentButtonType, currentButtonState);
            }
        }
        if ((uMsg == WM_NCHITTEST) && (currentButtonType != SystemButtonType::Unknown)) {
            const int hitTestResult = [currentButtonType]() -> int {
                switch (currentButtonType) {
                case SystemButtonType::WindowIcon:
                    return HTSYSMENU;
                case SystemButtonType::Help:
                    return HTHELP;
                case SystemButtonType::Minimize:
                    return HTREDUCE;
                case SystemButtonType::Maximize:
                case SystemButtonType::Restore:
                    return HTZOOM;
                case SystemButtonType::Close:
                    return HTCLOSE;
                default:
                    break;
                }
                return 0;
            }();
            Q_ASSERT(hitTestResult);
            if (hitTestResult != 0) {
                return hitTestResult;
            }
        }
    }
    Q_ASSERT(data.originalWindowProc);
    if (data.originalWindowProc) {
        return CallWindowProcW(data.originalWindowProc, hWnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

FramelessHelperWin::FramelessHelperWin() : QAbstractNativeEventFilter() {}

FramelessHelperWin::~FramelessHelperWin() = default;

void FramelessHelperWin::addWindow(const UserSettings &settings, const SystemParameters &params)
{
    Q_ASSERT(params.isValid());
    if (!params.isValid()) {
        return;
    }
    const WId windowId = params.getWindowId();
    g_win32Helper()->mutex.lock();
    if (g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return;
    }
    Win32HelperData data = {};
    data.settings = settings;
    data.params = params;
    if ((settings.options & Option::ForceHideWindowFrameBorder)
        && (settings.options & Option::ForceShowWindowFrameBorder)) {
        data.settings.options &= ~(Option::ForceHideWindowFrameBorder | Option::ForceShowWindowFrameBorder);
        qWarning() << "You can't use both \"Option::ForceHideWindowFrameBorder\" and "
                      "\"Option::ForceShowWindowFrameBorder\" at the same time.";
    }
    g_win32Helper()->data.insert(windowId, data);
    if (g_win32Helper()->nativeEventFilter.isNull()) {
        g_win32Helper()->nativeEventFilter.reset(new FramelessHelperWin);
        qApp->installNativeEventFilter(g_win32Helper()->nativeEventFilter.data());
    }
    g_win32Helper()->mutex.unlock();
    if (!(settings.options & Option::DontTouchQtInternals)) {
        Utils::fixupQtInternals(windowId);
    }
    Utils::updateInternalWindowFrameMargins(params.getWindowHandle(), true);
    Utils::updateWindowFrameMargins(windowId, false);
    if (Utils::isWin10_1607OrGreater()) {
        const bool dark = Utils::shouldAppsUseDarkMode();
        if (!(settings.options & Option::DontTouchWindowFrameBorderColor)) {
            Utils::updateWindowFrameBorderColor(windowId, dark);
        }
        if (Utils::isWin10_1809OrGreater()) {
            if (settings.options & Option::SyncNativeControlsThemeWithSystem) {
                Utils::updateGlobalWin32ControlsTheme(windowId, dark);
            }
            if (Utils::isWin11OrGreater()) {
                if (settings.options & Option::MaximizeButtonDocking) {
                    const auto hwnd = reinterpret_cast<HWND>(windowId);
                    SetLastError(ERROR_SUCCESS);
                    const auto originalWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
                    Q_ASSERT(originalWindowProc);
                    if (!originalWindowProc) {
                        qWarning() << Utils::getSystemErrorMessage(kGetWindowLongPtrW);
                        return;
                    }
                    g_win32Helper()->mutex.lock();
                    g_win32Helper()->data[windowId].originalWindowProc = originalWindowProc;
                    g_win32Helper()->mutex.unlock();
                    SetLastError(ERROR_SUCCESS);
                    if (SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MaximizeDockingHookWindowProc)) == 0) {
                        qWarning() << Utils::getSystemErrorMessage(kSetWindowLongPtrW);
                    }
                }
            }
        }
    }
}

bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, QT_NATIVE_EVENT_RESULT_TYPE *result)
{
    if ((eventType != kWin32MessageTypeName) || !message || !result) {
        return false;
    }
    // QPA by default stores the global mouse position in the pt field,
    // but let's not reply on such Qt-specific extensions.
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    // Work-around a bug caused by typo which only exists in Qt 5.11.1
    const auto msg = *static_cast<MSG **>(message);
#else
    const auto msg = static_cast<LPMSG>(message);
#endif
    const HWND hWnd = msg->hwnd;
    if (!hWnd) {
        // Why sometimes the window handle is null? Is it designed to be like this?
        // Anyway, we should skip the entire processing in this case.
        return false;
    }
    const auto windowId = reinterpret_cast<WId>(hWnd);
    g_win32Helper()->mutex.lock();
    if (!g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return false;
    }
    const Win32HelperData data = g_win32Helper()->data.value(windowId);
    g_win32Helper()->mutex.unlock();
    const bool frameBorderVisible = [&data]() -> bool {
        if (data.settings.options & Option::ForceShowWindowFrameBorder) {
            return true;
        }
        if (data.settings.options & Option::ForceHideWindowFrameBorder) {
            return false;
        }
        return Utils::isWindowFrameBorderVisible();
    }();
    const UINT uMsg = msg->message;
    const WPARAM wParam = msg->wParam;
    const LPARAM lParam = msg->lParam;
    switch (uMsg) {
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

        // If `wParam` is `FALSE`, `lParam` points to a `RECT` that contains
        // the proposed window rectangle for our window.  During our
        // processing of the `WM_NCCALCSIZE` message, we are expected to
        // modify the `RECT` that `lParam` points to, so that its value upon
        // our return is the new client area.  We must return 0 if `wParam`
        // is `FALSE`.
        //
        // If `wParam` is `TRUE`, `lParam` points to a `NCCALCSIZE_PARAMS`
        // struct.  This struct contains an array of 3 `RECT`s, the first of
        // which has the exact same meaning as the `RECT` that is pointed to
        // by `lParam` when `wParam` is `FALSE`.  The remaining `RECT`s, in
        // conjunction with our return value, can
        // be used to specify portions of the source and destination window
        // rectangles that are valid and should be preserved.  We opt not to
        // implement an elaborate client-area preservation technique, and
        // simply return 0, which means "preserve the entire old client area
        // and align it with the upper-left corner of our new client area".
        const auto clientRect = ((static_cast<BOOL>(wParam) == FALSE)
                                 ? reinterpret_cast<LPRECT>(lParam)
                                 : &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam))->rgrc[0]);
        if (frameBorderVisible) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame.
            const LRESULT ret = DefWindowProcW(hWnd, WM_NCCALCSIZE, wParam, lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        }
        const bool max = IsMaximized(hWnd);
        const bool full = Utils::isFullScreen(windowId);
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
            clientRect->top += frameSizeY;
            if (!frameBorderVisible) {
                clientRect->bottom -= frameSizeY;
                const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
                clientRect->left += frameSizeX;
                clientRect->right -= frameSizeX;
            }
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (max || full) {
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
                if (Utils::isWin8Point1OrGreater()) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    const HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                    if (!monitor) {
                        qWarning() << Utils::getSystemErrorMessage(kMonitorFromWindow);
                        break;
                    }
                    if (GetMonitorInfoW(monitor, &monitorInfo) == FALSE) {
                        qWarning() << Utils::getSystemErrorMessage(kGetMonitorInfoW);
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
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                        if (!windowMonitor) {
                            qWarning() << Utils::getSystemErrorMessage(kMonitorFromWindow);
                            break;
                        }
                        const HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (!taskbarMonitor) {
                            qWarning() << Utils::getSystemErrorMessage(kMonitorFromWindow);
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
                    clientRect->top += kAutoHideTaskBarThickness;
                } else if (bottom) {
                    clientRect->bottom -= kAutoHideTaskBarThickness;
                } else if (left) {
                    clientRect->left += kAutoHideTaskBarThickness;
                } else if (right) {
                    clientRect->right -= kAutoHideTaskBarThickness;
                }
            }
        }
        Utils::syncWmPaintWithDwm(); // This should be executed at the very last.
        // By returning WVR_REDRAW we can make the window resizing look less broken.
        // But we must return 0 if wParam is FALSE, according to Microsoft Docs.
        // **IMPORTANT NOTE**:
        // If you are drawing something manually through D3D in your window, don't
        // try to return WVR_REDRAW here, otherwise Windows exhibits bugs where
        // client pixels and child windows are mispositioned by the width/height
        // of the upper-left non-client area. It's confirmed that this issue exists
        // from Windows 7 to Windows 10. Not tested on Windows 11 yet. Don't know
        // whether it exists on Windows XP to Windows Vista or not.
        *result = ((static_cast<BOOL>(wParam) == FALSE) ? 0 : WVR_REDRAW);
        return true;
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

        if (data.params.isWindowFixedSize()) {
            *result = HTCLIENT;
            return true;
        }
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        if (ScreenToClient(hWnd, &localPos) == FALSE) {
            qWarning() << Utils::getSystemErrorMessage(kScreenToClient);
            break;
        }
        const bool max = IsMaximized(hWnd);
        const bool full = Utils::isFullScreen(windowId);
        const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
        const bool isTop = (localPos.y < frameSizeY);
        const bool isTitleBar = (false && !(data.settings.options & Option::DisableDragging));
        if (frameBorderVisible) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(hWnd, WM_NCHITTEST, 0, lParam);
            if (originalRet != HTCLIENT) {
                *result = originalRet;
                return true;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (full) {
                *result = HTCLIENT;
                return true;
            }
            if (max) {
                *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                return true;
            }
            if (isTop) {
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
            if (full) {
                *result = HTCLIENT;
                return true;
            }
            if (max) {
                *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                return true;
            }
            RECT clientRect = {0, 0, 0, 0};
            if (GetClientRect(hWnd, &clientRect) == FALSE) {
                qWarning() << Utils::getSystemErrorMessage(kGetClientRect);
                break;
            }
            const LONG width = clientRect.right;
            const LONG height = clientRect.bottom;
            const bool isBottom = (localPos.y >= (height - frameSizeY));
            // Make the border a little wider to let the user easy to resize on corners.
            const qreal scaleFactor = ((isTop || isBottom) ? 2.0 : 1.0);
            const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
            const auto scaledFrameSizeX = static_cast<int>(qRound(qreal(frameSizeX) * scaleFactor));
            const bool isLeft = (localPos.x < scaledFrameSizeX);
            const bool isRight = (localPos.x >= (width - scaledFrameSizeX));
            if (isTop) {
                if (isLeft) {
                    *result = HTTOPLEFT;
                    return true;
                }
                if (isRight) {
                    *result = HTTOPRIGHT;
                    return true;
                }
                *result = HTTOP;
                return true;
            }
            if (isBottom) {
                if (isLeft) {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                if (isRight) {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                *result = HTBOTTOM;
                return true;
            }
            if (isLeft) {
                *result = HTLEFT;
                return true;
            }
            if (isRight) {
                *result = HTRIGHT;
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        }
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 2))
    case WM_WINDOWPOSCHANGING: {
        // Tell Windows to discard the entire contents of the client area, as re-using
        // parts of the client area would lead to jitter during resize.
        const auto windowPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        windowPos->flags |= SWP_NOCOPYBITS;
    } break;
#endif
    case WM_DPICHANGED: {
        // Sync the internal window frame margins with the latest DPI.
        Utils::updateInternalWindowFrameMargins(data.params.getWindowHandle(), true);
    } break;
    case WM_DWMCOMPOSITIONCHANGED: {
        // Re-apply the custom window frame if recovered from the basic theme.
        Utils::updateWindowFrameMargins(windowId, false);
    } break;
    default:
        break;
    }
    if (!frameBorderVisible) {
        switch (uMsg) {
        case WM_NCUAHDRAWCAPTION:
        case WM_NCUAHDRAWFRAME: {
            // These undocumented messages are sent to draw themed window
            // borders. Block them to prevent drawing borders over the client
            // area.
            *result = 0;
            return true;
        }
        case WM_NCPAINT: {
            // 边框阴影处于非客户区的范围，因此如果直接阻止非客户区的绘制，会导致边框阴影丢失

            if (!Utils::isDwmCompositionEnabled()) {
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
            if (Utils::isDwmCompositionEnabled()) {
                // DefWindowProc won't repaint the window border if lParam (normally a HRGN)
                // is -1. See the following link's "lParam" section:
                // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
                // Don't use "*result = 0" here, otherwise the window won't respond to the
                // window activation state change.
                *result = DefWindowProcW(hWnd, WM_NCACTIVATE, wParam, -1);
            } else {
                if (static_cast<BOOL>(wParam) == FALSE) {
                    *result = TRUE;
                } else {
                    *result = FALSE;
                }
            }
            return true;
        }
        case WM_SETICON:
        case WM_SETTEXT: {
            // Disable painting while these messages are handled to prevent them
            // from drawing a window caption over the client area.
            SetLastError(ERROR_SUCCESS);
            const auto oldStyle = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
            if (oldStyle == 0) {
                qWarning() << Utils::getSystemErrorMessage(kGetWindowLongPtrW);
                break;
            }
            // Prevent Windows from drawing the default title bar by temporarily
            // toggling the WS_VISIBLE style.
            const DWORD newStyle = (oldStyle & ~WS_VISIBLE);
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLongPtrW(hWnd, GWL_STYLE, static_cast<LONG_PTR>(newStyle)) == 0) {
                qWarning() << Utils::getSystemErrorMessage(kSetWindowLongPtrW);
                break;
            }
            Utils::triggerFrameChange(windowId);
            const LRESULT ret = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLongPtrW(hWnd, GWL_STYLE, static_cast<LONG_PTR>(oldStyle)) == 0) {
                qWarning() << Utils::getSystemErrorMessage(kSetWindowLongPtrW);
                break;
            }
            Utils::triggerFrameChange(windowId);
            *result = ret;
            return true;
        }
        default:
            break;
        }
    }
    bool systemThemeChanged = ((uMsg == WM_THEMECHANGED) || (uMsg == WM_SYSCOLORCHANGE)
                               || (uMsg == WM_DWMCOLORIZATIONCOLORCHANGED));
    if (Utils::isWin10_1607OrGreater()) {
        if (uMsg == WM_SETTINGCHANGE) {
            if ((wParam == 0) && (QString::fromWCharArray(reinterpret_cast<LPCWSTR>(lParam))
                                      .compare(qThemeSettingChangeEventName, Qt::CaseInsensitive) == 0)) {
                systemThemeChanged = true;
                const bool dark = Utils::shouldAppsUseDarkMode();
                if (!(data.settings.options & Option::DontTouchWindowFrameBorderColor)) {
                    Utils::updateWindowFrameBorderColor(windowId, dark);
                }
                if (Utils::isWin10_1809OrGreater()) {
                    if (data.settings.options & Option::SyncNativeControlsThemeWithSystem) {
                        Utils::updateGlobalWin32ControlsTheme(windowId, dark);
                    }
                }
            }
        }
    }
    if (systemThemeChanged) {
        FramelessWindowsManager *manager = FramelessWindowsManager::instance();
        FramelessWindowsManagerPrivate *managerPriv = FramelessWindowsManagerPrivate::get(manager);
        managerPriv->notifySystemThemeHasChangedOrNot();
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
