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
#include <QTimer>
#include <QWindow>
#include <QtMath>
#include <windowsx.h>

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

#ifndef DWM_BB_ENABLE
// Only available since Windows Vista
#define DWM_BB_ENABLE 0x00000001
#endif

#ifndef WTNCA_NODRAWCAPTION
// Only available since Windows Vista
#define WTNCA_NODRAWCAPTION 0x00000001
#endif

#ifndef WTNCA_NODRAWICON
// Only available since Windows Vista
#define WTNCA_NODRAWICON 0x00000002
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
// Only available since Windows 7
#define WM_DPICHANGED 0x02E0
#endif

#ifndef WNEF_RESOLVE_WINAPI
#define WNEF_RESOLVE_WINAPI(libName, funcName)                                 \
    if (!m_##funcName) {                                                       \
        QLibrary library(QString::fromUtf8(#libName));                         \
        m_##funcName =                                                         \
            reinterpret_cast<lp##funcName>(library.resolve(#funcName));        \
        if (!m_##funcName) {                                                   \
            qWarning().noquote()                                               \
                << "Failed to resolve" << #funcName << "from" << #libName      \
                << "-->" << library.errorString();                             \
        }                                                                      \
    }
#endif

namespace {

using WINDOWTHEMEATTRIBUTETYPE = enum _WINDOWTHEMEATTRIBUTETYPE {
    WTA_NONCLIENT = 1
};

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA {
    DWORD dwAttribute;
    PVOID pvAttribute;
    DWORD cbAttribute;
};

using ACCENT_STATE = enum _ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

using ACCENT_POLICY = struct _ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

using MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
};

using WTA_OPTIONS = struct _WTA_OPTIONS {
    DWORD dwFlags;
    DWORD dwMask;
};

using DWMNCRENDERINGPOLICY = enum _DWMNCRENDERINGPOLICY { DWMNCRP_ENABLED = 2 };

using DWMWINDOWATTRIBUTE = enum _DWMWINDOWATTRIBUTE {
    DWMWA_NCRENDERING_POLICY = 2
};

using DWM_BLURBEHIND = struct _DWM_BLURBEHIND {
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
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

using lpGetSystemDpiForProcess = UINT(WINAPI *)(HANDLE);
lpGetSystemDpiForProcess m_GetSystemDpiForProcess = nullptr;

using lpGetDpiForWindow = UINT(WINAPI *)(HWND);
lpGetDpiForWindow m_GetDpiForWindow = nullptr;

using lpGetDpiForSystem = UINT(WINAPI *)();
lpGetDpiForSystem m_GetDpiForSystem = nullptr;

using lpGetSystemMetricsForDpi = int(WINAPI *)(int, UINT);
lpGetSystemMetricsForDpi m_GetSystemMetricsForDpi = nullptr;

using lpGetDpiForMonitor = HRESULT(WINAPI *)(HMONITOR, MONITOR_DPI_TYPE, UINT *,
                                             UINT *);
lpGetDpiForMonitor m_GetDpiForMonitor = nullptr;

using lpSetWindowCompositionAttribute =
    BOOL(WINAPI *)(HWND, const WINDOWCOMPOSITIONATTRIBDATA *);
lpSetWindowCompositionAttribute m_SetWindowCompositionAttribute = nullptr;

using lpSetWindowThemeAttribute = HRESULT(WINAPI *)(HWND,
                                                    WINDOWTHEMEATTRIBUTETYPE,
                                                    PVOID, DWORD);
lpSetWindowThemeAttribute m_SetWindowThemeAttribute = nullptr;

using lpIsThemeActive = BOOL(WINAPI *)();
lpIsThemeActive m_IsThemeActive = nullptr;

using lpDwmEnableBlurBehindWindow = HRESULT(WINAPI *)(HWND,
                                                      const DWM_BLURBEHIND *);
lpDwmEnableBlurBehindWindow m_DwmEnableBlurBehindWindow = nullptr;

using lpDwmExtendFrameIntoClientArea = HRESULT(WINAPI *)(HWND, const MARGINS *);
lpDwmExtendFrameIntoClientArea m_DwmExtendFrameIntoClientArea = nullptr;

using lpDwmIsCompositionEnabled = HRESULT(WINAPI *)(BOOL *);
lpDwmIsCompositionEnabled m_DwmIsCompositionEnabled = nullptr;

using lpDwmSetWindowAttribute = HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD);
lpDwmSetWindowAttribute m_DwmSetWindowAttribute = nullptr;

using lpSHAppBarMessage = UINT_PTR(WINAPI *)(DWORD, APPBARDATA *);
lpSHAppBarMessage m_SHAppBarMessage = nullptr;

using lpGetDeviceCaps = int(WINAPI *)(HDC, int);
lpGetDeviceCaps m_GetDeviceCaps = nullptr;

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
        for (auto &&window : qAsConst(m_framelessWindows)) {
            refreshWindow(window);
        }
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
    if (window && !m_framelessWindows.contains(window)) {
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
        refreshWindow(window);
    }
}

void WinNativeEventFilter::clearFramelessWindows() {
    if (!m_framelessWindows.isEmpty()) {
        m_framelessWindows.clear();
    }
}

int WinNativeEventFilter::borderWidth(HWND handle) {
    if (handle) {
        createUserData(handle);
        const auto userData = reinterpret_cast<WINDOW *>(
            GetWindowLongPtrW(handle, GWLP_USERDATA));
        const int bw = userData->windowData.borderWidth;
        if (bw > 0) {
            return bw * getDevicePixelRatioForWindow(handle);
        }
    }
    if (m_borderWidth > 0) {
        return m_borderWidth;
    }
    return getSystemMetricsForWindow(handle, SM_CXFRAME) +
        getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
}

int WinNativeEventFilter::borderHeight(HWND handle) {
    if (handle) {
        createUserData(handle);
        const auto userData = reinterpret_cast<WINDOW *>(
            GetWindowLongPtrW(handle, GWLP_USERDATA));
        const int bh = userData->windowData.borderHeight;
        if (bh > 0) {
            return bh * getDevicePixelRatioForWindow(handle);
        }
    }
    if (m_borderHeight > 0) {
        return m_borderHeight;
    }
    return getSystemMetricsForWindow(handle, SM_CYFRAME) +
        getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
}

int WinNativeEventFilter::titlebarHeight(HWND handle) {
    if (handle) {
        createUserData(handle);
        const auto userData = reinterpret_cast<WINDOW *>(
            GetWindowLongPtrW(handle, GWLP_USERDATA));
        const int tbh = userData->windowData.titlebarHeight;
        if (tbh > 0) {
            return tbh * getDevicePixelRatioForWindow(handle);
        }
    }
    if (m_titlebarHeight > 0) {
        return m_titlebarHeight;
    }
    return borderHeight(handle) +
        getSystemMetricsForWindow(handle, SM_CYCAPTION);
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
    if (m_framelessWindows.isEmpty()) {
        bool isTopLevel = false;
        // QWidgets with Qt::WA_NativeWindow enabled will make them become top
        // level windows even if they are not. Try if
        // Qt::WA_DontCreateNativeAncestors helps.
        const auto topLevelWindows = QGuiApplication::topLevelWindows();
        for (auto &&window : qAsConst(topLevelWindows)) {
            if (window->handle() &&
                (msg->hwnd == reinterpret_cast<HWND>(window->winId()))) {
                isTopLevel = true;
                break;
            }
        }
        if (!isTopLevel) {
            return false;
        }
    } else if (!m_framelessWindows.contains(msg->hwnd)) {
        return false;
    }
    createUserData(msg->hwnd);
    const auto data =
        reinterpret_cast<WINDOW *>(GetWindowLongPtrW(msg->hwnd, GWLP_USERDATA));
    // Don't forget to init it if not inited, otherwise the window style will
    // not be updated, but don't init it twice as well.
    if (!data->inited) {
        init(data);
    }
    switch (msg->message) {
    case WM_NCCREATE: {
        // Work-around a long-existing Windows bug.
        const auto userData =
            reinterpret_cast<LPCREATESTRUCTW>(msg->lParam)->lpCreateParams;
        SetWindowLongPtrW(msg->hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(userData));
        refreshWindow(msg->hwnd);
        break;
    }
    case WM_NCCALCSIZE: {
        // If wParam is TRUE, it specifies that the application should indicate
        // which part of the client area contains valid information. The system
        // copies the valid information to the specified area within the new
        // client area. If wParam is FALSE, the application does not need to
        // indicate the valid part of the client area.
        if (static_cast<BOOL>(msg->wParam)) {
            if (IsMaximized(msg->hwnd)) {
                const HMONITOR monitor =
                    MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                if (monitor) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    GetMonitorInfoW(monitor, &monitorInfo);
                    auto &params =
                        *reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam);
                    params.rgrc[0] = monitorInfo.rcWork;
                    // If the client rectangle is the same as the monitor's
                    // rectangle, the shell assumes that the window has gone
                    // fullscreen, so it removes the topmost attribute from any
                    // auto-hide appbars, making them inaccessible. To avoid
                    // this, reduce the size of the client area by one pixel on
                    // a certain edge. The edge is chosen based on which side of
                    // the monitor is likely to contain an auto-hide appbar, so
                    // the missing client area is covered by it.
                    if (EqualRect(&params.rgrc[0], &monitorInfo.rcMonitor)) {
                        if (m_SHAppBarMessage) {
                            APPBARDATA abd;
                            SecureZeroMemory(&abd, sizeof(abd));
                            abd.cbSize = sizeof(abd);
                            const UINT taskbarState =
                                m_SHAppBarMessage(ABM_GETSTATE, &abd);
                            if (taskbarState & ABS_AUTOHIDE) {
                                int edge = -1;
                                abd.hWnd =
                                    FindWindowW(L"Shell_TrayWnd", nullptr);
                                if (abd.hWnd) {
                                    const HMONITOR taskbarMonitor =
                                        MonitorFromWindow(
                                            abd.hWnd, MONITOR_DEFAULTTONEAREST);
                                    if (taskbarMonitor &&
                                        (taskbarMonitor == monitor)) {
                                        m_SHAppBarMessage(ABM_GETTASKBARPOS,
                                                          &abd);
                                        edge = abd.uEdge;
                                    }
                                }
                                if (edge == ABE_BOTTOM) {
                                    params.rgrc[0].bottom--;
                                } else if (edge == ABE_LEFT) {
                                    params.rgrc[0].left++;
                                } else if (edge == ABE_TOP) {
                                    params.rgrc[0].top++;
                                } else if (edge == ABE_RIGHT) {
                                    params.rgrc[0].right--;
                                }
                            }
                        }
                    }
                }
            }
        }
        // "*result = 0" removes the window frame (including the titlebar).
        // But the frame shadow is lost at the same time. We'll bring it
        // back later.
        // Don't use "*result = WVR_REDRAW", although it can also remove
        // the window frame, it will cause child widgets have strange behaviors.
        // "*result = 0" means we have processed this message and let Windows
        // ignore it to avoid Windows process this message again.
        // "return true" means we have filtered this message and let Qt ignore
        // it, in other words, it'll block Qt's own handling of this message,
        // so if you don't know what Qt does internally, don't block it.
        *result = 0;
        return true;
    }
    case WM_DWMCOMPOSITIONCHANGED: {
        // Bring the frame shadow back through DWM.
        // Don't paint the shadow manually using QPainter or QGraphicsEffect.
        handleDwmCompositionChanged(data);
        *result = 0;
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
        if (data->dwmCompositionEnabled) {
            break;
        } else {
            // Only block WM_NCPAINT when composition is disabled. If it's
            // blocked when composition is enabled, the window shadow won't
            // be drawn.
            *result = 0;
            return true;
        }
    }
    case WM_NCACTIVATE: {
        // DefWindowProc won't repaint the window border if lParam (normally
        // a HRGN) is -1.
        *result = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, -1);
        return true;
    }
    case WM_NCHITTEST: {
        const auto getHTResult = [](HWND _hWnd, LPARAM _lParam,
                                    const WINDOW *_data) -> LRESULT {
            const auto isInSpecificAreas = [](int x, int y,
                                              const QVector<QRect> &areas,
                                              qreal dpr) -> bool {
                for (auto &&area : qAsConst(areas)) {
                    if (!area.isValid()) {
                        continue;
                    }
                    if (QRect(area.x() * dpr, area.y() * dpr,
                              area.width() * dpr, area.height() * dpr)
                            .contains(x, y, true)) {
                        return true;
                    }
                }
                return false;
            };
            RECT clientRect = {0, 0, 0, 0};
            GetClientRect(_hWnd, &clientRect);
            const LONG ww = clientRect.right;
            const LONG wh = clientRect.bottom;
            POINT mouse;
            mouse.x = GET_X_LPARAM(_lParam);
            mouse.y = GET_Y_LPARAM(_lParam);
            ScreenToClient(_hWnd, &mouse);
            // These values are DPI-aware.
            const LONG bw = borderWidth(_hWnd);
            const LONG bh = borderHeight(_hWnd);
            const LONG tbh = titlebarHeight(_hWnd);
            const bool isInsideWindow = (mouse.x > 0) && (mouse.x < ww) &&
                (mouse.y > 0) && (mouse.y < wh);
            const qreal dpr = getDevicePixelRatioForWindow(_hWnd);
            const bool isTitlebar = isInsideWindow && (mouse.y < tbh) &&
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
            const bool isTop = isInsideWindow && (mouse.y < bh);
            const bool isBottom = isInsideWindow && (mouse.y > (wh - bh));
            const bool isLeft = isInsideWindow && (mouse.x < bw);
            const bool isRight = isInsideWindow && (mouse.x > (ww - bw));
            if (isTop) {
                if (isLeft) {
                    return HTTOPLEFT;
                }
                if (isRight) {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (isBottom) {
                if (isLeft) {
                    return HTBOTTOMLEFT;
                }
                if (isRight) {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
            if (isLeft) {
                return HTLEFT;
            }
            if (isRight) {
                return HTRIGHT;
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
            MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
        if (monitor) {
            MONITORINFO monitorInfo;
            SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
            monitorInfo.cbSize = sizeof(monitorInfo);
            GetMonitorInfoW(monitor, &monitorInfo);
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
                    qAbs(rcWorkArea.left - rcMonitorArea.left);
                mmi.ptMaxPosition.y = qAbs(rcWorkArea.top - rcMonitorArea.top);
            }
            if (data->windowData.maximumSize.isEmpty()) {
                mmi.ptMaxSize.x = qAbs(rcWorkArea.right - rcWorkArea.left);
                mmi.ptMaxSize.y = qAbs(rcWorkArea.bottom - rcWorkArea.top);
            } else {
                mmi.ptMaxSize.x = getDevicePixelRatioForWindow(msg->hwnd) *
                    data->windowData.maximumSize.width();
                mmi.ptMaxSize.y = getDevicePixelRatioForWindow(msg->hwnd) *
                    data->windowData.maximumSize.height();
            }
            mmi.ptMaxTrackSize.x = mmi.ptMaxSize.x;
            mmi.ptMaxTrackSize.y = mmi.ptMaxSize.y;
            if (!data->windowData.minimumSize.isEmpty()) {
                mmi.ptMinTrackSize.x = getDevicePixelRatioForWindow(msg->hwnd) *
                    data->windowData.minimumSize.width();
                mmi.ptMinTrackSize.y = getDevicePixelRatioForWindow(msg->hwnd) *
                    data->windowData.minimumSize.height();
            }
            *result = 0;
            return true;
        }
        break;
    }
    case WM_SETICON:
    case WM_SETTEXT: {
        // Disable painting while these messages are handled to prevent them
        // from drawing a window caption over the client area, but only when
        // composition and theming are disabled. These messages don't paint
        // when composition is enabled and blocking WM_NCUAHDRAWCAPTION should
        // be enough to prevent painting when theming is enabled.
        if (!data->dwmCompositionEnabled && !data->themeEnabled) {
            const LONG_PTR oldStyle = GetWindowLongPtrW(msg->hwnd, GWL_STYLE);
            // Prevent Windows from drawing the default title bar by temporarily
            // toggling the WS_VISIBLE style.
            SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle & ~WS_VISIBLE);
            refreshWindow(msg->hwnd);
            const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message,
                                               msg->wParam, msg->lParam);
            SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
            refreshWindow(msg->hwnd);
            *result = ret;
            return true;
        }
        break;
    }
    case WM_THEMECHANGED: {
        handleThemeChanged(data);
        break;
    }
    case WM_WINDOWPOSCHANGED: {
        // The client area of our window equals to it's non-client area now (we
        // achieve this in WM_NCCALCSIZE), so the following line will repaint
        // the whole window.
        InvalidateRect(msg->hwnd, nullptr, TRUE);
        // Don't return true here because it will block Qt's paint events
        // and it'll result in a never updated window.
        break;
    }
    case WM_DPICHANGED: {
        // Qt will do the scaling internally and automatically.
        // See: qt/qtbase/src/plugins/platforms/windows/qwindowscontext.cpp
        const auto dpiX = LOWORD(msg->wParam);
        const auto dpiY = HIWORD(msg->wParam);
        // dpiX and dpiY are identical. Just to silence a compiler warning.
        const auto dpi = dpiX == dpiY ? dpiY : dpiX;
        qDebug().noquote() << "Window DPI changed: new DPI -->" << dpi
                           << ", new DPR -->"
                           << getPreferedNumber(qreal(dpi) /
                                                qreal(m_defaultDotsPerInch));
        // Record the window handle now, don't use msg->hwnd directly because
        // when we finally execute the function, it has changed.
        const HWND _hWnd = msg->hwnd;
        // Wait some time for Qt to adjust the window size, but don't wait too
        // long, we want to refresh the window as soon as possible.
        // We can intercept Qt's handling of this message and resize the window
        // ourself, but after reading Qt's source code, I found that Qt does
        // more than resizing, so it's safer to let Qt do the scaling.
        QTimer::singleShot(50, [_hWnd]() {
            RECT rect = {0, 0, 0, 0};
            GetWindowRect(_hWnd, &rect);
            const int x = rect.left;
            const int y = rect.top;
            const int width = qAbs(rect.right - rect.left);
            const int height = qAbs(rect.bottom - rect.top);
            // Don't increase the window size too much, otherwise it would be
            // too obvious for the user and the experience is not good.
            MoveWindow(_hWnd, x, y, width + 1, height + 1, TRUE);
            // Re-paint the window after resizing.
            refreshWindow(_hWnd);
            // Restore and repaint.
            MoveWindow(_hWnd, x, y, width, height, TRUE);
            refreshWindow(_hWnd);
        });
        break;
    }
    default: {
        break;
    }
    }
    return false;
}

void WinNativeEventFilter::init(WINDOW *data) {
    // Make sure we don't init the same window twice.
    data->inited = TRUE;
    // Make sure our window is a normal application window, we'll remove the
    // window frame later in Win32 events, don't use WS_POPUP to do this.
    SetWindowLongPtrW(data->hWnd, GWL_STYLE,
                      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    // Make our window a layered window to get better performance. It's also
    // needed to remove the three system buttons (minimize, maximize and close)
    // with the help of the next line.
    SetWindowLongPtrW(data->hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_LAYERED);
    SetLayeredWindowAttributes(data->hWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    // Make sure our window has the frame shadow.
    // According to MSDN, SetWindowLong won't take effect unless we trigger a
    // frame change event manually, we will do it inside the
    // handleDwmCompositionChanged function, so it's not necessary to do it
    // here.
    handleDwmCompositionChanged(data);
    handleThemeChanged(data);
    // For debug purposes.
    qDebug().noquote() << "Window handle:" << data->hWnd;
    qDebug().noquote() << "Window DPI:" << getDotsPerInchForWindow(data->hWnd)
                       << "Window DPR:"
                       << getDevicePixelRatioForWindow(data->hWnd);
    qDebug().noquote() << "Window border width:" << borderWidth(data->hWnd)
                       << "Window border height:" << borderHeight(data->hWnd)
                       << "Window titlebar height:"
                       << titlebarHeight(data->hWnd);
}

void WinNativeEventFilter::handleDwmCompositionChanged(WINDOW *data) {
    BOOL enabled = FALSE;
    if (m_DwmIsCompositionEnabled) {
        m_DwmIsCompositionEnabled(&enabled);
    }
    data->dwmCompositionEnabled = enabled;
    // We should not draw the frame shadow if DWM composition is disabled, in
    // other words, a window should not have frame shadow when Windows Aero is
    // not enabled.
    // Note that, start from Win8, the DWM composition is always enabled and
    // can't be disabled.
    if (enabled) {
        if (m_DwmSetWindowAttribute) {
            // The frame shadow is drawn on the non-client area and thus we have
            // to make sure the non-client area rendering is enabled first.
            const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
            m_DwmSetWindowAttribute(data->hWnd, DWMWA_NCRENDERING_POLICY, &ncrp,
                                    sizeof(ncrp));
        }
        if (m_DwmExtendFrameIntoClientArea) {
            // Negative margins have special meaning to
            // DwmExtendFrameIntoClientArea. Negative margins create the "sheet
            // of glass" effect, where the client area is rendered as a solid
            // surface with no window border.
            const MARGINS margins = {-1, -1, -1, -1};
            m_DwmExtendFrameIntoClientArea(data->hWnd, &margins);
        }
    }
    if (m_SetWindowThemeAttribute) {
        WTA_OPTIONS options;
        options.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
        options.dwMask = options.dwFlags;
        // This is the official way to hide the window caption text and window
        // icon.
        m_SetWindowThemeAttribute(data->hWnd, WTA_NONCLIENT, &options,
                                  sizeof(options));
    }
    handleBlurForWindow(data);
    refreshWindow(data->hWnd);
}

void WinNativeEventFilter::handleThemeChanged(WINDOW *data) {
    data->themeEnabled = m_IsThemeActive ? m_IsThemeActive() : FALSE;
}

void WinNativeEventFilter::handleBlurForWindow(const WINDOW *data) {
    if ((QOperatingSystemVersion::current() <
         QOperatingSystemVersion::Windows7) ||
        !(data->dwmCompositionEnabled && data->windowData.blurEnabled)) {
        return;
    }
    // We prefer using DWM blur on Windows 7 because it has better appearance.
    // It's supported on Windows Vista as well actually, but Qt has drop it, so
    // we won't do it for Vista.
    if (QOperatingSystemVersion::current() <
        QOperatingSystemVersion::Windows8) {
        if (m_DwmEnableBlurBehindWindow) {
            // Windows Aero
            DWM_BLURBEHIND dwmbb;
            dwmbb.dwFlags = DWM_BB_ENABLE;
            dwmbb.fEnable = TRUE;
            dwmbb.hRgnBlur = nullptr;
            dwmbb.fTransitionOnMaximized = FALSE;
            m_DwmEnableBlurBehindWindow(data->hWnd, &dwmbb);
        }
    } else if (m_SetWindowCompositionAttribute) {
        ACCENT_POLICY accentPolicy;
        accentPolicy.AccentFlags = 0;
        // GradientColor only has effect when using with acrylic, so we can set
        // it to zero in most cases. It's an AGBR unsigned int, for example, use
        // 0xCC000000 for dark blur behind background.
        accentPolicy.GradientColor = 0;
        accentPolicy.AnimationId = 0;
        WINDOWCOMPOSITIONATTRIBDATA attribData;
        attribData.dwAttribute = WCA_ACCENT_POLICY;
        attribData.pvAttribute = &accentPolicy;
        attribData.cbAttribute = sizeof(accentPolicy);
        // Windows 10, version 1709 (10.0.16299)
        if (QOperatingSystemVersion::current() >=
            QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                    16299)) {
            // Acrylic (Will also blur but is completely different with
            // Windows Aero)
            accentPolicy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
        } else if (QOperatingSystemVersion::current() >=
                   QOperatingSystemVersion::Windows10) {
            // Blur (Something like Windows Aero in Windows 7)
            accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
        } else if (QOperatingSystemVersion::current() >=
                   QOperatingSystemVersion::Windows8) {
            // Transparent gradient color
            accentPolicy.AccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
        }
        m_SetWindowCompositionAttribute(data->hWnd, &attribData);
    }
}

UINT WinNativeEventFilter::getDotsPerInchForWindow(HWND handle) {
    const auto getScreenDpi = [](UINT defaultValue) -> UINT {
#if 0
        // Using Direct2D to get screen DPI. Available since Windows 7.
        ID2D1Factory *m_pDirect2dFactory = nullptr;
        if (SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                        &m_pDirect2dFactory)) &&
            m_pDirect2dFactory) {
            m_pDirect2dFactory->ReloadSystemMetrics();
            FLOAT dpiX = defaultValue, dpiY = defaultValue;
            m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
            // The values of *dpiX and *dpiY are identical.
            return dpiX;
        }
#endif
        // Available since Windows 2000.
        const HDC hdc = GetDC(nullptr);
        if (hdc && m_GetDeviceCaps) {
            const int dpiX = m_GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = m_GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
            // The values of dpiX and dpiY are identical actually, just to
            // silence a compiler warning.
            return dpiX == dpiY ? dpiY : dpiX;
        }
        return defaultValue;
    };
    if (!handle) {
        if (m_GetSystemDpiForProcess) {
            return m_GetSystemDpiForProcess(GetCurrentProcess());
        } else if (m_GetDpiForSystem) {
            return m_GetDpiForSystem();
        } else {
            return getScreenDpi(m_defaultDotsPerInch);
        }
    }
    if (m_GetDpiForWindow) {
        return m_GetDpiForWindow(handle);
    }
    if (m_GetDpiForMonitor) {
        UINT dpiX = m_defaultDotsPerInch, dpiY = m_defaultDotsPerInch;
        m_GetDpiForMonitor(MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST),
                           MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        // The values of *dpiX and *dpiY are identical.
        return dpiX;
    }
    return getScreenDpi(m_defaultDotsPerInch);
}

