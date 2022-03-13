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

#include "utilities.h"
#include <QtCore/private/qsystemlibrary_p.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#  include <QtCore/qoperatingsystemversion.h>
#else
#  include <QtCore/qsysinfo.h>
#endif
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qguiapplication.h>
#  include <QtGui/qpa/qplatformnativeinterface.h>
#else
#  include <QtGui/qpa/qplatformwindow_p.h>
#endif
#include "qwinregistry_p.h"
#include "framelesshelper_windows.h"

Q_DECLARE_METATYPE(QMargins)

FRAMELESSHELPER_BEGIN_NAMESPACE

#if (QT_VERSION < QT_VERSION_CHECK(5, 9, 0))
[[nodiscard]] static inline bool isWindowsVersionOrGreater(const DWORD dwMajor, const DWORD dwMinor, const DWORD dwBuild)
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = dwMajor;
    osvi.dwMinorVersion = dwMinor;
    osvi.dwBuildNumber = dwBuild;
    DWORDLONG dwlConditionMask = 0;
    const auto op = VER_GREATER_EQUAL;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}
#endif

[[nodiscard]] static inline QString __getSystemErrorMessage(const QString &function, const DWORD code)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    if (code == ERROR_SUCCESS) {
        return {};
    }
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buf), 0, nullptr) == 0) {
        return {};
    }
    const QString message = QStringLiteral("Function %1() failed with error code %2: %3.")
                                .arg(function, QString::number(code), QString::fromWCharArray(buf));
    LocalFree(buf);
    return message;
}

[[nodiscard]] static inline QString __getSystemErrorMessage(const QString &function, const HRESULT hr)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    if (SUCCEEDED(hr)) {
        return {};
    }
    const DWORD dwError = HRESULT_CODE(hr);
    return __getSystemErrorMessage(function, dwError);
}

[[nodiscard]] static inline int getSystemMetrics2(const WId winId, const int index,
                                                  const bool horizontal, const bool scaled)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 0;
    }
    const UINT windowDpi = Utilities::getWindowDpi(winId, horizontal);
    static const auto pGetSystemMetricsForDpi =
        reinterpret_cast<decltype(&GetSystemMetricsForDpi)>(
            QSystemLibrary::resolve(QStringLiteral("user32"), "GetSystemMetricsForDpi"));
    if (pGetSystemMetricsForDpi) {
        const UINT dpi = (scaled ? windowDpi : USER_DEFAULT_SCREEN_DPI);
        return pGetSystemMetricsForDpi(index, dpi);
    } else {
        // The returned value is already scaled, we need to divide the dpr to get the unscaled value.
        const qreal dpr = (scaled ? 1.0 : (qreal(windowDpi) / qreal(USER_DEFAULT_SCREEN_DPI)));
        return static_cast<int>(qRound(qreal(GetSystemMetrics(index)) / dpr));
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
[[nodiscard]] static inline DWORD qtEdgesToWin32Orientation(const Qt::Edges edges)
{
    if (edges == Qt::Edges{}) {
        return 0;
    }
    if (edges == (Qt::LeftEdge)) {
        return 0xF001; // SC_SIZELEFT
    } else if (edges == (Qt::RightEdge)) {
        return 0xF002; // SC_SIZERIGHT
    } else if (edges == (Qt::TopEdge)) {
        return 0xF003; // SC_SIZETOP
    } else if (edges == (Qt::TopEdge | Qt::LeftEdge)) {
        return 0xF004; // SC_SIZETOPLEFT
    } else if (edges == (Qt::TopEdge | Qt::RightEdge)) {
        return 0xF005; // SC_SIZETOPRIGHT
    } else if (edges == (Qt::BottomEdge)) {
        return 0xF006; // SC_SIZEBOTTOM
    } else if (edges == (Qt::BottomEdge | Qt::LeftEdge)) {
        return 0xF007; // SC_SIZEBOTTOMLEFT
    } else if (edges == (Qt::BottomEdge | Qt::RightEdge)) {
        return 0xF008; // SC_SIZEBOTTOMRIGHT
    } else {
        return 0xF000; // SC_SIZE
    }
}
#endif

bool Utilities::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8);
#endif
    return result;
}

