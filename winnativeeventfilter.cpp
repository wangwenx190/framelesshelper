#include "winnativeeventfilter.h"

#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QOperatingSystemVersion>
#include <QWindow>
#include <d2d1.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <windowsx.h>

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
// Only available since Windows 7
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif

namespace {

QScopedPointer<WinNativeEventFilter> instance;

bool isWindowTopLevel(HWND handle) {
    if (!handle) {
        return false;
    }
    const auto wid = reinterpret_cast<WId>(handle);
    const auto topLevelWindows = QGuiApplication::topLevelWindows();
    for (auto &&window : qAsConst(topLevelWindows)) {
        if (window->handle() && (window->winId() == wid)) {
            return true;
        }
    }
    return false;
}

} // namespace

WinNativeEventFilter::WinNativeEventFilter() {
    QLibrary shcoreLib(QString::fromUtf8("SHCore"));
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows8_1) {
        m_GetDpiForMonitor = reinterpret_cast<lpGetDpiForMonitor>(
            shcoreLib.resolve("GetDpiForMonitor"));
    }
    QLibrary user32Lib(QString::fromUtf8("User32"));
    // Windows 10, version 1607 (10.0.14393)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                14393)) {
        m_GetDpiForWindow = reinterpret_cast<lpGetDpiForWindow>(
            user32Lib.resolve("GetDpiForWindow"));
        m_GetDpiForSystem = reinterpret_cast<lpGetDpiForSystem>(
            user32Lib.resolve("GetDpiForSystem"));
        m_GetSystemMetricsForDpi = reinterpret_cast<lpGetSystemMetricsForDpi>(
            user32Lib.resolve("GetSystemMetricsForDpi"));
    }
    // Windows 10, version 1803 (10.0.17134)
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0,
                                17134)) {
        m_GetSystemDpiForProcess = reinterpret_cast<lpGetSystemDpiForProcess>(
            user32Lib.resolve("GetSystemDpiForProcess"));
    }
}

WinNativeEventFilter::~WinNativeEventFilter() = default;

void WinNativeEventFilter::setup() {
    if (instance.isNull()) {
        instance.reset(new WinNativeEventFilter);
        qApp->installNativeEventFilter(instance.data());
    }
}

UINT WinNativeEventFilter::windowDpi(HWND handle) const {
    return getDpiForWindow(handle);
}

qreal WinNativeEventFilter::windowDpr(HWND handle) const {
    return getDprForWindow(handle);
}

int WinNativeEventFilter::borderWidth(HWND handle) const {
    return getSystemMetricsForWindow(handle, SM_CXFRAME) +
        getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
}

int WinNativeEventFilter::borderHeight(HWND handle) const {
    return getSystemMetricsForWindow(handle, SM_CYFRAME) +
        getSystemMetricsForWindow(handle, SM_CXPADDEDBORDER);
}