qreal WinNativeEventFilter::getDevicePixelRatioForWindow(HWND handle) {
    const qreal dpr = handle
        ? (qreal(getDotsPerInchForWindow(handle)) / qreal(m_defaultDotsPerInch))
        : m_defaultDevicePixelRatio;
    return getPreferedNumber(dpr);
}

int WinNativeEventFilter::getSystemMetricsForWindow(HWND handle, int index) {
    if (m_GetSystemMetricsForDpi) {
        return m_GetSystemMetricsForDpi(index,
                                        static_cast<UINT>(getPreferedNumber(
                                            getDotsPerInchForWindow(handle))));
    } else {
        return GetSystemMetrics(index) * getDevicePixelRatioForWindow(handle);
    }
}

void WinNativeEventFilter::setWindowData(HWND window, const WINDOWDATA *data) {
    if (window && data) {
        createUserData(window, data);
        refreshWindow(window);
    }
}

WinNativeEventFilter::WINDOWDATA *
WinNativeEventFilter::windowData(HWND window) {
    if (window) {
        createUserData(window);
        return &reinterpret_cast<WINDOW *>(
                    GetWindowLongPtrW(window, GWLP_USERDATA))
                    ->windowData;
    }
    return nullptr;
}

void WinNativeEventFilter::createUserData(HWND handle, const WINDOWDATA *data) {
    if (handle) {
        const auto userData = reinterpret_cast<WINDOW *>(
            GetWindowLongPtrW(handle, GWLP_USERDATA));
        if (userData) {
            if (data) {
                if (userData->windowData.blurEnabled != data->blurEnabled) {
                    qDebug().noquote()
                        << "Due to technical issue, you can only enable or "
                           "disable blur before the window is shown.";
                }
                userData->windowData = *data;
            }
        } else {
            WINDOW *_data = new WINDOW;
            _data->hWnd = handle;
            if (data) {
                _data->windowData = *data;
            }
            SetWindowLongPtrW(handle, GWLP_USERDATA,
                              reinterpret_cast<LONG_PTR>(_data));
            refreshWindow(handle);
        }
    }
}

