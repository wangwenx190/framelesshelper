/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include "winnativeeventfilter.h"

#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QOperatingSystemVersion>
#include <cmath>
#include <d2d1.h>

#ifndef GET_X_LPARAM
// Only available since Windows 2000
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
// Only available since Windows 2000
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef ABM_GETSTATE
// Only available since Windows XP
#define ABM_GETSTATE 0x00000004
#endif

#ifndef ABM_GETTASKBARPOS
// Only available since Windows XP
#define ABM_GETTASKBARPOS 0x00000005
#endif

#ifndef ABS_AUTOHIDE
// Only available since Windows XP
#define ABS_AUTOHIDE 0x0000001
#endif

#ifndef ABE_LEFT
// Only available since Windows XP
#define ABE_LEFT 0
#endif

#ifndef ABE_TOP
// Only available since Windows XP
#define ABE_TOP 1
#endif

#ifndef ABE_RIGHT
// Only available since Windows XP
#define ABE_RIGHT 2
#endif

#ifndef ABE_BOTTOM
// Only available since Windows XP
#define ABE_BOTTOM 3
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
// Only available since Windows Vista
#define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER 92
#endif

#ifndef WM_NCUAHDRAWCAPTION
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWCAPTION 0x00AE
#endif

#ifndef WM_NCUAHDRAWFRAME
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWFRAME 0x00AF
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
// Only available since Windows Vista
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif

#ifndef WM_DPICHANGED
// Only available since Windows 8.1
#define WM_DPICHANGED 0x02E0
#endif

#ifndef ABM_GETAUTOHIDEBAREX
// Only available since Windows 8.1
#define ABM_GETAUTOHIDEBAREX 0x0000000b
#endif

#ifdef IsMinimized
#undef IsMinimized
#endif

#define IsMinimized m_lpIsIconic

#ifdef IsMaximized
#undef IsMaximized
#endif

#define IsMaximized m_lpIsZoomed

#ifndef WNEF_GENERATE_WINAPI
#define WNEF_GENERATE_WINAPI(funcName, resultType, ...)                        \
    using _WNEF_WINAPI_##funcName = resultType(WINAPI *)(__VA_ARGS__);         \
    _WNEF_WINAPI_##funcName m_lp##funcName = nullptr;
#endif