bool Utilities::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1);
#endif
    return result;
}

bool Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}

bool Utilities::isWin11OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22000));
#else
    static const bool result = isWindowsVersionOrGreater(10, 0, 22000);
#endif
    return result;
}

bool Utilities::isDwmCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    const auto resultFromRegistry = []() -> bool {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, kDwmRegistryKey);
        const auto result = registry.dwordValue(QStringLiteral("Composition"));
        return (result.second && (result.first != 0));
    };
    static const auto pDwmIsCompositionEnabled =
        reinterpret_cast<decltype(&DwmIsCompositionEnabled)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmIsCompositionEnabled"));
    if (!pDwmIsCompositionEnabled) {
        return resultFromRegistry();
    }
    BOOL enabled = FALSE;
    const HRESULT hr = pDwmIsCompositionEnabled(&enabled);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(QStringLiteral("DwmIsCompositionEnabled"), hr);
        return resultFromRegistry();
    }
    return (enabled != FALSE);
}

void Utilities::triggerFrameChange(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    static constexpr const UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetWindowPos"));
    }
}

void Utilities::updateWindowFrameMargins(const WId winId, const bool reset)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    // DwmExtendFrameIntoClientArea() will always fail if DWM composition is disabled.
    // No need to try in this case.
    if (!isDwmCompositionEnabled()) {
        return;
    }
    static const auto pDwmExtendFrameIntoClientArea =
        reinterpret_cast<decltype(&DwmExtendFrameIntoClientArea)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmExtendFrameIntoClientArea"));
    if (!pDwmExtendFrameIntoClientArea) {
        return;
    }
    const MARGINS margins = [reset, winId]() -> MARGINS {
        if (reset) {
            return {0, 0, 0, 0};
        }
        if (isWin10OrGreater()) {
            return {0, static_cast<int>(getTitleBarHeight(winId, true)), 0, 0};
        } else {
            return {1, 1, 1, 1};
        }
    }();
    const auto hwnd = reinterpret_cast<HWND>(winId);
    const HRESULT hr = pDwmExtendFrameIntoClientArea(hwnd, &margins);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(QStringLiteral("DwmExtendFrameIntoClientArea"), hr);
        return;
    }
    triggerFrameChange(winId);
}

void Utilities::updateInternalWindowFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const WId winId = window->winId();
    const QMargins margins = [winId]() -> QMargins {
        const int titleBarHeight = getTitleBarHeight(winId, true);
        if (isWin10OrGreater()) {
            return {0, -titleBarHeight, 0, 0};
        } else {
            const int frameSizeX = getResizeBorderThickness(winId, true, true);
            const int frameSizeY = getResizeBorderThickness(winId, false, true);
            return {-frameSizeX, -titleBarHeight, -frameSizeX, -frameSizeY};
        }
    }();
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty("_q_windowsCustomMargins", marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QStringLiteral("WindowsCustomMargins"), marginsVar);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
        return;
    }
#else
    auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
        window->handle());
    if (platformWindow) {
        platformWindow->setCustomMargins(margins);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
        return;
    }
#endif
    triggerFrameChange(winId);
}

QString Utilities::getSystemErrorMessage(const QString &function)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    const DWORD code = GetLastError();
    if (code == ERROR_SUCCESS) {
        return {};
    }
    return __getSystemErrorMessage(function, code);
}

QColor Utilities::getDwmColorizationColor()
{
    const auto resultFromRegistry = []() -> QColor {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, kDwmRegistryKey);
        const auto result = registry.dwordValue(QStringLiteral("ColorizationColor"));
        return (result.second ? QColor::fromRgba(result.first) : Qt::darkGray);
    };
    static const auto pDwmGetColorizationColor =
        reinterpret_cast<decltype(&DwmGetColorizationColor)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetColorizationColor"));
    if (!pDwmGetColorizationColor) {
        return resultFromRegistry();
    }
    DWORD color = 0;
    BOOL opaque = FALSE;
    const HRESULT hr = pDwmGetColorizationColor(&color, &opaque);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(QStringLiteral("DwmGetColorizationColor"), hr);
        return resultFromRegistry();
    }
    return QColor::fromRgba(color);
}

