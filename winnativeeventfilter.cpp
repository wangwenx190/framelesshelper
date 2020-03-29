#include "winnativeeventfilter.h"

#include <QGuiApplication>
#include <QLibrary>
#include <QOperatingSystemVersion>
#include <dwmapi.h>
#include <shellapi.h>
#include <windowsx.h>

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION 0x00AE
#endif

#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME 0x00AF
#endif

WinNativeEventFilter *WinNativeEventFilter::instance = nullptr;

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
}

WinNativeEventFilter::~WinNativeEventFilter() {
    instance = nullptr;
    if (!m_data.isEmpty()) {
        for (auto data : m_data) {
            delete data;
        }
    }
}

void WinNativeEventFilter::setup() {
    if (!instance) {
        instance = new WinNativeEventFilter;
        qApp->installNativeEventFilter(instance);
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
    if (!m_data.contains(msg->hwnd)) {
        LPWINDOW _data = new WINDOW;
        _data->hwnd = msg->hwnd;
        m_data.insert(msg->hwnd, _data);
        init(_data);
    }
    const auto data = m_data.value(msg->hwnd);
    switch (msg->message) {
    case WM_NCCALCSIZE: {
        handleNcCalcSize(data, msg->wParam, msg->lParam);
        if (static_cast<BOOL>(msg->wParam)) {
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
        if (data->compositionEnabled) {
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
        // a HRGN) is -1. This is recommended in:
        // https://blogs.msdn.microsoft.com/wpfsdk/2008/09/08/custom-window-chrome-in-wpf/
        *result = DefWindowProcW(data->hwnd, msg->message, msg->wParam, -1);
        return true;
    }
    case WM_WINDOWPOSCHANGED: {
        handleWindowPosChanged(data, msg->lParam);
        *result = 0;
        // Don't return true here because return true means we'll take over
        // the whole event from Qt and thus it will block Qt's paint events and
        // the window will not update itself anymore in the end.
        break;
    }
    case WM_NCHITTEST: {
        *result = handleNcHitTest(data, msg->lParam);
        return true;
    }
    default:
        break;
    }
    return false;
}

void WinNativeEventFilter::init(LPWINDOW data) {
    // Make sure our window is a normal application window, we'll remove the
    // window frame later in Win32 events, don't use WS_POPUP to do this.
    SetWindowLongPtrW(data->hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowLongPtrW(data->hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_LAYERED);
    // Make the window a layered window so the legacy GDI API can be used to
    // draw to it without messing up the area on top of the DWM frame. Note:
    // This is not necessary if other drawing APIs are used, eg. GDI+, OpenGL,
    // Direct2D, Direct3D, DirectComposition, etc.
    SetLayeredWindowAttributes(data->hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    // Make sure our window has the frame shadow.
    handleDwmCompositionChanged(data);
}

void WinNativeEventFilter::handleNcCalcSize(LPWINDOW data, WPARAM wParam,
                                            LPARAM lParam) {
    union {
        LPARAM lParam;
        RECT *rect;
    } params;
    params.lParam = lParam;
    // DefWindowProc must be called in both the maximized and non-maximized
    // cases, otherwise tile/cascade windows won't work.
    const RECT nonclient = *params.rect;
    DefWindowProcW(data->hwnd, WM_NCCALCSIZE, wParam, params.lParam);
    const RECT client = *params.rect;
    if (IsMaximized(data->hwnd)) {
        WINDOWINFO wi;
        SecureZeroMemory(&wi, sizeof(wi));
        wi.cbSize = sizeof(wi);
        GetWindowInfo(data->hwnd, &wi);
        // Maximized windows always have a non-client border that hangs over
        // the edge of the screen, so the size proposed by WM_NCCALCSIZE is
        // fine. Just adjust the top border to remove the window title.
        RECT rect;
        rect.left = client.left;
        rect.top = nonclient.top + wi.cyWindowBorders;
        rect.right = client.right;
        rect.bottom = client.bottom;
        *params.rect = rect;
        const HMONITOR mon =
            MonitorFromWindow(data->hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi;
        SecureZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(mon, &mi);
        // If the client rectangle is the same as the monitor's rectangle,
        // the shell assumes that the window has gone fullscreen, so it
        // removes the topmost attribute from any auto-hide appbars, making
        // them inaccessible. To avoid this, reduce the size of the client
        // area by one pixel on a certain edge. The edge is chosen based on
        // which side of the monitor is likely to contain an auto-hide
        // appbar, so the missing client area is covered by it.
        if (EqualRect(params.rect, &mi.rcMonitor)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = SHAppBarMessage(ABM_GETSTATE, &abd);
            if (taskbarState & ABS_AUTOHIDE) {
                UINT edge = -1;
                abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                if (abd.hWnd) {
                    const HMONITOR taskbarMonitor =
                        MonitorFromWindow(abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                    if (taskbarMonitor == mon) {
                        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
                        edge = abd.uEdge;
                    }
                }
                if (edge == ABE_BOTTOM) {
                    params.rect->bottom--;
                } else if (edge == ABE_LEFT) {
                    params.rect->left++;
                } else if (edge == ABE_TOP) {
                    params.rect->top++;
                } else if (edge == ABE_RIGHT) {
                    params.rect->right--;
                }
            }
        }
    } else {
        // For the non-maximized case, set the output RECT to what it was
        // before WM_NCCALCSIZE modified it. This will make the client size
        // the same as the non-client size.
        *params.rect = nonclient;
    }
}

void WinNativeEventFilter::updateRegion(LPWINDOW data) {
    const RECT old_rgn = data->region;
    const RECT null_rgn = {0, 0, 0, 0};
    if (IsMaximized(data->hwnd)) {
        WINDOWINFO wi;
        SecureZeroMemory(&wi, sizeof(wi));
        wi.cbSize = sizeof(wi);
        GetWindowInfo(data->hwnd, &wi);
        // For maximized windows, a region is needed to cut off the
        // non-client borders that hang over the edge of the screen.
        data->region.left = wi.rcClient.left - wi.rcWindow.left;
        data->region.top = wi.rcClient.top - wi.rcWindow.top;
        data->region.right = wi.rcClient.right - wi.rcWindow.left;
        data->region.bottom = wi.rcClient.bottom - wi.rcWindow.top;
    } else if (!data->compositionEnabled) {
        // For ordinary themed windows when composition is disabled, a
        // region is needed to remove the rounded top corners. Make it as
        // large as possible to avoid having to change it when the window is
        // resized.
        data->region.left = 0;
        data->region.top = 0;
        data->region.right = 32767;
        data->region.bottom = 32767;
    } else {
        // Don't mess with the region when composition is enabled and the
        // window is not maximized, otherwise it will lose its shadow.
        data->region = null_rgn;
    }
    // Avoid unnecessarily updating the region to avoid unnecessary redraws.
    if (EqualRect(&data->region, &old_rgn)) {
        return;
    }
    // Treat empty regions as NULL regions.
    if (EqualRect(&data->region, &null_rgn)) {
        SetWindowRgn(data->hwnd, nullptr, TRUE);
    } else {
        SetWindowRgn(data->hwnd, CreateRectRgnIndirect(&data->region), TRUE);
    }
}

void WinNativeEventFilter::handleDwmCompositionChanged(LPWINDOW data) {
    BOOL enabled = FALSE;
    DwmIsCompositionEnabled(&enabled);
    data->compositionEnabled = enabled;
    // We should not draw the frame shadow if DWM composition is disabled, in
    // other words, a window should not have frame shadow when Windows Aero is
    // not enabled.
    // Note that, start from Win8, the DWM composition is always enabled and
    // can't be disabled.
    if (enabled) {
        // The frame shadow is drawn on the non-client area and thus we have to
        // make sure the non-client area rendering is enabled first.
        const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        DwmSetWindowAttribute(data->hwnd, DWMWA_NCRENDERING_POLICY, &ncrp,
                              sizeof(ncrp));
        // Negative margins have special meaning to
        // DwmExtendFrameIntoClientArea. Negative margins create the "sheet of
        // glass" effect, where the client area is rendered as a solid surface
        // with no window border.
        const MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(data->hwnd, &margins);
    }
    updateRegion(data);
}

void WinNativeEventFilter::handleWindowPosChanged(LPWINDOW data,
                                                  LPARAM lParam) {
    RECT client;
    GetClientRect(data->hwnd, &client);
    const UINT old_width = data->width;
    const UINT old_height = data->height;
    data->width = client.right;
    data->height = client.bottom;
    const bool client_changed =
        (data->width != old_width) || (data->height != old_height);
    if (client_changed ||
        (reinterpret_cast<const LPWINDOWPOS>(lParam)->flags &
         SWP_FRAMECHANGED)) {
        updateRegion(data);
    }
    if (client_changed) {
        // Invalidate the changed parts of the rectangle drawn in WM_PAINT.
        RECT rect;
        if (data->width > old_width) {
            rect = {LONG(old_width - 1), 0, LONG(old_width), LONG(old_height)};
        } else {
            rect = {LONG(data->width - 1), 0, LONG(data->width),
                    LONG(data->height)};
        }
        if (data->height > old_height) {
            rect = {0, LONG(old_height - 1), LONG(old_width), LONG(old_height)};
        } else {
            rect = {0, LONG(data->height - 1), LONG(data->width),
                    LONG(data->height)};
        }
        InvalidateRect(data->hwnd, &rect, TRUE);
    }
}

LRESULT WinNativeEventFilter::handleNcHitTest(LPWINDOW data, LPARAM lParam) {
    POINT mouse = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(data->hwnd, &mouse);
    // These values are DPI-aware.
    const UINT border_width = borderWidth(data->hwnd);
    const UINT border_height = borderHeight(data->hwnd);
    const UINT titlebar_height = titlebarHeight(data->hwnd);
    if (IsMaximized(data->hwnd)) {
        if (mouse.y < LONG(titlebar_height)) {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    if (mouse.y < LONG(border_height)) {
        if (mouse.x < LONG(border_width)) {
            return HTTOPLEFT;
        }
        if (mouse.x > LONG(data->width - border_width)) {
            return HTTOPRIGHT;
        }
        return HTTOP;
    }
    if (mouse.y > LONG(data->height - border_height)) {
        if (mouse.x < LONG(border_width)) {
            return HTBOTTOMLEFT;
        }
        if (mouse.x > LONG(data->width - border_width)) {
            return HTBOTTOMRIGHT;
        }
        return HTBOTTOM;
    }
    if (mouse.x < LONG(border_width)) {
        return HTLEFT;
    }
    if (mouse.x > LONG(data->width - border_width)) {
        return HTRIGHT;
    }
    if (mouse.y < LONG(titlebar_height)) {
        return HTCAPTION;
    }
    return HTCLIENT;
}

UINT WinNativeEventFilter::getDpiForWindow(HWND handle) const {
    if (!handle) {
        if (m_GetDpiForSystem) {
            return m_GetDpiForSystem();
        } else {
            return m_defaultDPI;
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
    // TODO: Is there an elegant way to acquire the system DPI in
    // Win7/8/10(before 1607)?
    return m_defaultDPI;
}

qreal WinNativeEventFilter::getDprForWindow(HWND handle) const {
    return handle ? (qreal(getDpiForWindow(handle)) / qreal(m_defaultDPI))
                  : m_defaultDPR;
}

int WinNativeEventFilter::getSystemMetricsForWindow(HWND handle,
                                                    int index) const {
    const UINT dpi = getDpiForWindow(handle);
    if (m_GetSystemMetricsForDpi) {
        return m_GetSystemMetricsForDpi(index, dpi);
    } else {
        return GetSystemMetrics(index) * getDprForWindow(handle);
    }
}