#ifndef WNEF_RESOLVE_WINAPI
#define WNEF_RESOLVE_WINAPI(libName, funcName)                                 \
    if (!m_lp##funcName) {                                                     \
        QLibrary library(QString::fromUtf8(#libName));                         \
        m_lp##funcName = reinterpret_cast<_WNEF_WINAPI_##funcName>(            \
            library.resolve(#funcName));                                       \
        Q_ASSERT_X(m_lp##funcName, __FUNCTION__,                               \
                   qUtf8Printable(library.errorString()));                     \
    }
#endif

namespace {

using MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
};

using DWMNCRENDERINGPOLICY = enum _DWMNCRENDERINGPOLICY { DWMNCRP_ENABLED = 2 };

using DWMWINDOWATTRIBUTE = enum _DWMWINDOWATTRIBUTE {
    DWMWA_NCRENDERING_POLICY = 2
};

using MARGINS = struct _MARGINS {
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
};

using APPBARDATA = struct _APPBARDATA {
    DWORD cbSize;
    HWND hWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
    LPARAM lParam;
};

using PROCESS_DPI_AWARENESS = enum _PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
};

WNEF_GENERATE_WINAPI(GetSystemDpiForProcess, UINT, HANDLE)
WNEF_GENERATE_WINAPI(GetDpiForWindow, UINT, HWND)
WNEF_GENERATE_WINAPI(GetDpiForSystem, UINT)
WNEF_GENERATE_WINAPI(GetSystemMetricsForDpi, int, int, UINT)
WNEF_GENERATE_WINAPI(GetDpiForMonitor, HRESULT, HMONITOR, MONITOR_DPI_TYPE,
                     UINT *, UINT *)
WNEF_GENERATE_WINAPI(DwmExtendFrameIntoClientArea, HRESULT, HWND,
                     CONST MARGINS *)
WNEF_GENERATE_WINAPI(DwmIsCompositionEnabled, HRESULT, BOOL *)
WNEF_GENERATE_WINAPI(DwmSetWindowAttribute, HRESULT, HWND, DWORD, LPCVOID,
                     DWORD)
WNEF_GENERATE_WINAPI(SHAppBarMessage, UINT_PTR, DWORD, APPBARDATA *)
WNEF_GENERATE_WINAPI(GetDeviceCaps, int, HDC, int)
WNEF_GENERATE_WINAPI(DefWindowProcW, LRESULT, HWND, UINT, WPARAM, LPARAM)
WNEF_GENERATE_WINAPI(SetLayeredWindowAttributes, BOOL, HWND, COLORREF, BYTE,
                     DWORD)
WNEF_GENERATE_WINAPI(MoveWindow, BOOL, HWND, int, int, int, int, BOOL)
WNEF_GENERATE_WINAPI(IsZoomed, BOOL, HWND)
WNEF_GENERATE_WINAPI(IsIconic, BOOL, HWND)
WNEF_GENERATE_WINAPI(GetSystemMetrics, int, int)
WNEF_GENERATE_WINAPI(GetDC, HDC, HWND)
WNEF_GENERATE_WINAPI(ReleaseDC, int, HWND, HDC)
WNEF_GENERATE_WINAPI(RedrawWindow, BOOL, HWND, CONST RECT *, HRGN, UINT)
WNEF_GENERATE_WINAPI(GetClientRect, BOOL, HWND, LPRECT)
WNEF_GENERATE_WINAPI(GetWindowRect, BOOL, HWND, LPRECT)
WNEF_GENERATE_WINAPI(ScreenToClient, BOOL, HWND, LPPOINT)
WNEF_GENERATE_WINAPI(EqualRect, BOOL, CONST RECT *, CONST RECT *)
#ifdef Q_PROCESSOR_X86_64
WNEF_GENERATE_WINAPI(GetWindowLongPtrW, LONG_PTR, HWND, int)
WNEF_GENERATE_WINAPI(SetWindowLongPtrW, LONG_PTR, HWND, int, LONG_PTR)
WNEF_GENERATE_WINAPI(SetClassLongPtrW, ULONG_PTR, HWND, int, LONG_PTR)
#else
#define LONG_PTR LONG
WNEF_GENERATE_WINAPI(GetWindowLongW, LONG_PTR, HWND, int)
WNEF_GENERATE_WINAPI(SetWindowLongW, LONG_PTR, HWND, int, LONG_PTR)
#define m_lpGetWindowLongPtrW m_lpGetWindowLongW
#define m_lpSetWindowLongPtrW m_lpSetWindowLongW
WNEF_GENERATE_WINAPI(SetClassLongW, DWORD, HWND, int, LONG_PTR)
#define m_lpSetClassLongPtrW m_lpSetClassLongW
#endif
WNEF_GENERATE_WINAPI(FindWindowW, HWND, LPCWSTR, LPCWSTR)
WNEF_GENERATE_WINAPI(MonitorFromWindow, HMONITOR, HWND, DWORD)
WNEF_GENERATE_WINAPI(GetMonitorInfoW, BOOL, HMONITOR, LPMONITORINFO)
WNEF_GENERATE_WINAPI(GetAncestor, HWND, HWND, UINT)
WNEF_GENERATE_WINAPI(GetDesktopWindow, HWND)
WNEF_GENERATE_WINAPI(SendMessageW, LRESULT, HWND, UINT, WPARAM, LPARAM)
WNEF_GENERATE_WINAPI(SetWindowPos, BOOL, HWND, HWND, int, int, int, int, UINT)
WNEF_GENERATE_WINAPI(UpdateWindow, BOOL, HWND)
WNEF_GENERATE_WINAPI(InvalidateRect, BOOL, HWND, CONST RECT *, BOOL)
WNEF_GENERATE_WINAPI(SetWindowRgn, int, HWND, HRGN, BOOL)
WNEF_GENERATE_WINAPI(IsWindow, BOOL, HWND)
WNEF_GENERATE_WINAPI(GetWindowInfo, BOOL, HWND, LPWINDOWINFO)
WNEF_GENERATE_WINAPI(CreateSolidBrush, HBRUSH, COLORREF)
WNEF_GENERATE_WINAPI(FillRect, int, HDC, CONST RECT *, HBRUSH)
WNEF_GENERATE_WINAPI(DeleteObject, BOOL, HGDIOBJ)
WNEF_GENERATE_WINAPI(IsThemeActive, BOOL)
WNEF_GENERATE_WINAPI(BeginPaint, HDC, HWND, LPPAINTSTRUCT)
WNEF_GENERATE_WINAPI(EndPaint, BOOL, HWND, CONST PAINTSTRUCT *)
WNEF_GENERATE_WINAPI(GetCurrentProcess, HANDLE)
WNEF_GENERATE_WINAPI(GetProcessDpiAwareness, HRESULT, HANDLE,
                     PROCESS_DPI_AWARENESS *)
WNEF_GENERATE_WINAPI(IsProcessDPIAware, BOOL)
WNEF_GENERATE_WINAPI(D2D1CreateFactory, HRESULT, D2D1_FACTORY_TYPE, REFIID,
                     CONST D2D1_FACTORY_OPTIONS *, void **)

BOOL IsDwmCompositionEnabled() {
    // Since Win8, DWM composition is always enabled and can't be disabled.
    BOOL enabled = FALSE;
    return SUCCEEDED(m_lpDwmIsCompositionEnabled(&enabled)) && enabled;
}

BOOL IsFullScreened(HWND handle) {
    if (handle && m_lpIsWindow(handle)) {
        WINDOWINFO windowInfo;
        SecureZeroMemory(&windowInfo, sizeof(windowInfo));
        windowInfo.cbSize = sizeof(windowInfo);
        m_lpGetWindowInfo(handle, &windowInfo);
        MONITORINFO monitorInfo;
        SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
        monitorInfo.cbSize = sizeof(monitorInfo);
        const HMONITOR monitor =
            m_lpMonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        m_lpGetMonitorInfoW(monitor, &monitorInfo);
        // The only way to judge whether a window is fullscreened or not
        // is to compare it's size with the screen's size, there is no official
        // Win32 API to do this for us.
        return m_lpEqualRect(&windowInfo.rcWindow, &monitorInfo.rcMonitor) ||
            m_lpEqualRect(&windowInfo.rcClient, &monitorInfo.rcMonitor);
    }
    return FALSE;
}

BOOL IsWindowTopLevel(HWND handle) {
    if (handle && m_lpIsWindow(handle)) {
        if (m_lpGetWindowLongPtrW(handle, GWL_STYLE) & WS_CHILD) {
            return FALSE;
        }
        const HWND parent = m_lpGetAncestor(handle, GA_PARENT);
        if (parent && (parent != m_lpGetDesktopWindow())) {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

// The thickness of an auto-hide taskbar in pixels.
const int kAutoHideTaskbarThicknessPx = 2;
const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

const UINT m_defaultDotsPerInch = USER_DEFAULT_SCREEN_DPI;

const qreal m_defaultDevicePixelRatio = 1.0;

int m_borderWidth = -1, m_borderHeight = -1, m_titlebarHeight = -1;

QScopedPointer<WinNativeEventFilter> m_instance;

QVector<HWND> m_framelessWindows;

} // namespace

WinNativeEventFilter::WinNativeEventFilter() { initWin32Api(); }

WinNativeEventFilter::~WinNativeEventFilter() = default;

void WinNativeEventFilter::install() {
    if (m_instance.isNull()) {
        m_instance.reset(new WinNativeEventFilter);
        qApp->installNativeEventFilter(m_instance.data());
    }
}

void WinNativeEventFilter::uninstall() {
    if (!m_instance.isNull()) {
        qApp->removeNativeEventFilter(m_instance.data());
        m_instance.reset();
    }
    if (!m_framelessWindows.isEmpty()) {
        m_framelessWindows.clear();
    }
}

QVector<HWND> WinNativeEventFilter::framelessWindows() {
    return m_framelessWindows;
}

void WinNativeEventFilter::setFramelessWindows(QVector<HWND> windows) {
    if (!windows.isEmpty() && (windows != m_framelessWindows)) {
        m_framelessWindows = windows;
        install();
    }
}

void WinNativeEventFilter::addFramelessWindow(HWND window,
                                              const WINDOWDATA *data) {
    initWin32Api();
    if (window && m_lpIsWindow(window) &&
        !m_framelessWindows.contains(window)) {
        m_framelessWindows.append(window);
        if (data) {
            createUserData(window, data);
        }
        install();
    }
}

void WinNativeEventFilter::removeFramelessWindow(HWND window) {
    if (window && m_framelessWindows.contains(window)) {
        m_framelessWindows.removeAll(window);
    }
}

void WinNativeEventFilter::clearFramelessWindows() {
    if (!m_framelessWindows.isEmpty()) {
        m_framelessWindows.clear();
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool WinNativeEventFilter::nativeEventFilter(const QByteArray &eventType,
                                             void *message, qintptr *result)
#else
bool WinNativeEventFilter::nativeEventFilter(const QByteArray &eventType,
                                             void *message, long *result)
#endif
{
    Q_UNUSED(eventType)
    const auto msg = static_cast<LPMSG>(message);
    if (!msg->hwnd) {
        // Why sometimes the window handle is null? Is it designed to be?
        // Anyway, we should skip it in this case.
        return false;
    }
    if (!m_lpIsWindow(msg->hwnd)) {
        return false;
    }
    if (m_framelessWindows.isEmpty()) {
        // Only top level windows can be frameless.
        if (!IsWindowTopLevel(msg->hwnd)) {
            return false;
        }
    } else if (!m_framelessWindows.contains(msg->hwnd)) {
        return false;
    }
    createUserData(msg->hwnd);
    const auto data = reinterpret_cast<WINDOW *>(
        m_lpGetWindowLongPtrW(msg->hwnd, GWLP_USERDATA));
    if (!data->initialized) {
        // Avoid initializing a same window twice.
        data->initialized = TRUE;
        // Restore default window style.
        m_lpSetWindowLongPtrW(msg->hwnd, GWL_STYLE,
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
                                  WS_CLIPSIBLINGS);
        // The following two lines can help us get rid of the three system
        // buttons (minimize, maximize and close). But they also break the
        // Arcylic effect (introduced in Win10 1709), don't know why.
        m_lpSetWindowLongPtrW(msg->hwnd, GWL_EXSTYLE,
                              WS_EX_APPWINDOW | WS_EX_LAYERED);
        m_lpSetLayeredWindowAttributes(msg->hwnd, RGB(255, 0, 255), 0,
                                       LWA_COLORKEY);
        // Make sure our window have it's frame shadow.
        // The frame shadow is drawn by Desktop Window Manager (DWM), don't draw
        // it yourself. The frame shadow will get lost if DWM composition is
        // disabled, it's designed to be, don't force the window to draw a frame
        // shadow in that case.
        updateGlass(msg->hwnd);
#ifdef _DEBUG
        // For debug purposes.
        qDebug().noquote() << "Window handle:" << msg->hwnd;
        qDebug().noquote() << "Window DPI:"
                           << getDotsPerInchForWindow(msg->hwnd)
                           << "Window DPR:"
                           << getDevicePixelRatioForWindow(msg->hwnd);
        qDebug().noquote()
            << "Window border width:"
            << getSystemMetric(msg->hwnd, SystemMetric::BorderWidth, false)
            << "Window border height:"
            << getSystemMetric(msg->hwnd, SystemMetric::BorderHeight, false)
            << "Window titlebar height:"
            << getSystemMetric(msg->hwnd, SystemMetric::TitleBarHeight, false);
#endif
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
        // 小于屏幕尺寸。我下面的做法，参考了火狐和Chromium。但是如果没有开启任务
        // 栏自动隐藏，是不存在这个问题的，所以要先进行判断。
        // 一般情况下，*result设置为0（相当于DefWindowProc的返回值为0）就可以了，
        // 根据MSDN的说法，返回0意为此消息已经被程序自行处理了，让Windows跳过此消
        // 息，否则Windows会添加对此消息的默认处理，对于当前这个消息而言，就意味着
        // 标题栏和窗口边框又会回来，这当然不是我们想要的结果。根据MSDN，当wParam
        // 为FALSE时，只能返回0，但当其为TRUE时，可以返回0，也可以返回一个WVR_常
        // 量。根据Chromium的注释，当存在非客户区时，如果返回WVR_REDRAW会导致子
        // 窗口/子控件出现奇怪的bug（自绘控件错位），并且Lucas在Windows 10上成
        // 功复现，说明这个bug至今都没有解决。因为这个是Windows自身的bug，且已经
        // 存在很久，这种情况下只有返回0才能规避这个问题。但如果不存在非客户区，且
        // wParam为TRUE，最好返回WVR_REDRAW，否则窗口在调整大小可能会产生严重的
        // 闪烁现象。
        // 虽然对大多数消息来说，返回0都代表让Windows忽略此消息，但实际上不同消息
        // 能接受的返回值是不一样的，请注意自行查阅MSDN。

        // Sent when the size and position of a window's client area must be
        // calculated. By processing this message, an application can control
        // the content of the window's client area when the size or position of
        // the window changes. If wParam is TRUE, lParam points to an
        // NCCALCSIZE_PARAMS structure that contains information an application
        // can use to calculate the new size and position of the client
        // rectangle. If wParam is FALSE, lParam points to a RECT structure. On
        // entry, the structure contains the proposed window rectangle for the
        // window. On exit, the structure should contain the screen coordinates
        // of the corresponding window client area.
        const auto getClientAreaInsets = [](HWND _hWnd) -> RECT {
            if (IsMaximized(_hWnd) || IsFullScreened(_hWnd)) {
                // Windows automatically adds a standard width border to all
                // sides when a window is maximized.
                int frameThickness_x =
                    getSystemMetric(_hWnd, SystemMetric::BorderWidth);
                int frameThickness_y =
                    getSystemMetric(_hWnd, SystemMetric::BorderHeight);
                // The following two lines are two seperate functions in
                // Chromium, it uses them to judge whether the window
                // should draw it's own frame or not. But here we will always
                // draw our own frame because our window is totally frameless,
                // so we can simply use constants here. I don't remove them
                // completely because I don't want to forget what it's about to
                // achieve.
                const bool removeStandardFrame = true;
                const bool hasFrame = !removeStandardFrame;
                if (!hasFrame) {
                    frameThickness_x -= 1;
                    frameThickness_y -= 1;
                }
                RECT rect;
                rect.top = frameThickness_y;
                rect.bottom = frameThickness_y;
                rect.left = frameThickness_x;
                rect.right = frameThickness_x;
                return rect;
            }
            return {0, 0, 0, 0};
        };
        const RECT insets = getClientAreaInsets(msg->hwnd);
        const auto mode = static_cast<BOOL>(msg->wParam);
        // If the window bounds change, we're going to relayout and repaint
        // anyway. Returning WVR_REDRAW avoids an extra paint before that of the
        // old client pixels in the (now wrong) location, and thus makes actions
        // like resizing a window from the left edge look slightly less broken.
        *result = mode ? WVR_REDRAW : 0;
        const auto clientRect = mode
            ? &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam)->rgrc[0])
            : reinterpret_cast<LPRECT>(msg->lParam);
        clientRect->top += insets.top;
        clientRect->bottom -= insets.bottom;
        clientRect->left += insets.left;
        clientRect->right -= insets.right;
        if (IsMaximized(msg->hwnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = m_lpSHAppBarMessage(ABM_GETSTATE, &abd);
            if (taskbarState & ABS_AUTOHIDE) {
                const HMONITOR windowMonitor =
                    m_lpMonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                bool top = false, bottom = false, left = false, right = false;
                if (QOperatingSystemVersion::current() >=
                    QOperatingSystemVersion::Windows8_1) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    m_lpGetMonitorInfoW(windowMonitor, &monitorInfo);
                    const auto hasAutohideTaskbar =
                        [&monitorInfo](const UINT edge) -> bool {
                        APPBARDATA _abd;
                        SecureZeroMemory(&_abd, sizeof(_abd));
                        _abd.cbSize = sizeof(_abd);
                        _abd.uEdge = edge;
                        _abd.rc = monitorInfo.rcMonitor;
                        const auto hTaskbar = reinterpret_cast<HWND>(
                            m_lpSHAppBarMessage(ABM_GETAUTOHIDEBAREX, &_abd));
                        return hTaskbar != nullptr;
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
                    _abd.hWnd = m_lpFindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR taskbarMonitor = m_lpMonitorFromWindow(
                            _abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (taskbarMonitor == windowMonitor) {
                            m_lpSHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    }
                    top = edge == ABE_TOP;
                    bottom = edge == ABE_BOTTOM;
                    left = edge == ABE_LEFT;
                    right = edge == ABE_RIGHT;
                }
                if (top) {
                    clientRect->top += kAutoHideTaskbarThicknessPy;
                } else if (bottom) {
                    clientRect->bottom -= kAutoHideTaskbarThicknessPy;
                } else if (left) {
                    clientRect->left += kAutoHideTaskbarThicknessPx;
                } else if (right) {
                    clientRect->right -= kAutoHideTaskbarThicknessPx;
                }
            }
            // We cannot return WVR_REDRAW when there is nonclient area, or
            // Windows exhibits bugs where client pixels and child HWNDs are
            // mispositioned by the width/height of the upper-left nonclient
            // area.
            *result = 0;
        }
        return true;
    }
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

        if (IsDwmCompositionEnabled()) {
            break;
        } else {
            // Only block WM_NCPAINT when DWM composition is disabled. If it's
            // blocked when DWM composition is enabled, the frame shadow won't
            // be drawn.
            *result = 0;
            return true;
        }
    }
    case WM_NCACTIVATE: {
        // DefWindowProc won't repaint the window border if lParam (normally
        // a HRGN) is -1.
        // Don't use "*result = 0" otherwise the window won't respond to
        // the window active state change.
        *result = m_lpDefWindowProcW(msg->hwnd, msg->message, msg->wParam, -1);
        return true;
    }
    case WM_NCHITTEST: {
        // 测试了一下原生Win32窗口，发现只有顶边是在窗口内部resize的，其余三边
        // 都是在窗口外部进行resize的，但WM_NCHITTEST这个消息只有在鼠标进入窗
        // 口的范围内才会被触发，因此没法在这个消息里实现窗口外部的resize。火狐
        // 和Chromium是怎么做到的？Hook鼠标事件？

        if (data->windowData.mouseTransparent) {
            // Mouse events will be passed to the parent window.
            *result = HTTRANSPARENT;
            return true;
        }
        const auto getHTResult = [](HWND _hWnd, LPARAM _lParam,
                                    const WINDOW *_data) -> LRESULT {
            const auto isInSpecificAreas = [](int x, int y,
                                              const QVector<QRect> &areas,
                                              qreal dpr) -> bool {
                for (auto &&area : std::as_const(areas)) {
                    if (!area.isValid()) {
                        continue;
                    }
                    if (QRect(std::round(area.x() * dpr),
                              std::round(area.y() * dpr),
                              std::round(area.width() * dpr),
                              std::round(area.height() * dpr))
                            .contains(x, y, true)) {
                        return true;
                    }
                }
                return false;
            };
            RECT clientRect = {0, 0, 0, 0};
            m_lpGetClientRect(_hWnd, &clientRect);
            const LONG ww = clientRect.right;
            const LONG wh = clientRect.bottom;
            POINT mouse;
            // Don't use HIWORD(lParam) and LOWORD(lParam) to get cursor
            // coordinates because their results are unsigned numbers, however
            // the cursor position may be negative due to in a different
            // monitor.
            mouse.x = GET_X_LPARAM(_lParam);
            mouse.y = GET_Y_LPARAM(_lParam);
            m_lpScreenToClient(_hWnd, &mouse);
            // These values are DPI-aware.
            const LONG bw = getSystemMetric(_hWnd, SystemMetric::BorderWidth);
            const LONG bh = getSystemMetric(_hWnd, SystemMetric::BorderHeight);
            const LONG tbh =
                getSystemMetric(_hWnd, SystemMetric::TitleBarHeight);
            const qreal dpr = getDevicePixelRatioForWindow(_hWnd);
            const bool isTitlebar = (mouse.y < tbh) &&
                !isInSpecificAreas(mouse.x, mouse.y,
                                   _data->windowData.ignoreAreas, dpr) &&
                (_data->windowData.draggableAreas.isEmpty()
                     ? true
                     : isInSpecificAreas(mouse.x, mouse.y,
                                         _data->windowData.draggableAreas,
                                         dpr));
            if (IsMaximized(_hWnd)) {
                if (isTitlebar) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            }
            const bool isTop = mouse.y < bh;
            const bool isBottom = mouse.y > (wh - bh);
            // Make the border wider to let the user easy to resize on corners.
            const int factor = (isTop || isBottom) ? 2 : 1;
            const bool isLeft = mouse.x < (bw * factor);
            const bool isRight = mouse.x > (ww - (bw * factor));
            const bool fixedSize = _data->windowData.fixedSize;
            const auto getBorderValue = [fixedSize](int value) -> int {
                // HTBORDER: non-resizeable window border.
                return fixedSize ? HTBORDER : value;
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
            if (isTitlebar) {
                return HTCAPTION;
            }
            return HTCLIENT;
        };
        *result = getHTResult(msg->hwnd, msg->lParam, data);
        return true;
    }
    case WM_GETMINMAXINFO: {
        // Don't cover the taskbar when maximized.
        const HMONITOR monitor =
            m_lpMonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
        monitorInfo.cbSize = sizeof(monitorInfo);
        m_lpGetMonitorInfoW(monitor, &monitorInfo);
        const RECT rcWorkArea = monitorInfo.rcWork;
        const RECT rcMonitorArea = monitorInfo.rcMonitor;
        auto &mmi = *reinterpret_cast<LPMINMAXINFO>(msg->lParam);
        if (QOperatingSystemVersion::current() <
            QOperatingSystemVersion::Windows8) {
            // FIXME: Buggy on Windows 7:
            // The origin of coordinates is the top left edge of the
            // monitor's work area. Why? It should be the top left edge of
            // the monitor's area.
            mmi.ptMaxPosition.x = rcMonitorArea.left;
            mmi.ptMaxPosition.y = rcMonitorArea.top;
        } else {
            // Works fine on Windows 8/8.1/10
            mmi.ptMaxPosition.x =
                std::abs(rcWorkArea.left - rcMonitorArea.left);
            mmi.ptMaxPosition.y = std::abs(rcWorkArea.top - rcMonitorArea.top);
        }
        if (data->windowData.maximumSize.isEmpty()) {
            mmi.ptMaxSize.x = std::abs(rcWorkArea.right - rcWorkArea.left);
            mmi.ptMaxSize.y = std::abs(rcWorkArea.bottom - rcWorkArea.top);
        } else {
            mmi.ptMaxSize.x =
                std::round(getDevicePixelRatioForWindow(msg->hwnd) *
                           data->windowData.maximumSize.width());
            mmi.ptMaxSize.y =
                std::round(getDevicePixelRatioForWindow(msg->hwnd) *
                           data->windowData.maximumSize.height());
        }
        mmi.ptMaxTrackSize.x = mmi.ptMaxSize.x;
        mmi.ptMaxTrackSize.y = mmi.ptMaxSize.y;
        if (!data->windowData.minimumSize.isEmpty()) {
            mmi.ptMinTrackSize.x =
                std::round(getDevicePixelRatioForWindow(msg->hwnd) *
                           data->windowData.minimumSize.width());
            mmi.ptMinTrackSize.y =
                std::round(getDevicePixelRatioForWindow(msg->hwnd) *
                           data->windowData.minimumSize.height());
        }
        *result = 0;
        return true;
    }
    case WM_SETICON:
    case WM_SETTEXT: {
        // Disable painting while these messages are handled to prevent them
        // from drawing a window caption over the client area.
        const auto oldStyle = m_lpGetWindowLongPtrW(msg->hwnd, GWL_STYLE);
        // Prevent Windows from drawing the default title bar by temporarily
        // toggling the WS_VISIBLE style.
        m_lpSetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle & ~WS_VISIBLE);
        const LRESULT ret = m_lpDefWindowProcW(msg->hwnd, msg->message,
                                               msg->wParam, msg->lParam);
        m_lpSetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
        *result = ret;
        return true;
    }
    case WM_DWMCOMPOSITIONCHANGED: {
        updateGlass(msg->hwnd);
        break;
    }
    default: {
        break;
    }
    }
    return false;
}

void WinNativeEventFilter::updateGlass(HWND handle) {
    MARGINS margins = {0, 0, 0, 0};
    if (IsDwmCompositionEnabled()) {
        // The frame shadow is drawn on the non-client area and thus we have
        // to make sure the non-client area rendering is enabled first.
        const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        m_lpDwmSetWindowAttribute(handle, DWMWA_NCRENDERING_POLICY, &ncrp,
                                  sizeof(ncrp));
        // Negative margins have special meaning to
        // DwmExtendFrameIntoClientArea. Negative margins create the "sheet of
        // glass" effect, where the client area is rendered as a solid surface
        // with no window border.
        // Use positive margins have similar appearance, but the window
        // background will be transparent, we don't want that.
        margins = {-1, -1, -1, -1};
    }
    m_lpDwmExtendFrameIntoClientArea(handle, &margins);
    updateWindow(handle);
}

UINT WinNativeEventFilter::getDotsPerInchForWindow(HWND handle) {
    const auto getScreenDpi = [](UINT defaultValue) -> UINT {
        if (m_lpD2D1CreateFactory) {
            // Using Direct2D to get the screen DPI.
            // Available since Windows 7.
            ID2D1Factory *m_pDirect2dFactory = nullptr;
            if (SUCCEEDED(m_lpD2D1CreateFactory(
                    D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory),
                    nullptr, reinterpret_cast<void **>(&m_pDirect2dFactory))) &&
                m_pDirect2dFactory) {
                m_pDirect2dFactory->ReloadSystemMetrics();
                FLOAT dpiX = defaultValue, dpiY = defaultValue;
                m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
                // The values of *dpiX and *dpiY are identical.
                return std::round(dpiX);
            }
        }
        // Available since Windows 2000.
        const HDC hdc = m_lpGetDC(nullptr);
        if (hdc) {
            const int dpiX = m_lpGetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = m_lpGetDeviceCaps(hdc, LOGPIXELSY);
            m_lpReleaseDC(nullptr, hdc);
            // The values of dpiX and dpiY are identical actually, just to
            // silence a compiler warning.
            return dpiX == dpiY ? dpiY : dpiX;
        }
        return defaultValue;
    };
    bool dpiEnabled = false;
    if (m_lpGetProcessDpiAwareness) {
        PROCESS_DPI_AWARENESS awareness = PROCESS_DPI_UNAWARE;
        m_lpGetProcessDpiAwareness(m_lpGetCurrentProcess(), &awareness);
        dpiEnabled = awareness != PROCESS_DPI_UNAWARE;
    } else if (m_lpIsProcessDPIAware) {
        dpiEnabled = m_lpIsProcessDPIAware();
    }
    if (!dpiEnabled) {
        // Return hard-coded DPI if DPI scaling is disabled.
        return m_defaultDotsPerInch;
    }
    if (!handle) {
        if (m_lpGetSystemDpiForProcess) {
            return m_lpGetSystemDpiForProcess(m_lpGetCurrentProcess());
        } else if (m_lpGetDpiForSystem) {
            return m_lpGetDpiForSystem();
        }
        return getScreenDpi(m_defaultDotsPerInch);
    }
    if (m_lpGetDpiForWindow) {
        return m_lpGetDpiForWindow(handle);
    }
    if (m_lpGetDpiForMonitor) {
        UINT dpiX = m_defaultDotsPerInch, dpiY = m_defaultDotsPerInch;
        m_lpGetDpiForMonitor(
            m_lpMonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST),
            MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        // The values of *dpiX and *dpiY are identical.
        return dpiX;
    }
    return getScreenDpi(m_defaultDotsPerInch);
}

qreal WinNativeEventFilter::getDevicePixelRatioForWindow(HWND handle) {
    const qreal dpr = handle
        ? (static_cast<qreal>(getDotsPerInchForWindow(handle)) /
           static_cast<qreal>(m_defaultDotsPerInch))
        : m_defaultDevicePixelRatio;
    return getPreferedNumber(dpr);
}

int WinNativeEventFilter::getSystemMetricsForWindow(HWND handle, int index) {
    if (m_lpGetSystemMetricsForDpi) {
        return m_lpGetSystemMetricsForDpi(
            index,
            static_cast<UINT>(std::round(
                getPreferedNumber(getDotsPerInchForWindow(handle)))));
    } else {
        return std::round(m_lpGetSystemMetrics(index) *
                          getDevicePixelRatioForWindow(handle));
    }
}

void WinNativeEventFilter::setWindowData(HWND window, const WINDOWDATA *data) {
    initWin32Api();
    if (window && m_lpIsWindow(window) && data) {
        createUserData(window, data);
    }
}

WinNativeEventFilter::WINDOWDATA *
WinNativeEventFilter::windowData(HWND window) {
    initWin32Api();
    if (window && m_lpIsWindow(window)) {
        createUserData(window);
        return &reinterpret_cast<WINDOW *>(
                    m_lpGetWindowLongPtrW(window, GWLP_USERDATA))
                    ->windowData;
    }
    return nullptr;
}

void WinNativeEventFilter::createUserData(HWND handle, const WINDOWDATA *data) {
    if (handle) {
        const auto userData = reinterpret_cast<WINDOW *>(
            m_lpGetWindowLongPtrW(handle, GWLP_USERDATA));
        if (userData) {
            if (data) {
                userData->windowData = *data;
            }
        } else {
            WINDOW *_data = new WINDOW;
            _data->hWnd = handle;
            if (data) {
                _data->windowData = *data;
            }
            m_lpSetWindowLongPtrW(handle, GWLP_USERDATA,
                                  reinterpret_cast<LONG_PTR>(_data));
        }
    }
}

void WinNativeEventFilter::initWin32Api() {
    static bool resolved = false;
    if (resolved) {
        return;
    }
    resolved = true;
    // Available since Windows 2000.
    WNEF_RESOLVE_WINAPI(User32, EndPaint)
    WNEF_RESOLVE_WINAPI(User32, BeginPaint)
    WNEF_RESOLVE_WINAPI(User32, FillRect)
    WNEF_RESOLVE_WINAPI(User32, GetWindowInfo)
    WNEF_RESOLVE_WINAPI(User32, IsWindow)
    WNEF_RESOLVE_WINAPI(User32, SetWindowRgn)
    WNEF_RESOLVE_WINAPI(User32, InvalidateRect)
    WNEF_RESOLVE_WINAPI(User32, UpdateWindow)
    WNEF_RESOLVE_WINAPI(User32, SetWindowPos)
    WNEF_RESOLVE_WINAPI(User32, SendMessageW)
    WNEF_RESOLVE_WINAPI(User32, GetDesktopWindow)
    WNEF_RESOLVE_WINAPI(User32, GetAncestor)
    WNEF_RESOLVE_WINAPI(User32, DefWindowProcW)
    WNEF_RESOLVE_WINAPI(User32, SetLayeredWindowAttributes)
    WNEF_RESOLVE_WINAPI(User32, MoveWindow)
    WNEF_RESOLVE_WINAPI(User32, IsZoomed)
    WNEF_RESOLVE_WINAPI(User32, IsIconic)
    WNEF_RESOLVE_WINAPI(User32, GetSystemMetrics)
    WNEF_RESOLVE_WINAPI(User32, GetDC)
    WNEF_RESOLVE_WINAPI(User32, ReleaseDC)
    WNEF_RESOLVE_WINAPI(User32, RedrawWindow)
    WNEF_RESOLVE_WINAPI(User32, GetClientRect)
    WNEF_RESOLVE_WINAPI(User32, GetWindowRect)
    WNEF_RESOLVE_WINAPI(User32, ScreenToClient)
    WNEF_RESOLVE_WINAPI(User32, EqualRect)
#ifdef Q_PROCESSOR_X86_64
    WNEF_RESOLVE_WINAPI(User32, GetWindowLongPtrW)
    WNEF_RESOLVE_WINAPI(User32, SetWindowLongPtrW)
    WNEF_RESOLVE_WINAPI(User32, SetClassLongPtrW)
#else
    WNEF_RESOLVE_WINAPI(User32, GetWindowLongW)
    WNEF_RESOLVE_WINAPI(User32, SetWindowLongW)
    WNEF_RESOLVE_WINAPI(User32, SetClassLongW)
#endif
    WNEF_RESOLVE_WINAPI(User32, FindWindowW)
    WNEF_RESOLVE_WINAPI(User32, MonitorFromWindow)
    WNEF_RESOLVE_WINAPI(User32, GetMonitorInfoW)
    WNEF_RESOLVE_WINAPI(Gdi32, GetDeviceCaps)
    WNEF_RESOLVE_WINAPI(Gdi32, CreateSolidBrush)
    WNEF_RESOLVE_WINAPI(Gdi32, DeleteObject)
    // Available since Windows XP.
    WNEF_RESOLVE_WINAPI(Shell32, SHAppBarMessage)
    WNEF_RESOLVE_WINAPI(Kernel32, GetCurrentProcess)
    // Available since Windows Vista.
    WNEF_RESOLVE_WINAPI(User32, IsProcessDPIAware)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmIsCompositionEnabled)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmExtendFrameIntoClientArea)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmSetWindowAttribute)
    WNEF_RESOLVE_WINAPI(UxTheme, IsThemeActive)
    // Available since Windows 7.
    WNEF_RESOLVE_WINAPI(D2D1, D2D1CreateFactory)
    // Available since Windows 8.1
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows8_1) {
        WNEF_RESOLVE_WINAPI(SHCore, GetDpiForMonitor)
        WNEF_RESOLVE_WINAPI(SHCore, GetProcessDpiAwareness)
    }
    // Available since Windows 10, version 1607 (10.0.14393)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                14393)) {
        WNEF_RESOLVE_WINAPI(User32, GetDpiForWindow)
        WNEF_RESOLVE_WINAPI(User32, GetDpiForSystem)
        WNEF_RESOLVE_WINAPI(User32, GetSystemMetricsForDpi)
    }
    // Available since Windows 10, version 1803 (10.0.17134)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                17134)) {
        WNEF_RESOLVE_WINAPI(User32, GetSystemDpiForProcess)
    }
}

