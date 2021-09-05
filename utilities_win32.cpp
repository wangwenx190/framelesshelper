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
#include <QtCore/qsettings.h>
#include <QtCore/qt_windows.h>
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
#include <dwmapi.h>

Q_DECLARE_METATYPE(QMargins)

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER (92)
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

static constexpr char kDwmRegistryKey[] = R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)";
static constexpr char kPersonalizeRegistryKey[] = R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr int kDefaultResizeBorderThicknessClassic = 4;
static constexpr int kDefaultResizeBorderThicknessAero = 8;
static constexpr int kDefaultCaptionHeight = 23;

enum : WORD
{
    _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19,
    _DWMWA_USE_IMMERSIVE_DARK_MODE = 20,
    _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37
};

[[nodiscard]] static inline bool isWin10RS1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 14393));
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}

[[nodiscard]] static inline bool isWin1019H1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 18362));
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
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

bool Utilities::isDwmCompositionAvailable()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    const HRESULT hr = DwmIsCompositionEnabled(&enabled);
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        qWarning() << getSystemErrorMessage(QStringLiteral("DwmIsCompositionEnabled"), hr);
        const QSettings registry(QString::fromUtf8(kDwmRegistryKey), QSettings::NativeFormat);
        bool ok = false;
        const DWORD value = registry.value(QStringLiteral("Composition"), 0).toUInt(&ok);
        return (ok && (value != 0));
    }
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
                const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMetrics"), hr);
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
                const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                qWarning() << getSystemErrorMessage(QStringLiteral("GetSystemMetrics"), hr);
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
        const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        qWarning() << getSystemErrorMessage(QStringLiteral("SetWindowPos"), hr);
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
    const auto hwnd = reinterpret_cast<HWND>(winId);
    const MARGINS margins = reset ? MARGINS{0, 0, 0, 0} : MARGINS{1, 1, 1, 1};
    const HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
    if (FAILED(hr)) {
        qWarning() << getSystemErrorMessage(QStringLiteral("DwmExtendFrameIntoClientArea"), hr);
    }
}

void Utilities::updateQtFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const bool shouldApplyCustomFrameMargins = (enable
                                                && (window->windowState() != Qt::WindowMaximized)
                                                && (window->windowState() != Qt::WindowFullScreen));
    const int resizeBorderThickness = shouldApplyCustomFrameMargins ? Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, true, true) : 0;
    const int titleBarHeight = shouldApplyCustomFrameMargins ? Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, true, true) : 0;
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

QString Utilities::getSystemErrorMessage(const QString &function, const HRESULT hr)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    if (SUCCEEDED(hr)) {
        return QStringLiteral("Operation succeeded.");
    }
    const DWORD dwError = HRESULT_CODE(hr);
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, nullptr) == 0) {
        return QStringLiteral("Failed to retrieve the error message from system.");
    }
    const QString message = QStringLiteral("%1 failed with error %2: %3.")
                             .arg(function, QString::number(dwError), QString::fromWCharArray(buf));
    LocalFree(buf);
    return message;
}

QColor Utilities::getColorizationColor()
{
    COLORREF color = RGB(0, 0, 0);
    BOOL opaque = FALSE;
    const HRESULT hr = DwmGetColorizationColor(&color, &opaque);
    if (FAILED(hr)) {
        qWarning() << getSystemErrorMessage(QStringLiteral("DwmGetColorizationColor"), hr);
        const QSettings registry(QString::fromUtf8(kDwmRegistryKey), QSettings::NativeFormat);
        bool ok = false;
        color = registry.value(QStringLiteral("ColorizationColor"), 0).toUInt(&ok);
        if (!ok || (color == 0)) {
            color = RGB(128, 128, 128); // Dark gray
        }
    }
    return QColor::fromRgba(color);
}

int Utilities::getWindowVisibleFrameBorderThickness(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    if (!isWin10OrGreater()) {
        return 0;
    }
    const auto hWnd = reinterpret_cast<HWND>(window->winId());
    UINT value = 0;
    const HRESULT hr = DwmGetWindowAttribute(hWnd, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        return value;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    const bool max = (window->windowState() == Qt::WindowMaximized);
    const bool full = (window->windowState() == Qt::WindowFullScreen);
    return ((max || full) ? 0 : qRound(1.0 * window->devicePixelRatio()));
}

bool Utilities::shouldAppsUseDarkMode()
{
    if (!isWin10RS1OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        const QSettings registry(QString::fromUtf8(kPersonalizeRegistryKey), QSettings::NativeFormat);
        bool ok = false;
        const DWORD value = registry.value(QStringLiteral("AppsUseLightTheme"), 0).toUInt(&ok);
        return (ok && (value == 0));
    };
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (isWin1019H1OrGreater()) {
        return resultFromRegistry();
    } else {
        static bool tried = false;
        using sig = BOOL(WINAPI *)();
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return resultFromRegistry();
            } else {
                tried = true;
                const HMODULE dll = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!dll) {
                    const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                    qWarning() << getSystemErrorMessage(QStringLiteral("LoadLibraryExW"), hr);
                    return resultFromRegistry();
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(132)));
                if (!func) {
                    const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                    qWarning() << getSystemErrorMessage(QStringLiteral("GetProcAddress"), hr);
                    return resultFromRegistry();
                }
            }
        }
        return (func() != FALSE);
    }
}

bool Utilities::isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        qWarning() << getSystemErrorMessage(QStringLiteral("SystemParametersInfoW"), hr);
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

ColorizationArea Utilities::getColorizationArea()
{
    if (!isWin10OrGreater()) {
        return ColorizationArea::None;
    }
    const QString keyName = QStringLiteral("ColorPrevalence");
    const QSettings themeRegistry(QString::fromUtf8(kPersonalizeRegistryKey), QSettings::NativeFormat);
    const DWORD themeValue = themeRegistry.value(keyName, 0).toUInt();
    const QSettings dwmRegistry(QString::fromUtf8(kDwmRegistryKey), QSettings::NativeFormat);
    const DWORD dwmValue = dwmRegistry.value(keyName, 0).toUInt();
    const bool theme = (themeValue != 0);
    const bool dwm = (dwmValue != 0);
    if (theme && dwm) {
        return ColorizationArea::All;
    } else if (theme) {
        return ColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return ColorizationArea::TitleBar_WindowBorder;
    }
    return ColorizationArea::None;
}

bool Utilities::isWindowDarkFrameBorderEnabled(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return false;
    }
    if (!isWin10RS1OrGreater()) {
        return false;
    }
    const auto hWnd = reinterpret_cast<HWND>(winId);
    BOOL enabled = FALSE;
    HRESULT hr = DwmGetWindowAttribute(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    hr = DwmGetWindowAttribute(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
