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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "utilities.h"
#include <QtCore/qsettings.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qt_windows.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <dwmapi.h>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/qpa/qplatformnativeinterface.h>
#else
#include <QtGui/qpa/qplatformwindow_p.h>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif

Q_DECLARE_METATYPE(QMargins)

#ifndef USER_DEFAULT_SCREEN_DPI
// Only available since Windows Vista
#define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER 92
#endif

enum : WORD
{
    DwmwaUseImmersiveDarkMode = 20,
    DwmwaUseImmersiveDarkModeBefore20h1 = 19
};

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_ACCENT_POLICY = 19
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using ACCENT_STATE = enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

using ACCENT_POLICY = struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    COLORREF GradientColor;
    DWORD AnimationId;
};

using IMMERSIVE_HC_CACHE_MODE = enum _IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

using PREFERRED_APP_MODE = enum _PREFERRED_APP_MODE
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

static const QString g_dwmRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)");
static const QString g_personalizeRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)");

// The standard values of border width, border height and title bar height when DPI is 96.
static const int g_defaultBorderWidth = 8, g_defaultBorderHeight = 8, g_defaultTitleBarHeight = 31;

using MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE
{
    MDT_EFFECTIVE_DPI = 0
};

using PROCESS_DPI_AWARENESS = enum _PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
};

using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);
using ShouldAppsUseDarkModePtr =  BOOL(WINAPI *)();
using AllowDarkModeForWindowPtr = BOOL(WINAPI *)(HWND, BOOL);
using AllowDarkModeForAppPtr = BOOL(WINAPI *)(BOOL);
using IsDarkModeAllowedForWindowPtr = BOOL(WINAPI *)(HWND);
using GetIsImmersiveColorUsingHighContrastPtr = BOOL(WINAPI *)(IMMERSIVE_HC_CACHE_MODE);
using RefreshImmersiveColorPolicyStatePtr = VOID(WINAPI *)();
using ShouldSystemUseDarkModePtr = BOOL(WINAPI *)();
using SetPreferredAppModePtr = PREFERRED_APP_MODE(WINAPI *)(PREFERRED_APP_MODE);
using IsDarkModeAllowedForAppPtr = BOOL(WINAPI *)();

using GetDpiForMonitorPtr = HRESULT(WINAPI *)(HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *);
using GetProcessDpiAwarenessPtr = HRESULT(WINAPI *)(HANDLE, PROCESS_DPI_AWARENESS *);
using GetSystemDpiForProcessPtr = UINT(WINAPI *)(HANDLE);
using GetDpiForWindowPtr = UINT(WINAPI *)(HWND);
using GetDpiForSystemPtr = UINT(WINAPI *)();
using GetSystemMetricsForDpiPtr = int(WINAPI *)(int, UINT);
using AdjustWindowRectExForDpiPtr = BOOL(WINAPI *)(LPRECT, DWORD, BOOL, DWORD, UINT);