void WinNativeEventFilter::refreshWindow(HWND handle) {
    if (handle) {
        // SWP_FRAMECHANGED: Applies new frame styles set using the
        // SetWindowLong function. Sends a WM_NCCALCSIZE message to the window,
        // even if the window's size is not being changed.
        // SWP_NOACTIVATE: Does not activate the window. If this flag is not
        // set, the window is activated and moved to the top of either the
        // topmost or non-topmost group (depending on the setting of the
        // hWndInsertAfter parameter).
        // SWP_NOSIZE: Retains the current size (ignores the cx and cy
        // parameters).
        // SWP_NOMOVE: Retains the current position (ignores X and Y
        // parameters).
        // SWP_NOZORDER: Retains the current Z order (ignores the
        // hWndInsertAfter parameter).
        // SWP_NOOWNERZORDER: Does not change the owner window's position
        // in the Z order.
        // We will remove the window frame (including the three system buttons)
        // when we are processing the WM_NCCALCSIZE message, so we want to
        // trigger this message as early as possible to let it happen.
        SetWindowPos(handle, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE |
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        // Inform the window to adjust it's size to let it's contents fit the
        // adjusted window.
        RECT rect = {0, 0, 0, 0};
        GetWindowRect(handle, &rect);
        const int width = qAbs(rect.right - rect.left);
        const int height = qAbs(rect.bottom - rect.top);
        SendMessageW(handle, WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, height));
        // The InvalidateRect function adds a rectangle to the specified
        // window's update region. The update region represents the portion of
        // the window's client area that must be redrawn. If lpRect is NULL, the
        // entire client area is added to the update region.
        InvalidateRect(handle, nullptr, TRUE);
        // The UpdateWindow function updates the client area of the specified
        // window by sending a WM_PAINT message to the window if the window's
        // update region is not empty. The function sends a WM_PAINT message
        // directly to the window procedure of the specified window, bypassing
        // the application queue. If the update region is empty, no message is
        // sent.
        UpdateWindow(handle);
    }
}