bool Utilities::shouldAppsUseDarkMode()
{
    // The global dark mode was first introduced in Windows 10 1809.
    if (!isWin101809OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, kPersonalizeRegistryKey);
        const auto result = registry.dwordValue(QStringLiteral("AppsUseLightTheme"));
        return (result.second && (result.first == 0));
    };
    // Starting from Windows 10 1903, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    return resultFromRegistry();
}

DwmColorizationArea Utilities::getDwmColorizationArea()
{
    // It's a Win10 only feature.
    if (!isWin10OrGreater()) {
        return DwmColorizationArea::None;
    }
    const QString keyName = QStringLiteral("ColorPrevalence");
    const QWinRegistryKey themeRegistry(HKEY_CURRENT_USER, kPersonalizeRegistryKey);
    const auto themeValue = themeRegistry.dwordValue(keyName);
    const QWinRegistryKey dwmRegistry(HKEY_CURRENT_USER, kDwmRegistryKey);
    const auto dwmValue = dwmRegistry.dwordValue(keyName);
    const bool theme = themeValue.second && (themeValue.first != 0);
    const bool dwm = dwmValue.second && (dwmValue.first != 0);
    if (theme && dwm) {
        return DwmColorizationArea::All;
    } else if (theme) {
        return DwmColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return DwmColorizationArea::TitleBar_WindowBorder;
    }
    return DwmColorizationArea::None;
}

void Utilities::showSystemMenu(const WId winId, const QPointF &pos)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hWnd = reinterpret_cast<HWND>(winId);
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMenu"));
        return;
    }
    // Update the options based on window state.
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = MFT_STRING;
    const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
        mii.fState = (enabled ? MF_ENABLED : MF_DISABLED);
        if (SetMenuItemInfoW(menu, item, FALSE, &mii) == FALSE) {
            qWarning() << getSystemErrorMessage(QStringLiteral("SetMenuItemInfoW"));
            return false;
        }
        return true;
    };
    const bool max = IsMaximized(hWnd);
    if (!setState(SC_RESTORE, max)) {
        return;
    }
    if (!setState(SC_MOVE, !max)) {
        return;
    }
    if (!setState(SC_SIZE, !max)) {
        return;
    }
    if (!setState(SC_MINIMIZE, true)) {
        return;
    }
    if (!setState(SC_MAXIMIZE, !max)) {
        return;
    }
    if (!setState(SC_CLOSE, true)) {
        return;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetMenuDefaultItem"));
        return;
    }
    const QPoint roundedPos = pos.toPoint();
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, roundedPos.x(), roundedPos.y(), 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            qWarning() << getSystemErrorMessage(QStringLiteral("PostMessageW"));
        }
    }
}

bool Utilities::isFullScreen(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    RECT wndRect = {};
    if (GetWindowRect(hwnd, &wndRect) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetWindowRect"));
        return false;
    }
    // According to Microsoft Docs, we should compare to primary screen's geometry.
    const HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    if (!mon) {
        qWarning() << getSystemErrorMessage(QStringLiteral("MonitorFromWindow"));
        return false;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetMonitorInfoW"));
        return false;
    }
    // Compare to the full area of the screen, not the work area.
    const RECT scrRect = mi.rcMonitor;
    return ((wndRect.left == scrRect.left) && (wndRect.top == scrRect.top)
            && (wndRect.right == scrRect.right) && (wndRect.bottom == scrRect.bottom));
}

bool Utilities::isWindowNoState(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp); // This line is important! Don't miss it!
    if (GetWindowPlacement(hwnd, &wp) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetWindowPlacement"));
        return false;
    }
    return ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE));
}