using Win32Data = struct _FLH_UTILITIES_WIN32_DATA
{
    _FLH_UTILITIES_WIN32_DATA() { load(); }

    SetWindowCompositionAttributePtr SetWindowCompositionAttributePFN = nullptr;
    ShouldAppsUseDarkModePtr ShouldAppsUseDarkModePFN = nullptr;
    AllowDarkModeForWindowPtr AllowDarkModeForWindowPFN = nullptr;
    AllowDarkModeForAppPtr AllowDarkModeForAppPFN = nullptr;
    IsDarkModeAllowedForWindowPtr IsDarkModeAllowedForWindowPFN = nullptr;
    GetIsImmersiveColorUsingHighContrastPtr GetIsImmersiveColorUsingHighContrastPFN = nullptr;
    RefreshImmersiveColorPolicyStatePtr RefreshImmersiveColorPolicyStatePFN = nullptr;
    ShouldSystemUseDarkModePtr ShouldSystemUseDarkModePFN = nullptr;
    SetPreferredAppModePtr SetPreferredAppModePFN = nullptr;
    IsDarkModeAllowedForAppPtr IsDarkModeAllowedForAppPFN = nullptr;

    GetDpiForMonitorPtr GetDpiForMonitorPFN = nullptr;
    GetProcessDpiAwarenessPtr GetProcessDpiAwarenessPFN = nullptr;
    GetSystemDpiForProcessPtr GetSystemDpiForProcessPFN = nullptr;
    GetDpiForWindowPtr GetDpiForWindowPFN = nullptr;
    GetDpiForSystemPtr GetDpiForSystemPFN = nullptr;
    GetSystemMetricsForDpiPtr GetSystemMetricsForDpiPFN = nullptr;
    AdjustWindowRectExForDpiPtr AdjustWindowRectExForDpiPFN = nullptr;

    void load()
    {
        QLibrary User32Dll(QStringLiteral("User32"));
        SetWindowCompositionAttributePFN = reinterpret_cast<SetWindowCompositionAttributePtr>(User32Dll.resolve("SetWindowCompositionAttribute"));
        GetDpiForWindowPFN = reinterpret_cast<GetDpiForWindowPtr>(User32Dll.resolve("GetDpiForWindow"));
        GetDpiForSystemPFN = reinterpret_cast<GetDpiForSystemPtr>(User32Dll.resolve("GetDpiForSystem"));
        GetSystemMetricsForDpiPFN = reinterpret_cast<GetSystemMetricsForDpiPtr>(User32Dll.resolve("GetSystemMetricsForDpi"));
        AdjustWindowRectExForDpiPFN = reinterpret_cast<AdjustWindowRectExForDpiPtr>(User32Dll.resolve("AdjustWindowRectExForDpi"));
        GetSystemDpiForProcessPFN = reinterpret_cast<GetSystemDpiForProcessPtr>(User32Dll.resolve("GetSystemDpiForProcess"));

        QLibrary UxThemeDll(QStringLiteral("UxTheme"));
        ShouldAppsUseDarkModePFN = reinterpret_cast<ShouldAppsUseDarkModePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(132)));
        AllowDarkModeForWindowPFN = reinterpret_cast<AllowDarkModeForWindowPtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(133)));
        AllowDarkModeForAppPFN = reinterpret_cast<AllowDarkModeForAppPtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(135)));
        RefreshImmersiveColorPolicyStatePFN = reinterpret_cast<RefreshImmersiveColorPolicyStatePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(104)));
        IsDarkModeAllowedForWindowPFN = reinterpret_cast<IsDarkModeAllowedForWindowPtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(137)));
        GetIsImmersiveColorUsingHighContrastPFN = reinterpret_cast<GetIsImmersiveColorUsingHighContrastPtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(106)));
        ShouldSystemUseDarkModePFN = reinterpret_cast<ShouldSystemUseDarkModePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(138)));
        SetPreferredAppModePFN = reinterpret_cast<SetPreferredAppModePtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(135)));
        IsDarkModeAllowedForAppPFN = reinterpret_cast<IsDarkModeAllowedForAppPtr>(UxThemeDll.resolve(MAKEINTRESOURCEA(139)));

        QLibrary SHCoreDll(QStringLiteral("SHCore"));
        GetDpiForMonitorPFN = reinterpret_cast<GetDpiForMonitorPtr>(SHCoreDll.resolve("GetDpiForMonitor"));
        GetProcessDpiAwarenessPFN = reinterpret_cast<GetProcessDpiAwarenessPtr>(SHCoreDll.resolve("GetProcessDpiAwareness"));
    }
};

Q_GLOBAL_STATIC(Win32Data, win32Data)

