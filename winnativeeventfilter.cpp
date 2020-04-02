#include "winnativeeventfilter.h"

#include <QtMath>
#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QOperatingSystemVersion>
#include <QWindow>
#include <d2d1.h>
#include <dwmapi.h>
#include <shellapi.h>
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
// Only available since Windows Vista
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif

#ifndef WM_DPICHANGED
// Only available since Windows 8.1
#define WM_DPICHANGED 0x02E0
#endif

namespace {

QScopedPointer<WinNativeEventFilter> instance;
QVector<HWND> m_framelessWindows;

} // namespace

WinNativeEventFilter::WinNativeEventFilter() {
    QLibrary user32Lib(QString::fromUtf8("User32")),
        shcoreLib(QString::fromUtf8("SHCore"));
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows7) {
        m_SetWindowCompositionAttribute =
            reinterpret_cast<lpSetWindowCompositionAttribute>(
                user32Lib.resolve("SetWindowCompositionAttribute"));
    }
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion::Windows8_1) {
        m_GetDpiForMonitor = reinterpret_cast<lpGetDpiForMonitor>(
            shcoreLib.resolve("GetDpiForMonitor"));
    }
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

void WinNativeEventFilter::install() {
    if (instance.isNull()) {
        instance.reset(new WinNativeEventFilter);
        qApp->installNativeEventFilter(instance.data());
    }
}