void Utilities::syncWmPaintWithDwm()
{
    // No need to sync with DWM if DWM composition is disabled.
    if (!isDwmCompositionEnabled()) {
        return;
    }
    QSystemLibrary winmmLib(QStringLiteral("winmm"));
    static const auto ptimeGetDevCaps =
        reinterpret_cast<decltype(&timeGetDevCaps)>(winmmLib.resolve("timeGetDevCaps"));
    static const auto ptimeBeginPeriod =
        reinterpret_cast<decltype(&timeBeginPeriod)>(winmmLib.resolve("timeBeginPeriod"));
    static const auto ptimeEndPeriod =
        reinterpret_cast<decltype(&timeEndPeriod)>(winmmLib.resolve("timeEndPeriod"));
    static const auto pDwmGetCompositionTimingInfo =
        reinterpret_cast<decltype(&DwmGetCompositionTimingInfo)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetCompositionTimingInfo"));
    if (!ptimeGetDevCaps || !ptimeBeginPeriod || !ptimeEndPeriod || !pDwmGetCompositionTimingInfo) {
        return;
    }
    // Dirty hack to workaround the resize flicker caused by DWM.
    LARGE_INTEGER freq = {};
    if (QueryPerformanceFrequency(&freq) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("QueryPerformanceFrequency"));
        return;
    }
    TIMECAPS tc = {};
    if (ptimeGetDevCaps(&tc, sizeof(tc)) != MMSYSERR_NOERROR) {
        qWarning() << "timeGetDevCaps() failed.";
        return;
    }
    const UINT ms_granularity = tc.wPeriodMin;
    if (ptimeBeginPeriod(ms_granularity) != TIMERR_NOERROR) {
        qWarning() << "timeBeginPeriod() failed.";
        return;
    }
    LARGE_INTEGER now0 = {};
    if (QueryPerformanceCounter(&now0) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("QueryPerformanceCounter"));
        return;
    }
    // ask DWM where the vertical blank falls
    DWM_TIMING_INFO dti;
    SecureZeroMemory(&dti, sizeof(dti));
    dti.cbSize = sizeof(dti);
    const HRESULT hr = pDwmGetCompositionTimingInfo(nullptr, &dti);
    if (FAILED(hr)) {
        qWarning() << getSystemErrorMessage(QStringLiteral("DwmGetCompositionTimingInfo"));
        return;
    }
    LARGE_INTEGER now1 = {};
    if (QueryPerformanceCounter(&now1) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("QueryPerformanceCounter"));
        return;
    }
    // - DWM told us about SOME vertical blank
    //   - past or future, possibly many frames away
    // - convert that into the NEXT vertical blank
    const LONGLONG period = dti.qpcRefreshPeriod;
    const LONGLONG dt = dti.qpcVBlank - now1.QuadPart;
    LONGLONG w = 0, m = 0;
    if (dt >= 0) {
        w = dt / period;
    } else {
        // reach back to previous period
        // - so m represents consistent position within phase
        w = -1 + dt / period;
    }
    m = dt - (period * w);
    Q_ASSERT(m >= 0);
    Q_ASSERT(m < period);
    const qreal m_ms = 1000.0 * static_cast<qreal>(m) / static_cast<qreal>(freq.QuadPart);
    Sleep(static_cast<DWORD>(qRound(m_ms)));
    if (ptimeEndPeriod(ms_granularity) != TIMERR_NOERROR) {
        qWarning() << "timeEndPeriod() failed.";
    }
}

bool Utilities::isWin101809OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10_1809);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17763));
#else
    static const bool result = isWindowsVersionOrGreater(10, 0, 17763);
#endif
    return result;
}

bool Utilities::isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SystemParametersInfoW"));
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