bool Utilities::isDwmBlurAvailable()
{
    if (isWin8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    if (FAILED(DwmIsCompositionEnabled(&enabled))) {
        qWarning() << "DwmIsCompositionEnabled failed.";
        return false;
    }
    return isWin7OrGreater() && (enabled == TRUE);
}

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiAware, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    const qreal dpr = dpiAware ? window->devicePixelRatio() : 1.0;
    const auto getSystemMetricsForWindow = [dpr](const int index, const bool dpiAware) -> int {
        if (win32Data()->GetSystemMetricsForDpiPFN) {
            const quint32 dpi = dpiAware ? qRound(USER_DEFAULT_SCREEN_DPI * dpr) : USER_DEFAULT_SCREEN_DPI;
            return win32Data()->GetSystemMetricsForDpiPFN(index, dpi);
        } else {
            const int value = GetSystemMetrics(index);
            return dpiAware ? value : qRound(value / dpr);
        }
    };
    int ret = 0;
    switch (metric) {
    case SystemMetric::BorderWidth: {
        const int bw = window->property(_flh_global::_flh_borderWidth_flag).toInt();
        if ((bw > 0) && !forceSystemValue) {
            ret = qRound(bw * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CXSIZEFRAME, false)
                                      + getSystemMetricsForWindow(SM_CXPADDEDBORDER, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CXSIZEFRAME, true)
                                   + getSystemMetricsForWindow(SM_CXPADDEDBORDER, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(g_defaultBorderWidth * dpr);
        }
    } break;
    case SystemMetric::BorderHeight: {
        const int bh = window->property(_flh_global::_flh_borderHeight_flag).toInt();
        if ((bh > 0) && !forceSystemValue) {
            ret = qRound(bh * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CYSIZEFRAME, false)
                                      + getSystemMetricsForWindow(SM_CXPADDEDBORDER, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CYSIZEFRAME, true)
                                   + getSystemMetricsForWindow(SM_CXPADDEDBORDER, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(g_defaultBorderHeight * dpr);
        }
    } break;
    case SystemMetric::TitleBarHeight: {
        const int tbh = window->property(_flh_global::_flh_titleBarHeight_flag).toInt();
        if ((tbh > 0) && !forceSystemValue) {
            // Special case: this is the user defined value,
            // don't change it and just return it untouched.
            return qRound(tbh * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CYCAPTION, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CYCAPTION, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(g_defaultTitleBarHeight * dpr);
        }
    } break;
    }
    // When dpr = 1.0 (DPI = 96):
    // SM_CXSIZEFRAME = SM_CYSIZEFRAME = 4px
    // SM_CXPADDEDBORDER = 4px
    // SM_CYCAPTION = 23px
    // Border Width = Border Height = SM_C(X|Y)SIZEFRAME + SM_CXPADDEDBORDER = 8px
    // Title Bar Height = Border Height + SM_CYCAPTION = 31px
    // dpr = 1.25 --> Title Bar Height = 38px
    // dpr = 1.5 --> Title Bar Height = 45px
    // dpr = 1.75 --> Title Bar Height = 51px
    ret += (metric == SystemMetric::TitleBarHeight) ? getSystemMetric(window, SystemMetric::BorderHeight, dpiAware) : 0;
    return ret;
}

bool Utilities::setBlurEffectEnabled(const QWindow *window, const bool enabled, const QColor &gradientColor)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    bool result = false;
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    // We prefer DwmEnableBlurBehindWindow on Windows 7.
    if (isWin8OrGreater() && win32Data()->SetWindowCompositionAttributePFN) {
        ACCENT_POLICY accentPolicy;
        SecureZeroMemory(&accentPolicy, sizeof(accentPolicy));
        WINDOWCOMPOSITIONATTRIBDATA wcaData;
        SecureZeroMemory(&wcaData, sizeof(wcaData));
        wcaData.Attrib = WCA_ACCENT_POLICY;
        wcaData.pvData = &accentPolicy;
        wcaData.cbData = sizeof(accentPolicy);
        if (enabled) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            if (gradientColor.isValid()) {
                accentPolicy.GradientColor = qRgba(gradientColor.blue(), gradientColor.green(), gradientColor.red(), gradientColor.alpha());
            } else {
                const QColor colorizationColor = getColorizationColor();
                accentPolicy.GradientColor =
                    RGB(qRound(colorizationColor.red() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.green() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.blue() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()));
            }
            if (isOfficialMSWin10AcrylicBlurAvailable()) {
                accentPolicy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                if (!gradientColor.isValid()) {
                    accentPolicy.GradientColor = 0x01FFFFFF;
                }
            } else {
                accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
            }
        } else {
            accentPolicy.AccentState = ACCENT_DISABLED;
        }
        result = win32Data()->SetWindowCompositionAttributePFN(hwnd, &wcaData) == TRUE;
        if (!result) {
            qWarning() << "SetWindowCompositionAttribute failed.";
        }
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = enabled ? TRUE : FALSE;
        result = SUCCEEDED(DwmEnableBlurBehindWindow(hwnd, &dwmBB));
        if (!result) {
            qWarning() << "DwmEnableBlurBehindWindow failed.";
        }
    }
    return result;
}

bool Utilities::isColorizationEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_dwmRegistryKey, QSettings::NativeFormat);
    const bool colorPrevalence
        = registry.value(QStringLiteral("ColorPrevalence"), 0).toULongLong(&ok) != 0;
    return (ok && colorPrevalence);
}

