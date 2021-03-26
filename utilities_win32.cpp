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
#include <QtGui/qguiapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <dwmapi.h>
#include <shobjidl_core.h>
#include <wininet.h>
#include <shlobj_core.h>
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
#include <QtCore/qscopeguard.h>
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
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6
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
static const QString g_desktopRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Control Panel\Desktop)");

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
    return isWin7OrGreater() && (enabled != FALSE);
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
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return false;
    }
    bool result = false;
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
        result = (win32Data()->SetWindowCompositionAttributePFN(hwnd, &wcaData) != FALSE);
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
    if (result) {
        const auto win = const_cast<QWindow *>(window);
        win->setProperty(_flh_global::_flh_acrylic_blurEnabled_flag, enabled);
        win->setProperty(_flh_global::_flh_acrylic_gradientColor_flag, gradientColor);
    }
    return result;
}

bool Utilities::isColorizationEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    // TODO: Is there an official Win32 API to do this?
    bool ok = false;
    const QSettings registry(g_dwmRegistryKey, QSettings::NativeFormat);
    const bool colorPrevalence = registry.value(QStringLiteral("ColorPrevalence"), 0).toULongLong(&ok) != 0;
    return (ok && colorPrevalence);
}

QColor Utilities::getColorizationColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&color, &opaqueBlend))) {
        return QColor::fromRgba(color);
    }
    qWarning() << "DwmGetColorizationColor failed, reading from the registry instead.";
    bool ok = false;
    const QSettings settings(g_dwmRegistryKey, QSettings::NativeFormat);
    const DWORD value = settings.value(QStringLiteral("ColorizationColor"), 0).toULongLong(&ok);
    return ok ? QColor::fromRgba(value) : Qt::darkGray;
}

bool Utilities::isLightThemeEnabled()
{
    return !isDarkThemeEnabled();
}

bool Utilities::isDarkThemeEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    // We can't use ShouldAppsUseDarkMode due to the following reason:
    // it's not exported publicly so we can only load it dynamically through its ordinal name,
    // however, its ordinal name has changed in some unknown system versions so we can't find
    // the actual function now. But ShouldSystemUseDarkMode is not affected, we can still
    // use it in the latest version of Windows.
    if (win32Data()->ShouldSystemUseDarkModePFN) {
        return win32Data()->ShouldSystemUseDarkModePFN();
    }
    qDebug() << "ShouldSystemUseDarkMode() not available, reading from the registry instead.";
    bool ok = false;
    const QSettings settings(g_personalizeRegistryKey, QSettings::NativeFormat);
    const bool lightThemeEnabled = settings.value(QStringLiteral("AppsUseLightTheme"), 0).toULongLong(&ok) != 0;
    return (ok && !lightThemeEnabled);
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
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return false;
    }
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
    // TODO: Is there an official Win32 API to do this?
    bool ok = false;
    const QSettings registry(g_personalizeRegistryKey, QSettings::NativeFormat);
    const bool transparencyEnabled = registry.value(QStringLiteral("EnableTransparency"), 0).toULongLong(&ok) != 0;
    return (ok && transparencyEnabled);
}

void Utilities::triggerFrameChange(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
        qWarning() << "SetWindowPos failed.";
    }
}

void Utilities::updateFrameMargins(const QWindow *window, const bool reset)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    const MARGINS margins = reset ? MARGINS{0, 0, 0, 0} : MARGINS{1, 1, 1, 1};
    if (FAILED(DwmExtendFrameIntoClientArea(hwnd, &margins))) {
        qWarning() << "DwmExtendFrameIntoClientArea failed.";
    }
}