quint32 Utilities::getPrimaryScreenDpi(const bool horizontal)
{
    static const auto pGetDpiForMonitor =
        reinterpret_cast<decltype(&GetDpiForMonitor)>(
            QSystemLibrary::resolve(QStringLiteral("shcore"), "GetDpiForMonitor"));
    if (pGetDpiForMonitor) {
        const HMONITOR monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
        if (monitor) {
            UINT dpiX = 0, dpiY = 0;
            if (SUCCEEDED(pGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                return (horizontal ? dpiX : dpiY);
            }
        }
    }
    // todo: d2d
    const HDC hdc = GetDC(nullptr);
    if (hdc) {
        const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(nullptr, hdc);
        if (horizontal && (dpiX > 0)) {
            return dpiX;
        }
        if (!horizontal && (dpiY > 0)) {
            return dpiY;
        }
    }
    return USER_DEFAULT_SCREEN_DPI;
}

quint32 Utilities::getWindowDpi(const WId winId, const bool horizontal)
{
    Q_ASSERT(winId);
    if (!winId) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    QSystemLibrary user32Lib(QStringLiteral("user32"));
    static const auto pGetDpiForWindow =
        reinterpret_cast<decltype(&GetDpiForWindow)>(user32Lib.resolve("GetDpiForWindow"));
    if (pGetDpiForWindow) {
        return pGetDpiForWindow(hwnd);
    }
    static const auto pGetSystemDpiForProcess =
        reinterpret_cast<decltype(&GetSystemDpiForProcess)>(user32Lib.resolve("GetSystemDpiForProcess"));
    if (pGetSystemDpiForProcess) {
        return pGetSystemDpiForProcess(GetCurrentProcess());
    }
    static const auto pGetDpiForSystem =
        reinterpret_cast<decltype(&GetDpiForSystem)>(user32Lib.resolve("GetDpiForSystem"));
    if (pGetDpiForSystem) {
        return pGetDpiForSystem();
    }
    static const auto pGetDpiForMonitor =
        reinterpret_cast<decltype(&GetDpiForMonitor)>(
            QSystemLibrary::resolve(QStringLiteral("shcore"), "GetDpiForMonitor"));
    if (pGetDpiForMonitor) {
        const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (monitor) {
            UINT dpiX = 0, dpiY = 0;
            if (SUCCEEDED(pGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                return (horizontal ? dpiX : dpiY);
            }
        }
    }
    return getPrimaryScreenDpi(horizontal);
}

quint32 Utilities::getResizeBorderThickness(const WId winId, const bool horizontal, const bool scaled)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 0;
    }
    const int paddedBorderWidth = getSystemMetrics2(winId, SM_CXPADDEDBORDER, true, scaled);
    if (horizontal) {
        return (getSystemMetrics2(winId, SM_CXSIZEFRAME, true, scaled) + paddedBorderWidth);
    } else {
        return (getSystemMetrics2(winId, SM_CYSIZEFRAME, false, scaled) + paddedBorderWidth);
    }
}

quint32 Utilities::getCaptionHeight(const WId winId, const bool scaled)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 0;
    }
    return getSystemMetrics2(winId, SM_CYCAPTION, false, scaled);
}

quint32 Utilities::getTitleBarHeight(const WId winId, const bool scaled)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 0;
    }
    return (getCaptionHeight(winId, scaled) + getResizeBorderThickness(winId, false, scaled));
}

quint32 Utilities::getFrameBorderThickness(const WId winId, const bool scaled)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 0;
    }
    // There's no window frame border before Windows 10.
    if (!isWin10OrGreater()) {
        return 0;
    }
    static const auto pDwmGetWindowAttribute =
        reinterpret_cast<decltype(&DwmGetWindowAttribute)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetWindowAttribute"));
    if (!pDwmGetWindowAttribute) {
        return 0;
    }
    const UINT dpi = getWindowDpi(winId, true);
    const qreal scaleFactor = (qreal(dpi) / qreal(USER_DEFAULT_SCREEN_DPI));
    const auto hwnd = reinterpret_cast<HWND>(winId);
    UINT value = 0;
    if (SUCCEEDED(pDwmGetWindowAttribute(hwnd, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value)))) {
        const qreal dpr = (scaled ? 1.0 : scaleFactor);
        return static_cast<int>(qRound(qreal(value) / dpr));
    } else {
        const qreal dpr = (scaled ? scaleFactor : 1.0);
        return static_cast<int>(qRound(qreal(kDefaultWindowFrameBorderThickness) * dpr));
    }
}