QColor Utilities::getColorizationColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    if (FAILED(DwmGetColorizationColor(&color, &opaqueBlend))) {
        qWarning() << "DwmGetColorizationColor failed.";
        return Qt::darkGray;
    }
    return QColor::fromRgba(color);
}

bool Utilities::isLightThemeEnabled()
{
    return !isDarkThemeEnabled();
}

bool Utilities::isDarkThemeEnabled()
{
    return win32Data()->ShouldSystemUseDarkModePFN ? win32Data()->ShouldSystemUseDarkModePFN() : false;
}

bool Utilities::isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &hc, 0) == FALSE) {
        qWarning() << "SystemParametersInfoW failed.";
        return false;
    }
    return hc.dwFlags & HCF_HIGHCONTRASTON;
}

bool Utilities::isDarkFrameEnabled(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    if (!isWin10OrGreater(17763)) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    BOOL result = FALSE;
    const bool ok = SUCCEEDED(DwmGetWindowAttribute(hwnd, DwmwaUseImmersiveDarkMode, &result, sizeof(result)))
                    || SUCCEEDED(DwmGetWindowAttribute(hwnd, DwmwaUseImmersiveDarkModeBefore20h1, &result, sizeof(result)));
    return (ok && result);
}

bool Utilities::isTransparencyEffectEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_personalizeRegistryKey, QSettings::NativeFormat);
    const bool enableTransparency
        = registry.value(QStringLiteral("EnableTransparency"), 0).toULongLong(&ok) != 0;
    return (ok && enableTransparency);
}

void Utilities::triggerFrameChange(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (SetWindowPos(reinterpret_cast<HWND>(window->winId()), nullptr, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
        qWarning() << "SetWindowPos failed.";
    }
}

void Utilities::updateFrameMargins(const QWindow *window, const bool reset)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const MARGINS margins = reset ? MARGINS{0, 0, 0, 0} : MARGINS{1, 1, 1, 1};
    if (FAILED(DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(window->winId()), &margins))) {
        qWarning() << "DwmExtendFrameIntoClientArea failed.";
    }
}

QImage Utilities::getDesktopWallpaperImage(const int screen)
{
    Q_UNUSED(screen);
    WCHAR path[MAX_PATH];
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0) == FALSE) {
        qWarning() << "SystemParametersInfoW failed.";
        return {};
    }
    return QImage(QString::fromWCharArray(path));
}

Utilities::DesktopWallpaperAspectStyle Utilities::getDesktopWallpaperAspectStyle(const int screen)
{
    Q_UNUSED(screen);
    const QSettings settings(QStringLiteral(R"(HKEY_CURRENT_USER\Control Panel\Desktop)"), QSettings::NativeFormat);
    const DWORD style = settings.value(QStringLiteral("WallpaperStyle")).toULongLong();
    switch (style) {
    case 0: {
        if (settings.value(QStringLiteral("TileWallpaper")).toBool()) {
            return DesktopWallpaperAspectStyle::Tiled;
        } else {
            return DesktopWallpaperAspectStyle::Central;
        }
    }
    case 2:
        return DesktopWallpaperAspectStyle::IgnoreRatio;
    case 6:
        return DesktopWallpaperAspectStyle::KeepRatio;
    default:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
    }
}