void WinNativeEventFilter::initWin32Api() {
    // Available since Windows 2000.
    WNEF_RESOLVE_WINAPI(Gdi32, GetDeviceCaps)
    // Available since Windows XP.
    WNEF_RESOLVE_WINAPI(Shell32, SHAppBarMessage)
    // Available since Windows Vista.
    WNEF_RESOLVE_WINAPI(UxTheme, SetWindowThemeAttribute)
    WNEF_RESOLVE_WINAPI(UxTheme, IsThemeActive)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmIsCompositionEnabled)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmExtendFrameIntoClientArea)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmSetWindowAttribute)
    WNEF_RESOLVE_WINAPI(Dwmapi, DwmEnableBlurBehindWindow)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows7) {
        WNEF_RESOLVE_WINAPI(User32, SetWindowCompositionAttribute)
    }
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows8_1) {
        WNEF_RESOLVE_WINAPI(SHCore, GetDpiForMonitor)
    }
    // Windows 10, version 1607 (10.0.14393)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                14393)) {
        WNEF_RESOLVE_WINAPI(User32, GetDpiForWindow)
        WNEF_RESOLVE_WINAPI(User32, GetDpiForSystem)
        WNEF_RESOLVE_WINAPI(User32, GetSystemMetricsForDpi)
    }
    // Windows 10, version 1803 (10.0.17134)
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
            return qRound(in);
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
        result = qFloor(num);
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Ceil:
        result = qCeil(num);
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