QColor Utilities::getFrameBorderColor(const bool active)
{
    // There's no window frame border before Windows 10.
    // So we just return a default value which is based on most window managers.
    if (!isWin10OrGreater()) {
        return (active ? Qt::black : Qt::darkGray);
    }
    const bool dark = shouldAppsUseDarkMode();
    if (active) {
        const DwmColorizationArea area = getDwmColorizationArea();
        if ((area == DwmColorizationArea::TitleBar_WindowBorder) || (area == DwmColorizationArea::All)) {
            return getDwmColorizationColor();
        } else {
            return (dark ? QColor(QStringLiteral("#4d4d4d")) : QColor(Qt::white));
        }
    } else {
        return (dark ? QColor(QStringLiteral("#575959")) : QColor(QStringLiteral("#999999")));
    }
}

void Utilities::updateWindowFrameBorderColor(const WId winId, const bool dark)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    // There's no global dark theme before Win10 1809.
    if (!isWin101809OrGreater()) {
        return;
    }
    static const auto pSetWindowTheme =
        reinterpret_cast<decltype(&SetWindowTheme)>(
            QSystemLibrary::resolve(QStringLiteral("uxtheme"), "SetWindowTheme"));
    static const auto pDwmSetWindowAttribute =
        reinterpret_cast<decltype(&DwmSetWindowAttribute)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmSetWindowAttribute"));
    if (!pSetWindowTheme || !pDwmSetWindowAttribute) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    const BOOL value = (dark ? TRUE : FALSE);
    HRESULT hr = pDwmSetWindowAttribute(hwnd, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &value, sizeof(value));
    if (FAILED(hr)) {
        // Just eat this error, because it only works on systems before Win10 20H1.
    }
    hr = pDwmSetWindowAttribute(hwnd, _DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    if (FAILED(hr)) {
        // Eat this error, it only works on systems greater than or equal to Win10 20H1.
    }
    hr = pSetWindowTheme(hwnd, (dark ? L"DarkMode_Explorer" : L" "), nullptr);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(QStringLiteral("SetWindowTheme"), hr);
    }
}

void Utilities::fixupQtInternals(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    SetLastError(ERROR_SUCCESS);
    const auto oldClassStyle = static_cast<DWORD>(GetClassLongPtrW(hwnd, GCL_STYLE));
    if (oldClassStyle == 0) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetClassLongPtrW"));
        return;
    }
    const DWORD newClassStyle = (oldClassStyle | CS_HREDRAW | CS_VREDRAW);
    SetLastError(ERROR_SUCCESS);
    if (SetClassLongPtrW(hwnd, GCL_STYLE, static_cast<LONG_PTR>(newClassStyle)) == 0) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetClassLongPtrW"));
        return;
    }
    SetLastError(ERROR_SUCCESS);
    const auto oldWindowStyle = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (oldWindowStyle == 0) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetWindowLongPtrW"));
        return;
    }
    const DWORD newWindowStyle = (oldWindowStyle & ~WS_POPUP) | WS_OVERLAPPED;
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWL_STYLE, static_cast<LONG_PTR>(newWindowStyle)) == 0) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetWindowLongPtrW"));
    }
}

void Utilities::startSystemMove(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    if (ReleaseCapture() == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("ReleaseCapture"));
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, 0xF012 /*SC_DRAGMOVE*/, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("PostMessageW"));
    }
#endif
}

void Utilities::startSystemResize(QWindow *window, const Qt::Edges edges)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemResize(edges);
#else
    if (edges == Qt::Edges{}) {
        return;
    }
    if (ReleaseCapture() == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("ReleaseCapture"));
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, qtEdgesToWin32Orientation(edges), 0) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("PostMessageW"));
    }
#endif
}

FRAMELESSHELPER_END_NAMESPACE