void WinNativeEventFilter::uninstall() {
    if (!instance.isNull()) {
        qApp->removeNativeEventFilter(instance.data());
        instance.reset();
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

void WinNativeEventFilter::addFramelessWindow(HWND window, WINDOWDATA *data) {
    if (window && !m_framelessWindows.contains(window)) {
        m_framelessWindows.append(window);
        if (data) {
            createUserData(window, data);
            refreshWindow(window);
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

void WinNativeEventFilter::refreshWindow(HWND handle) {
    if (handle) {
        SetWindowPos(handle, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE |
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        SendMessageW(handle, WM_SIZE, 0, 0);
        UpdateWindow(handle);
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
        reinterpret_cast<LPWINDOW>(GetWindowLongPtrW(msg->hwnd, GWLP_USERDATA));
    // Don't forget to init it if not inited, otherwise the window style will not be updated, but don't init it twice as well.
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
        break;
    }
    case WM_NCCALCSIZE: {
        // MSDN: No special handling is needed when wParam is FALSE.
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
                        APPBARDATA abd;
                        SecureZeroMemory(&abd, sizeof(abd));
                        abd.cbSize = sizeof(abd);
                        const UINT taskbarState =
                            SHAppBarMessage(ABM_GETSTATE, &abd);
                        if (taskbarState & ABS_AUTOHIDE) {
                            int edge = -1;
                            abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                            if (abd.hWnd) {
                                const HMONITOR taskbarMonitor =
                                    MonitorFromWindow(abd.hWnd,
                                                      MONITOR_DEFAULTTONEAREST);
                                if (taskbarMonitor &&
                                    (taskbarMonitor == monitor)) {
                                    SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
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
        // This line removes the window frame (including the titlebar).
        // But the frame shadow is lost at the same time. We'll bring it
        // back later.
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
        const auto getHTResult = [this](HWND _hWnd, LPARAM _lParam,
                                        LPWINDOW _data) -> LRESULT {
            const auto isInSpecificAreas = [](int x, int y,
                                           const QVector<QRect> &areas, qreal dpr) -> bool {
                for (auto &&area : qAsConst(areas)) {
                    if (!area.isValid()) {
                        continue;
                    }
                    if (QRect(area.x() * dpr, area.y() * dpr, area.width() * dpr,
                              area.height() * dpr).contains(x, y, true)) {
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
            const int borderWidth_userDefined = _data->windowData.borderWidth;
            const int borderHeight_userDefined = _data->windowData.borderHeight;
            const int titlebarHeight_userDefined =
                _data->windowData.titlebarHeight;
            // These values should be DPI-aware.
            const qreal dpr = windowDpr(_hWnd);
            const LONG bw = borderWidth_userDefined > 0 ? dpr * borderWidth_userDefined
                                                       : borderWidth(_hWnd);
            const LONG bh = borderHeight_userDefined > 0
                ? dpr * borderHeight_userDefined
                : borderHeight(_hWnd);
            const LONG tbh = titlebarHeight_userDefined > 0
                ? dpr * titlebarHeight_userDefined
                : titlebarHeight(_hWnd);
            const bool isInsideWindow = (mouse.x > 0) && (mouse.x < ww) && (mouse.y > 0) && (mouse.y < wh);
            const bool isTitlebar = isInsideWindow && (mouse.y < tbh) &&
                !isInSpecificAreas(mouse.x, mouse.y, _data->windowData.ignoreAreas, dpr)
                 && (_data->windowData.draggableAreas.isEmpty() ? true : isInSpecificAreas(mouse.x, mouse.y, _data->windowData.draggableAreas, dpr));
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
            mmi.ptMaxPosition.x = qAbs(rcWorkArea.left - rcMonitorArea.left);
            mmi.ptMaxPosition.y = qAbs(rcWorkArea.top - rcMonitorArea.top);
            mmi.ptMaxSize.x = qAbs(rcWorkArea.right - rcWorkArea.left);
            mmi.ptMaxSize.y = qAbs(rcWorkArea.bottom - rcWorkArea.top);
            mmi.ptMaxTrackSize.x = mmi.ptMaxSize.x;
            mmi.ptMaxTrackSize.y = mmi.ptMaxSize.y;
            if (!data->windowData.minimumSize.isEmpty()) {
                mmi.ptMinTrackSize.x = windowDpr(msg->hwnd) * data->windowData.minimumSize.width();
                mmi.ptMinTrackSize.y = windowDpr(msg->hwnd) * data->windowData.minimumSize.height();
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
            const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message,
                                               msg->wParam, msg->lParam);
            SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
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
        // Repaint the non-client area immediately.
        InvalidateRect(msg->hwnd, nullptr, TRUE);
        break;
    }
    case WM_DPICHANGED: {
        const auto dpiX = LOWORD(msg->wParam);
        const auto dpiY = HIWORD(msg->wParam);
        // dpiX and dpiY are identical. Just to silence a compiler warning.
        const auto dpi = dpiX == dpiY ? dpiY : dpiX;
        qDebug().noquote() << "Window DPI changed: new DPI -->" << dpi
                           << "new DPR -->" << qreal(dpi) / qreal(m_defaultDPI);
        // FIXME: Temporary solution.
        refreshWindow(msg->hwnd);
        break;
    }
    default:
        break;
    }
    return false;
}

void WinNativeEventFilter::init(LPWINDOW data) {
    // Make sure we don't init the same window twice.
    data->inited = TRUE;
    // Make sure our window is a normal application window, we'll remove the
    // window frame later in Win32 events, don't use WS_POPUP to do this.
    SetWindowLongPtrW(data->hWnd, GWL_STYLE,
                      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    // Make our window a layered window to get better performance. It's also needed to remove the three
    // system buttons (minimize, maximize and close) with the help of the next line.
    SetWindowLongPtrW(data->hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_LAYERED);
    if (!data->windowData.blurEnabled) {
        // The following line is necessary if blur is disabled, without it, the window can't be shown.
        // But if blur is enabled, this line will make the window totally dark.
        SetLayeredWindowAttributes(data->hWnd, RGB(255, 0, 255), 0,
                                   LWA_COLORKEY);
    }
    // Make sure our window has the frame shadow.
    // According to MSDN, SetWindowLong won't take effect unless we trigger a frame change event manually,
    // we will do it inside the handleDwmCompositionChanged function, so it's not necessary to
    // do it here.
    handleDwmCompositionChanged(data);
    handleThemeChanged(data);
    // For debug purposes.
    qDebug().noquote() << "Window handle:" << data->hWnd;
    qDebug().noquote() << "Window DPI:" << windowDpi(data->hWnd)
                       << "Window DPR:" << windowDpr(data->hWnd) << "(May be different from Qt because they are calculated from raw data)";
    qDebug().noquote() << "Window border width:" << borderWidth(data->hWnd) << "(Expected)" << data->windowData.borderWidth << "(User defined)";
    qDebug().noquote() << "Window border height:" << borderHeight(data->hWnd) << "(Expected)" << data->windowData.borderHeight << "(User defined)";
    qDebug().noquote() << "Window titlebar height:" << titlebarHeight(data->hWnd) << "(Expected)" << data->windowData.titlebarHeight << "(User defined)";
}

void WinNativeEventFilter::handleDwmCompositionChanged(LPWINDOW data) {
    BOOL enabled = FALSE;
    DwmIsCompositionEnabled(&enabled);
    data->dwmCompositionEnabled = enabled;
    // We should not draw the frame shadow if DWM composition is disabled, in
    // other words, a window should not have frame shadow when Windows Aero is
    // not enabled.
    // Note that, start from Win8, the DWM composition is always enabled and
    // can't be disabled.
    if (enabled) {
        // The frame shadow is drawn on the non-client area and thus we have to
        // make sure the non-client area rendering is enabled first.
        const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        DwmSetWindowAttribute(data->hWnd, DWMWA_NCRENDERING_POLICY, &ncrp,
                              sizeof(ncrp));
        // Negative margins have special meaning to
        // DwmExtendFrameIntoClientArea. Negative margins create the "sheet of
        // glass" effect, where the client area is rendered as a solid surface
        // with no window border.
        const MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(data->hWnd, &margins);
    }
    // Has big side-effect.
    // handleBlurForWindow(data);
    refreshWindow(data->hWnd);
}

void WinNativeEventFilter::handleThemeChanged(LPWINDOW data) {
    data->themeEnabled = IsThemeActive();
}

void WinNativeEventFilter::handleBlurForWindow(LPWINDOW data) {
    if (QOperatingSystemVersion::current() <
        QOperatingSystemVersion::Windows7) {
        return;
    }
    // We prefer using DWM blur on Windows 7 because it has better appearance.
    // It's supported on Windows Vista as well actually, but Qt has drop it, so
    // we won't do it for Vista.
    const BOOL enableBlur =
        data->dwmCompositionEnabled && data->windowData.blurEnabled;
    if (QOperatingSystemVersion::current() <
        QOperatingSystemVersion::Windows8) {
        // Windows Aero
        DWM_BLURBEHIND dwmbb;
        dwmbb.dwFlags = DWM_BB_ENABLE;
        dwmbb.fEnable = enableBlur;
        dwmbb.hRgnBlur = nullptr;
        dwmbb.fTransitionOnMaximized = FALSE;
        DwmEnableBlurBehindWindow(data->hWnd, &dwmbb);
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
        if (enableBlur) {
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
        } else {
            accentPolicy.AccentState = ACCENT_DISABLED;
        }
        m_SetWindowCompositionAttribute(data->hWnd, &attribData);
    }
}

UINT WinNativeEventFilter::getDpiForWindow(HWND handle) const {
    const auto getScreenDpi = [](UINT defaultValue) -> UINT {
        // Available since Windows 7.
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
        // Available since Windows 2000.
        const HDC hdc = GetDC(nullptr);
        if (hdc) {
            const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
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
            return getScreenDpi(m_defaultDPI);
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
    return getScreenDpi(m_defaultDPI);
}

qreal WinNativeEventFilter::getDprForWindow(HWND handle) const {
    qreal dpr = handle ? (qreal(getDpiForWindow(handle)) / qreal(m_defaultDPI))
                       : m_defaultDPR;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    switch (QGuiApplication::highDpiScaleFactorRoundingPolicy()) {
    case Qt::HighDpiScaleFactorRoundingPolicy::PassThrough:
        // Default for Qt 6.
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Round:
        dpr = qRound(dpr);
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Floor:
        dpr = qFloor(dpr);
        break;
    case Qt::HighDpiScaleFactorRoundingPolicy::Ceil:
        dpr = qCeil(dpr);
        break;
    default:
        break;
    }
#else
    // Default behavior from Qt 5.6 to 5.15
    dpr = qRound(dpr);
#endif
    return dpr;
}

int WinNativeEventFilter::getSystemMetricsForWindow(HWND handle,
                                                    int index) const {
    if (m_GetSystemMetricsForDpi) {
        return m_GetSystemMetricsForDpi(index, getDpiForWindow(handle));
    } else {
        return GetSystemMetrics(index) * getDprForWindow(handle);
    }
}

void WinNativeEventFilter::setWindowData(HWND window, WINDOWDATA *data) {
    if (window && data) {
        createUserData(window, data);
        refreshWindow(window);
    }
}

WinNativeEventFilter::WINDOWDATA *
WinNativeEventFilter::windowData(HWND window) {
    if (window) {
        createUserData(window);
        return &reinterpret_cast<LPWINDOW>(
            GetWindowLongPtrW(window, GWLP_USERDATA))->windowData;
    }
    return nullptr;
}

void WinNativeEventFilter::createUserData(HWND handle, WINDOWDATA *data)
{
    if (handle) {
        const auto userData = reinterpret_cast<LPWINDOW>(GetWindowLongPtrW(handle, GWLP_USERDATA));
        if (userData) {
            if (data) {
                if (userData->windowData.blurEnabled != data->blurEnabled) {
                    qDebug().noquote() << "Due to technical issue, you can only enable or disable blur before the window is shown.";
                }
                userData->windowData = *data;
            }
        } else {
            LPWINDOW _data = new WINDOW;
            _data->hWnd = handle;
            if (data) {
                _data->windowData = *data;
            }
            SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(_data));
        }
    }
}