QImage Utilities::getDesktopWallpaperImage(const int screen)
{
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper* pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                UINT monitorCount = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount))) {
                    if (screen > int(monitorCount - 1)) {
                        qWarning() << "Screen number above total screen count.";
                        return {};
                    }
                    const UINT monitorIndex = qMax(screen, 0);
                    LPWSTR monitorId = nullptr;
                    if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathAt(monitorIndex, &monitorId)) && monitorId) {
                        LPWSTR wallpaperPath = nullptr;
                        if (SUCCEEDED(pDesktopWallpaper->GetWallpaper(monitorId, &wallpaperPath)) && wallpaperPath) {
                            return QImage(QString::fromWCharArray(wallpaperPath));
                        } else {
                            qWarning() << "IDesktopWallpaper::GetWallpaper() failed.";
                        }
                    } else {
                        qWarning() << "IDesktopWallpaper::GetMonitorDevicePathAt() failed";
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetMonitorDevicePathCount() failed";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            WCHAR wallpaperPath[MAX_PATH] = {};
            // TODO: AD_GETWP_BMP, AD_GETWP_IMAGE, AD_GETWP_LAST_APPLIED. What's the difference?
            if (SUCCEEDED(pActiveDesktop->GetWallpaper(wallpaperPath, MAX_PATH, AD_GETWP_LAST_APPLIED))) {
                return QImage(QString::fromWCharArray(wallpaperPath));
            } else {
                qWarning() << "IActiveDesktop::GetWallpaper() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Using SystemParametersInfoW instead.";
    LPWSTR wallpaperPath = nullptr;
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0) != FALSE) {
        return QImage(QString::fromWCharArray(wallpaperPath));
    }
    qWarning() << "SystemParametersInfoW failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    const QString path = settings.value(QStringLiteral("WallPaper")).toString();
    if (QFileInfo::exists(path)) {
        return QImage(path);
    }
    qWarning() << "Failed to read the registry.";
    return {};
}

QColor Utilities::getDesktopBackgroundColor(const int screen)
{
    Q_UNUSED(screen);
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper]() {
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                COLORREF color = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetBackgroundColor(&color))) {
                    return QColor::fromRgba(color);
                } else {
                    qWarning() << "IDesktopWallpaper::GetBackgroundColor() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed.";
    }
    // TODO: Is there any other way to get the background color? Traditional Win32 API? Registry?
    // Is there a Shell API for Win7?
    return Qt::black;
}