quint32 Utilities::getWindowDpi(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    if (QCoreApplication::testAttribute(Qt::AA_Use96Dpi)) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    if (win32Data()->GetDpiForWindowPFN) {
        return win32Data()->GetDpiForWindowPFN(hwnd);
    } else if (win32Data()->GetSystemDpiForProcessPFN) {
        return win32Data()->GetSystemDpiForProcessPFN(GetCurrentProcess());
    } else if (win32Data()->GetDpiForSystemPFN) {
        return win32Data()->GetDpiForSystemPFN();
    } else if (win32Data()->GetDpiForMonitorPFN) {
        const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (monitor) {
            UINT dpiX = USER_DEFAULT_SCREEN_DPI, dpiY = USER_DEFAULT_SCREEN_DPI;
            if (SUCCEEDED(win32Data()->GetDpiForMonitorPFN(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                return dpiX;
            } else {
                qWarning() << "GetDpiForMonitor failed";
            }
        } else {
            qWarning() << "MonitorFromWindow failed.";
        }
    }
    // Using Direct2D to get DPI is deprecated.
    const HDC hdc = GetDC(nullptr);
    if (hdc) {
        const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(nullptr, hdc);
        if (dpiX > 0) {
            return dpiX;
        } else {
            qWarning() << "GetDeviceCaps failed.";
        }
    }
    return USER_DEFAULT_SCREEN_DPI;
}

QMargins Utilities::getWindowNativeFrameMargins(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return {};
    }
    RECT rect = {0, 0, 0, 0};
    const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (win32Data()->AdjustWindowRectExForDpiPFN) {
        if (win32Data()->AdjustWindowRectExForDpiPFN(&rect, style, FALSE, exStyle, getWindowDpi(window)) == FALSE) {
            qWarning() << "AdjustWindowRectExForDpiPFN failed.";
        }
    } else {
        if (AdjustWindowRectEx(&rect, style, FALSE, exStyle) == FALSE) {
            qWarning() << "AdjustWindowRectEx failed.";
        }
    }
    return {qAbs(rect.left), qAbs(rect.top), qAbs(rect.right), qAbs(rect.bottom)};
}

QColor Utilities::getNativeWindowFrameColor(const bool isActive)
{
    if (!isActive) {
        return Qt::darkGray;
    }
    if (!isWin10OrGreater()) {
        return Qt::black;
    }
    return isColorizationEnabled() ? getColorizationColor() : (isDarkThemeEnabled() ? Qt::white : Qt::black);
}

void Utilities::updateQtFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const int tbh = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::TitleBarHeight, true, true) : 0;
    const QMargins margins = {0, -tbh, 0, 0};
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty("_q_windowsCustomMargins", marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QStringLiteral("WindowsCustomMargins"), marginsVar);
    }
#else
    auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
        window->handle());
    if (platformWindow) {
        platformWindow->setCustomMargins(margins);
    }
#endif
}

bool Utilities::isWin7OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows7;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS7;
#endif
}

bool Utilities::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8;
#endif
}

bool Utilities::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1;
#endif
}

bool Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

bool Utilities::isWin10OrGreater(const int subVer)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, subVer);
#else
    Q_UNUSED(ver);
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

static inline bool forceEnableDwmBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_forceEnableTraditionalBlur_flag);
}

static inline bool forceDisableWallpaperBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_forceDisableWallpaperBlur_flag);
}

static inline bool forceEnableOfficialMSWin10AcrylicBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_forceEnableOfficialMSWin10AcrylicBlur_flag);
}

static inline bool shouldUseOfficialMSWin10AcrylicBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    const QOperatingSystemVersion currentVersion = QOperatingSystemVersion::current();
    if (currentVersion > QOperatingSystemVersion::Windows10) {
        return true;
    }
    return ((currentVersion.microVersion() >= 16190) && (currentVersion.microVersion() < 18362));
#else
    // TODO
    return false;
#endif
}

bool Utilities::isOfficialMSWin10AcrylicBlurAvailable()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    if (!forceEnableDwmBlur() && !forceDisableWallpaperBlur()) {
        // We can't enable the official Acrylic blur in wallpaper blur mode.
        return false;
    }
    if (forceEnableOfficialMSWin10AcrylicBlur()) {
        return true;
    }
    return shouldUseOfficialMSWin10AcrylicBlur();
}

static inline bool shouldUseOriginalDwmBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return Utilities::isWin10OrGreater() || (QOperatingSystemVersion::current() >= QOperatingSystemVersion::OSXYosemite);
#else
    // TODO
    return false;
#endif
}

bool Utilities::shouldUseTraditionalBlur()
{
    if ((forceEnableDwmBlur() || forceDisableWallpaperBlur()) && shouldUseOriginalDwmBlur()) {
        return true;
    }
    return false;
}
