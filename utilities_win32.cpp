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
#include "framelesshelper_windows.h"

Q_DECLARE_METATYPE(QMargins)

FRAMELESSHELPER_BEGIN_NAMESPACE

[[nodiscard]] static inline QPointF extractMousePositionFromLParam(const LPARAM lParam)
{
    const POINT nativePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    return QPointF(static_cast<qreal>(nativePos.x), static_cast<qreal>(nativePos.y));
}

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

QString Utilities::getSystemErrorMessage(const QString &function)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    const HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (SUCCEEDED(hr)) {
        return QStringLiteral("Operation succeeded.");
    }
    return getSystemErrorMessage(function, hr);
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

int Utilities::getWindowVisibleFrameBorderThickness(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return 1;
    }
    if (!isWin10OrGreater()) {
        return 1;
    }
    const auto hWnd = reinterpret_cast<HWND>(winId);
    UINT value = 0;
    const HRESULT hr = DwmGetWindowAttribute(hWnd, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        return value;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    return 1;
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
                    qWarning() << getSystemErrorMessage(QStringLiteral("LoadLibraryExW"));
                    return resultFromRegistry();
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(132)));
                if (!func) {
                    qWarning() << getSystemErrorMessage(QStringLiteral("GetProcAddress"));
                    return resultFromRegistry();
                }
            }
        }
        return (func() != FALSE);
    }
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
            *pos = extractMousePositionFromLParam(msg->lParam);
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

FRAMELESSHELPER_END_NAMESPACE