void WinNativeEventFilter::setBorderWidth(int bw) {
    if (m_borderWidth != bw) {
        m_borderWidth = bw;
    }
}

void WinNativeEventFilter::setBorderHeight(int bh) {
    if (m_borderHeight != bh) {
        m_borderHeight = bh;
    }
}

void WinNativeEventFilter::setTitlebarHeight(int tbh) {
    if (m_titlebarHeight != tbh) {
        m_titlebarHeight = tbh;
    }
}

qreal WinNativeEventFilter::getPreferedNumber(qreal num) {
    qreal result = -1.0;
    const auto getRoundedNumber = [](qreal in) -> qreal {
        // If the given number is not very large, we assume it's a
        // device pixel ratio (DPR), otherwise we assume it's a DPI.
        if (in < m_defaultDotsPerInch) {
            return std::round(in);
        } else {
            if (in < (m_defaultDotsPerInch * 1.5)) {
                return m_defaultDotsPerInch;
            } else if (in == (m_defaultDotsPerInch * 1.5)) {
                return m_defaultDotsPerInch * 1.5;
            } else if (in < (m_defaultDotsPerInch * 2.5)) {
                return m_defaultDotsPerInch * 2;
            } else if (in == (m_defaultDotsPerInch * 2.5)) {
                return m_defaultDotsPerInch * 2.5;
            } else if (in < (m_defaultDotsPerInch * 3.5)) {
                return m_defaultDotsPerInch * 3;
            } else if (in == (m_defaultDotsPerInch * 3.5)) {
                return m_defaultDotsPerInch * 3.5;
            } else if (in < (m_defaultDotsPerInch * 4.5)) {
                return m_defaultDotsPerInch * 4;
            } else {
                qWarning().noquote()
                    << "DPI too large:" << static_cast<int>(in);
            }
        }
        return -1.0;
    };
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    switch (QGuiApplication::highDpiScaleFactorRoundingPolicy()) {
    case Qt::HighDpiScaleFactorRoundingPolicy::PassThrough:
        // Default behavior for Qt 6.
        result = num;
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Floor:
        result = std::floor(num);
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Ceil:
        result = std::ceil(num);
        break;
    default:
        // Default behavior for Qt 5.6 to 5.15
        result = getRoundedNumber(num);
        break;
    }
#else
    // Default behavior for Qt 5.6 to 5.15
    result = getRoundedNumber(num);
#endif
    return result;
}

