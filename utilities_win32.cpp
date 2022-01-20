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

#include "utilities.h"
#include <QtCore/qdebug.h>
#include <QtCore/private/qsystemlibrary_p.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif
#include <QtGui/qguiapplication.h>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/qpa/qplatformnativeinterface.h>
#else
#include <QtGui/qpa/qplatformwindow_p.h>
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

[[nodiscard]] static inline bool isWin10RS5OrGreater()
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

[[nodiscard]] static inline bool isWin1019H1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10_1903);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 18362));
#else
    static const bool result = isWindowsVersionOrGreater(10, 0, 18362);
#endif
    return result;
}

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

bool Utilities::isDwmCompositionAvailable()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    const auto resultFromRegistry = []() -> bool {
        QWinRegistryKey registry(HKEY_CURRENT_USER, QString::fromUtf8(kDwmRegistryKey));
        const auto result = registry.dwordValue(QStringLiteral("Composition"));
        return (result.second && (result.first != 0));
    };
    static const auto pDwmIsCompositionEnabled =
        reinterpret_cast<HRESULT(WINAPI *)(BOOL *)>(QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmIsCompositionEnabled"));
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

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    const qreal devicePixelRatio = window->devicePixelRatio();
    const qreal scaleFactor = (dpiScale ? devicePixelRatio : 1.0);
    switch (metric) {
    case SystemMetric::ResizeBorderThickness: {
        const int resizeBorderThickness = window->property(Constants::kResizeBorderThicknessFlag).toInt();
        if ((resizeBorderThickness > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(resizeBorderThickness) * scaleFactor);
        } else {
            const int result = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / devicePixelRatio);
                }
            } else {
                qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMetrics"));
                // The padded border will disappear if DWM composition is disabled.
                const int defaultResizeBorderThickness = (isDwmCompositionAvailable() ? kDefaultResizeBorderThicknessAero : kDefaultResizeBorderThicknessClassic);
                if (dpiScale) {
                    return qRound(static_cast<qreal>(defaultResizeBorderThickness) * devicePixelRatio);
                } else {
                    return defaultResizeBorderThickness;
                }
            }
        }
    }
    case SystemMetric::CaptionHeight: {
        const int captionHeight = window->property(Constants::kCaptionHeightFlag).toInt();
        if ((captionHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(captionHeight) * scaleFactor);
        } else {
            const int result = GetSystemMetrics(SM_CYCAPTION);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / devicePixelRatio);
                }
            } else {
                qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMetrics"));
                if (dpiScale) {
                    return qRound(static_cast<qreal>(kDefaultCaptionHeight) * devicePixelRatio);
                } else {
                    return kDefaultCaptionHeight;
                }
            }
        }
    }
    case SystemMetric::TitleBarHeight: {
        const int titleBarHeight = window->property(Constants::kTitleBarHeightFlag).toInt();
        if ((titleBarHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(titleBarHeight) * scaleFactor);
        } else {
            const int captionHeight = getSystemMetric(window,SystemMetric::CaptionHeight,
                                                      dpiScale, forceSystemValue);
            const int resizeBorderThickness = getSystemMetric(window, SystemMetric::ResizeBorderThickness,
                                                              dpiScale, forceSystemValue);
            return (((window->windowState() == Qt::WindowMaximized)
                     || (window->windowState() == Qt::WindowFullScreen))
                    ? captionHeight : (captionHeight + resizeBorderThickness));
        }
    }
    }
    return 0;
}

void Utilities::triggerFrameChange(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetWindowPos"));
    }
}

void Utilities::updateFrameMargins(const WId winId, const bool reset)
{
    // DwmExtendFrameIntoClientArea() will always fail if DWM composition is disabled.
    // No need to try in this case.
    if (!isDwmCompositionAvailable()) {
        return;
    }
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    static const auto pDwmExtendFrameIntoClientArea =
        reinterpret_cast<HRESULT(WINAPI *)(HWND, const MARGINS *)>(QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmExtendFrameIntoClientArea"));
    if (!pDwmExtendFrameIntoClientArea) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    const MARGINS margins = reset ? MARGINS{0, 0, 0, 0} : MARGINS{1, 1, 1, 1};
    const HRESULT hr = pDwmExtendFrameIntoClientArea(hwnd, &margins);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(QStringLiteral("DwmExtendFrameIntoClientArea"), hr);
    }
}

void Utilities::updateQtFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const bool useCustomFrameMargin = (enable && (window->windowState() != Qt::WindowMaximized)
                                         && (window->windowState() != Qt::WindowFullScreen));
    const int resizeBorderThickness = useCustomFrameMargin ?
                                      Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, true, true) : 0;
    const int titleBarHeight = enable ? Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, true, true) : 0;
    const QMargins margins = {-resizeBorderThickness, -titleBarHeight, -resizeBorderThickness, -resizeBorderThickness}; // left, top, right, bottom
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty("_q_windowsCustomMargins", marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QStringLiteral("WindowsCustomMargins"), marginsVar);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
    }
#else
    auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
        window->handle());
    if (platformWindow) {
        platformWindow->setCustomMargins(margins);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
    }
#endif
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