int WinNativeEventFilter::titlebarHeight(HWND handle) const {
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
    if (!isWindowTopLevel(msg->hwnd)) {
        // Only top level windows can be frameless.
        return false;
    }
    if (!m_windowData.contains(msg->hwnd)) {
        m_windowData.insert(msg->hwnd, qMakePair(FALSE, FALSE));
        init(msg->hwnd);
    }
    switch (msg->message) {
    case WM_NCCREATE: {
        // Work-around a long-existing Windows bug.
        const auto userData =
            reinterpret_cast<LPCREATESTRUCTW>(msg->lParam)->lpCreateParams;
        SetWindowLongPtrW(msg->hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(userData));
        break;
    }
    case WM_NCCALCSIZE: {
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
                }
            }
            // This line removes the window frame (including the titlebar).
            // But the frame shadow is lost at the same time. We'll bring it
            // back later.
            *result = WVR_REDRAW;
        } else {
            *result = 0;
        }
        return true;
    }
    case WM_DWMCOMPOSITIONCHANGED: {
        // Bring the frame shadow back through DWM.
        // Don't paint the shadow manually using QPainter or QGraphicsEffect.
        handleDwmCompositionChanged(msg->hwnd);
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
        if (m_windowData.value(msg->hwnd).first) {
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
        const auto getHTResult = [this, msg]() -> LRESULT {
            RECT winRect;
            GetWindowRect(msg->hwnd, &winRect);
            const LONG ww = qAbs(winRect.right - winRect.left);
            const LONG wh = qAbs(winRect.bottom - winRect.top);
            POINT mouse = {LONG(GET_X_LPARAM(msg->lParam)),
                           LONG(GET_Y_LPARAM(msg->lParam))};
            ScreenToClient(msg->hwnd, &mouse);
            // These values are DPI-aware.
            const LONG bw = borderWidth(msg->hwnd);
            const LONG bh = borderHeight(msg->hwnd);
            const LONG tbh = titlebarHeight(msg->hwnd);
            if (IsMaximized(msg->hwnd)) {
                if (mouse.y < tbh) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            }
            if (mouse.y < bh) {
                if (mouse.x < bw) {
                    return HTTOPLEFT;
                }
                if (mouse.x > (ww - bw)) {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (mouse.y > (wh - bh)) {
                if (mouse.x < bw) {
                    return HTBOTTOMLEFT;
                }
                if (mouse.x > (ww - bw)) {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
            if (mouse.x < bw) {
                return HTLEFT;
            }
            if (mouse.x > (ww - bw)) {
                return HTRIGHT;
            }
            if (mouse.y < tbh) {
                return HTCAPTION;
            }
            return HTCLIENT;
        };
        *result = getHTResult();
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
            mmi.ptMaxPosition.x = qAbs(rcWorkArea.left - rcMonitorArea.left);
            mmi.ptMaxPosition.y = qAbs(rcWorkArea.top - rcMonitorArea.top);
            mmi.ptMaxSize.x = qAbs(rcWorkArea.right - rcWorkArea.left);
            mmi.ptMaxSize.y = qAbs(rcWorkArea.bottom - rcWorkArea.top);
            mmi.ptMaxTrackSize.x = mmi.ptMaxSize.x;
            mmi.ptMaxTrackSize.y = mmi.ptMaxSize.y;
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
        if (!m_windowData.value(msg->hwnd).first &&
            !m_windowData.value(msg->hwnd).second) {
            const LONG_PTR oldStyle = GetWindowLongPtrW(msg->hwnd, GWL_STYLE);
            // Prevent Windows from drawing the default title bar by temporarily
            // toggling the WS_VISIBLE style.
            SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle & ~WS_VISIBLE);
            const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message,
                                               msg->wParam, msg->lParam);
            SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
            *result = ret;
            return true;
        }
        break;
    }
    case WM_THEMECHANGED: {
        const BOOL dwm = m_windowData.value(msg->hwnd).first;
        m_windowData[msg->hwnd] = qMakePair(dwm, IsThemeActive());
        break;
    }
    default:
        break;
    }
    return false;
}

void WinNativeEventFilter::init(HWND handle) {
    // Make sure our window is a normal application window, we'll remove the
    // window frame later in Win32 events, don't use WS_POPUP to do this.
    SetWindowLongPtrW(handle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowLongPtrW(handle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_LAYERED);
    // Make the window a layered window so the legacy GDI API can be used to
    // draw to it without messing up the area on top of the DWM frame. Note:
    // This is not necessary if other drawing APIs are used, eg. GDI+, OpenGL,
    // Direct2D, Direct3D, DirectComposition, etc.
    SetLayeredWindowAttributes(handle, RGB(255, 0, 255), 0, LWA_COLORKEY);
    // Make sure our window has the frame shadow.
    handleDwmCompositionChanged(handle);
    // Tell the window to redraw itself.
    SetWindowPos(handle, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE |
                     SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    // For debug purposes.
    qDebug().noquote() << "Window handle:" << handle;
    qDebug().noquote() << "Window DPI:" << windowDpi(handle)
                       << "Window DPR:" << windowDpr(handle);
    qDebug().noquote() << "Window border width:" << borderWidth(handle)
                       << "Window border height:" << borderHeight(handle)
                       << "Window titlebar height:" << titlebarHeight(handle);
}

void WinNativeEventFilter::handleDwmCompositionChanged(HWND handle) {
    BOOL enabled = FALSE;
    DwmIsCompositionEnabled(&enabled);
    const BOOL theme = m_windowData.value(handle).second;
    m_windowData[handle] = qMakePair(enabled, theme);
    WTA_OPTIONS options;
    options.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
    // We should not draw the frame shadow if DWM composition is disabled, in
    // other words, a window should not have frame shadow when Windows Aero is
    // not enabled.
    // Note that, start from Win8, the DWM composition is always enabled and
    // can't be disabled.
    if (enabled) {
        options.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
        // The frame shadow is drawn on the non-client area and thus we have to
        // make sure the non-client area rendering is enabled first.
        const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        DwmSetWindowAttribute(handle, DWMWA_NCRENDERING_POLICY, &ncrp,
                              sizeof(ncrp));
        // Negative margins have special meaning to
        // DwmExtendFrameIntoClientArea. Negative margins create the "sheet of
        // glass" effect, where the client area is rendered as a solid surface
        // with no window border.
        const MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(handle, &margins);
    }
    SetWindowThemeAttribute(handle, WTA_NONCLIENT, &options, sizeof(options));
}

UINT WinNativeEventFilter::getDpiForWindow(HWND handle) const {
    const auto getScreenDpi = [this]() -> UINT {
        // Available since Windows 7.
        ID2D1Factory *m_pDirect2dFactory = nullptr;
        if (SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                        &m_pDirect2dFactory)) &&
            m_pDirect2dFactory) {
            m_pDirect2dFactory->ReloadSystemMetrics();
            FLOAT dpiX = m_defaultDPI, dpiY = m_defaultDPI;
            m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
            // The values of *dpiX and *dpiY are identical.
            return dpiX;
        }
        // Available since Windows 2000.
        const HDC hdc = GetDC(nullptr);
        if (hdc) {
            const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
            // The values of dpiX and dpiY are identical actually, just to
            // silence a compiler warning.
            return dpiX == dpiY ? dpiX : dpiY;
        }
        return m_defaultDPI;
    };
    if (!handle) {
        if (m_GetSystemDpiForProcess) {
            return m_GetSystemDpiForProcess(GetCurrentProcess());
        } else if (m_GetDpiForSystem) {
            return m_GetDpiForSystem();
        } else {
            return getScreenDpi();
        }
    }
    if (m_GetDpiForWindow) {
        return m_GetDpiForWindow(handle);
    }
    if (m_GetDpiForMonitor) {
        UINT dpiX = m_defaultDPI, dpiY = m_defaultDPI;
        m_GetDpiForMonitor(MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST),
                           MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        // The values of *dpiX and *dpiY are identical.
        return dpiX;
    }
    return getScreenDpi();
}

qreal WinNativeEventFilter::getDprForWindow(HWND handle) const {
    return handle ? (qreal(getDpiForWindow(handle)) / qreal(m_defaultDPI))
                  : m_defaultDPR;
}

int WinNativeEventFilter::getSystemMetricsForWindow(HWND handle,
                                                    int index) const {
    if (m_GetSystemMetricsForDpi) {
        return m_GetSystemMetricsForDpi(index, getDpiForWindow(handle));
    } else {
        return GetSystemMetrics(index) * getDprForWindow(handle);
    }
}