void WinNativeEventFilter::updateWindow(HWND handle, bool triggerFrameChange) {
    initWin32Api();
    if (handle && m_lpIsWindow(handle)) {
        if (triggerFrameChange) {
            m_lpSetWindowPos(handle, nullptr, 0, 0, 0, 0,
                             SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE |
                                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
        m_lpRedrawWindow(handle, nullptr, nullptr,
                         RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
    }
}

int WinNativeEventFilter::getSystemMetric(HWND handle, SystemMetric metric,
                                          bool dpiAware) {
    initWin32Api();
    const qreal dpr = dpiAware ? getDevicePixelRatioForWindow(handle)
                               : m_defaultDevicePixelRatio;
    if (handle && m_lpIsWindow(handle)) {
        createUserData(handle);
        const auto userData = reinterpret_cast<WINDOW *>(
            m_lpGetWindowLongPtrW(handle, GWLP_USERDATA));
        switch (metric) {
        case SystemMetric::BorderWidth: {
            const int bw = userData->windowData.borderWidth;
            if (bw > 0) {
                return std::round(bw * dpr);
            }
            break;
        }
        case SystemMetric::BorderHeight: {
            const int bh = userData->windowData.borderHeight;
            if (bh > 0) {
                return std::round(bh * dpr);
            }
            break;
        }
        case SystemMetric::TitleBarHeight: {
            const int tbh = userData->windowData.titlebarHeight;
            if (tbh > 0) {
                return std::round(tbh * dpr);
            }
            break;
        }
        }
    }
    switch (metric) {
    case SystemMetric::BorderWidth: {
        if (m_borderWidth > 0) {
            return std::round(m_borderWidth * dpr);
        } else {
            const int result = m_lpGetSystemMetrics(SM_CXSIZEFRAME) +
                m_lpGetSystemMetrics(SM_CXPADDEDBORDER);
            const int result_dpi =
                getSystemMetricsForWindow(handle, SM_CXSIZEFRAME) +
                getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
            return dpiAware ? result_dpi : result;
        }
    }
    case SystemMetric::BorderHeight: {
        if (m_borderHeight > 0) {
            return std::round(m_borderHeight * dpr);
        } else {
            const int result = m_lpGetSystemMetrics(SM_CYSIZEFRAME) +
                m_lpGetSystemMetrics(SM_CXPADDEDBORDER);
            const int result_dpi =
                getSystemMetricsForWindow(handle, SM_CYSIZEFRAME) +
                getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
            return dpiAware ? result_dpi : result;
        }
    }
    case SystemMetric::TitleBarHeight: {
        if (m_titlebarHeight > 0) {
            return std::round(m_titlebarHeight * dpr);
        } else {
            const int result = m_lpGetSystemMetrics(SM_CYSIZEFRAME) +
                m_lpGetSystemMetrics(SM_CXPADDEDBORDER) +
                m_lpGetSystemMetrics(SM_CYCAPTION);
            const int result_dpi =
                getSystemMetricsForWindow(handle, SM_CYSIZEFRAME) +
                getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER) +
                getSystemMetricsForWindow(handle, SM_CYCAPTION);
            return dpiAware ? result_dpi : result;
        }
    }
    }
    return -1;
}