QColor Utilities::getColorizationColor()
{
    const auto resultFromRegistry = []() -> QColor {
        QWinRegistryKey registry(HKEY_CURRENT_USER, QString::fromUtf8(kDwmRegistryKey));
        const auto result = registry.dwordValue(QStringLiteral("ColorizationColor"));
        return (result.second ? QColor::fromRgba(result.first) : Qt::darkGray);
    };
    static const auto pDwmGetColorizationColor =
        reinterpret_cast<HRESULT(WINAPI *)(DWORD *, BOOL *)>(QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetColorizationColor"));
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

int Utilities::getWindowVisibleFrameBorderThickness(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 1;
    }
    if (!isWin10OrGreater()) {
        return 1;
    }
    static const auto pDwmGetWindowAttribute =
        reinterpret_cast<HRESULT(WINAPI *)(HWND, DWORD, PVOID, DWORD)>(QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetWindowAttribute"));
    if (!pDwmGetWindowAttribute) {
        return 1;
    }
    const auto hWnd = reinterpret_cast<HWND>(winId);
    UINT value = 0;
    const HRESULT hr = pDwmGetWindowAttribute(hWnd, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        const QWindow *w = findWindow(winId);
        return static_cast<int>(qRound(static_cast<qreal>(value) / (w ? w->devicePixelRatio() : 1.0)));
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    return 1;
}

bool Utilities::shouldAppsUseDarkMode()
{
    // The dark mode was introduced in Windows 10 1809.
    if (!isWin10RS5OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        QWinRegistryKey registry(HKEY_CURRENT_USER, QString::fromUtf8(kPersonalizeRegistryKey));
        const auto result = registry.dwordValue(QStringLiteral("AppsUseLightTheme"));
        return (result.second && (result.first == 0));
    };
    // Starting from Windows 10 1903, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (isWin1019H1OrGreater()) {
        return resultFromRegistry();
    } else {
        static const auto pShouldAppsUseDarkMode =
            reinterpret_cast<BOOL(WINAPI *)(VOID)>(QSystemLibrary::resolve(QStringLiteral("uxtheme"), MAKEINTRESOURCEA(132)));
        return (pShouldAppsUseDarkMode ? (pShouldAppsUseDarkMode() != FALSE) : resultFromRegistry());
    }
}

ColorizationArea Utilities::getColorizationArea()
{
    if (!isWin10OrGreater()) {
        return ColorizationArea::None;
    }
    const QString keyName = QStringLiteral("ColorPrevalence");
    QWinRegistryKey themeRegistry(HKEY_CURRENT_USER, QString::fromUtf8(kPersonalizeRegistryKey));
    const auto themeValue = themeRegistry.dwordValue(keyName);
    QWinRegistryKey dwmRegistry(HKEY_CURRENT_USER, QString::fromUtf8(kDwmRegistryKey));
    const auto dwmValue = dwmRegistry.dwordValue(keyName);
    const bool theme = themeValue.second && (themeValue.first != 0);
    const bool dwm = dwmValue.second && (dwmValue.first != 0);
    if (theme && dwm) {
        return ColorizationArea::All;
    } else if (theme) {
        return ColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return ColorizationArea::TitleBar_WindowBorder;
    }
    return ColorizationArea::None;
}

bool Utilities::isThemeChanged(const void *data)
{
    Q_ASSERT(data);
    if (!data) {
        return false;
    }
    const auto msg = static_cast<const MSG *>(data);
    if (msg->message == WM_THEMECHANGED) {
        return true;
    } else if (msg->message == WM_DWMCOLORIZATIONCOLORCHANGED) {
        return true;
    } else if (msg->message == WM_SETTINGCHANGE) {
        if ((msg->wParam == 0) && (_wcsicmp(reinterpret_cast<LPCWSTR>(msg->lParam), L"ImmersiveColorSet") == 0)) {
            return true;
        }
    }
    return false;
}

bool Utilities::isSystemMenuRequested(const void *data, QPointF *pos)
{
    Q_ASSERT(data);
    if (!data) {
        return false;
    }
    bool result = false;
    const auto msg = static_cast<const MSG *>(data);
    if (msg->message == WM_NCRBUTTONUP) {
        if (msg->wParam == HTCAPTION) {
            result = true;
        }
    } else if (msg->message == WM_SYSCOMMAND) {
        const WPARAM filteredWParam = (msg->wParam & 0xFFF0);
        if ((filteredWParam == SC_KEYMENU) && (msg->lParam == VK_SPACE)) {
            result = true;
        }
    } else if (msg->message == WM_CONTEXTMENU) {
        //
    }
    if (result) {
        if (pos) {
            *pos = [msg](){
                const POINT nativePos = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
                return QPointF(static_cast<qreal>(nativePos.x), static_cast<qreal>(nativePos.y));
            }();
        }
    }
    return result;
}

bool Utilities::showSystemMenu(const WId winId, const QPointF &pos)
{
    Q_ASSERT(winId);
    if (!winId) {
        return false;
    }
    const auto hWnd = reinterpret_cast<HWND>(winId);
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMenu"));
        return false;
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
        return false;
    }
    if (!setState(SC_MOVE, !max)) {
        return false;
    }
    if (!setState(SC_SIZE, !max)) {
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        return false;
    }
    if (!setState(SC_MAXIMIZE, !max)) {
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("SetMenuDefaultItem"));
        return false;
    }
    const QPoint roundedPos = pos.toPoint();
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, roundedPos.x(), roundedPos.y(), 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            qWarning() << getSystemErrorMessage(QStringLiteral("PostMessageW"));
            return false;
        }
    }
    return true;
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
    // According to Microsoft Docs, we should compare to primary
    // screen's geometry.
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
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hwnd, &wp) == FALSE) {
        qWarning() << getSystemErrorMessage(QStringLiteral("GetWindowPlacement"));
        return false;
    }
    return ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE));
}

FRAMELESSHELPER_END_NAMESPACE