Utilities::DesktopWallpaperAspectStyle Utilities::getDesktopWallpaperAspectStyle(const int screen)
{
    Q_UNUSED(screen);
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
                if (SUCCEEDED(pDesktopWallpaper->GetPosition(&position))) {
                    switch (position) {
                    case DWPOS_CENTER:
                        return DesktopWallpaperAspectStyle::Central;
                    case DWPOS_TILE:
                        return DesktopWallpaperAspectStyle::Tiled;
                    case DWPOS_STRETCH:
                        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                    case DWPOS_FIT:
                        return DesktopWallpaperAspectStyle::KeepRatioFit;
                    case DWPOS_FILL:
                        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                    case DWPOS_SPAN:
                        return DesktopWallpaperAspectStyle::Span;
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetPosition() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            WALLPAPEROPT opt;
            SecureZeroMemory(&opt, sizeof(opt));
            opt.dwSize = sizeof(opt);
            if (SUCCEEDED(pActiveDesktop->GetWallpaperOptions(&opt, 0))) {
                switch (opt.dwStyle) {
                case WPSTYLE_CENTER:
                    return DesktopWallpaperAspectStyle::Central;
                case WPSTYLE_TILE:
                    return DesktopWallpaperAspectStyle::Tiled;
                case WPSTYLE_STRETCH:
                    return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                case WPSTYLE_KEEPASPECT:
                    return DesktopWallpaperAspectStyle::KeepRatioFit;
                case WPSTYLE_CROPTOFIT:
                    return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                case WPSTYLE_SPAN:
                    return DesktopWallpaperAspectStyle::Span;
                }
            } else {
                qWarning() << "IActiveDesktop::GetWallpaperOptions() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    bool ok = false;
    const DWORD style = settings.value(QStringLiteral("WallpaperStyle"), 0).toULongLong(&ok);
    if (!ok) {
        qWarning() << "Failed to read the registry.";
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
    }
    switch (style) {
    case 0: {
        bool ok = false;
        if ((settings.value(QStringLiteral("TileWallpaper"), 0).toULongLong(&ok) != 0) && ok) {
            return DesktopWallpaperAspectStyle::Tiled;
        } else {
            return DesktopWallpaperAspectStyle::Central;
        }
    }
    case 2:
        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
    case 6:
        return DesktopWallpaperAspectStyle::KeepRatioFit;
    case 10:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
    case 22:
        return DesktopWallpaperAspectStyle::Span;
    default:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
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
    // We can use Direct2D to get DPI since Win7 or Vista, but it's marked as deprecated by Microsoft
    // in the latest SDK and we'll get compilation warnings because of that, so we just don't use it here.
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
    const int bw = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderWidth, true, true) : 0;
    const int bh = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderHeight, true, true) : 0;
    const QMargins margins = {-bw, -tbh, -bw, -bh}; // left, top, right, bottom
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
    if (!forceEnableTraditionalBlur() && !forceDisableWallpaperBlur() && !disableExtraProcessingForBlur()) {
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
    if ((forceEnableTraditionalBlur() || forceDisableWallpaperBlur() || disableExtraProcessingForBlur()) && shouldUseOriginalDwmBlur()) {
        return true;
    }
    return false;
}

void Utilities::displaySystemMenu(const QWindow *window, const QPoint &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    const HMENU hMenu = GetSystemMenu(hwnd, FALSE);
    if (!hMenu) {
        qWarning() << "Failed to acquire the system menu.";
        return;
    }
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = 0;
    mii.fState = MF_ENABLED;
    SetMenuItemInfoW(hMenu, SC_RESTORE, FALSE, &mii);
    SetMenuItemInfoW(hMenu, SC_SIZE, FALSE, &mii);
    SetMenuItemInfoW(hMenu, SC_MOVE, FALSE, &mii);
    SetMenuItemInfoW(hMenu, SC_MAXIMIZE, FALSE, &mii);
    SetMenuItemInfoW(hMenu, SC_MINIMIZE, FALSE, &mii);
    mii.fState = MF_GRAYED;
    const bool isMin = [window]{
        return (window->windowState() == Qt::WindowMinimized);
    }();
    const bool isMax = [window]{
        return (window->windowState() == Qt::WindowMaximized);
    }();
    const bool isFull = [window]{
        return (window->windowState() == Qt::WindowFullScreen);
    }();
    const bool isNormal = [window]{
        return (window->windowState() == Qt::WindowNoState);
    }();
    const bool isFix = isWindowFixedSize(window);
    if (isFix || isMax || isFull) {
        SetMenuItemInfoW(hMenu, SC_SIZE, FALSE, &mii);
        SetMenuItemInfoW(hMenu, SC_MAXIMIZE, FALSE, &mii);
    }
    if (isFix || isFull || isNormal) {
        SetMenuItemInfoW(hMenu, SC_RESTORE, FALSE, &mii);
    }
    if (isMax || isFull) {
        SetMenuItemInfoW(hMenu, SC_MOVE, FALSE, &mii);
    }
    if (isMin) {
        SetMenuItemInfoW(hMenu, SC_MINIMIZE, FALSE, &mii);
    }
    const bool isRtl = QGuiApplication::isRightToLeft();
    const QPoint point = pos.isNull() ? QCursor::pos(window->screen()) : window->mapToGlobal(pos);
    const LPARAM cmd = TrackPopupMenu(hMenu,
            (TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_TOPALIGN |
            (isRtl ? TPM_RIGHTALIGN : TPM_LEFTALIGN)),
            point.x(), point.y(), 0, hwnd, nullptr);
    if (cmd) {
        PostMessageW(hwnd, WM_SYSCOMMAND, cmd, 0);
    }
}
