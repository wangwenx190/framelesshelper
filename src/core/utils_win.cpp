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

#include "utils.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qhash.h>
#include <QtCore/qsettings.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/private/qsystemlibrary_p.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#  include <QtCore/qoperatingsystemversion.h>
#else
#  include <QtCore/qsysinfo.h>
#endif
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qpa/qplatformnativeinterface.h>
#else
#  include <QtGui/qpa/qplatformwindow_p.h>
#endif
#include "qwinregistry_p.h"
#include "framelesswindowsmanager.h"
#include "framelesshelper_windows.h"
#include <atlbase.h>
#include <d2d1.h>

Q_DECLARE_METATYPE(QMargins)

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct Win32UtilsHelperData
{
    WNDPROC originalWindowProc = nullptr;
    Options options = {};
    QPoint offset = {};
    IsWindowFixedSizeCallback isWindowFixedSize = nullptr;
};

struct Win32UtilsHelper
{
    QMutex mutex = {};
    QHash<WId, Win32UtilsHelperData> data = {};
};

Q_GLOBAL_STATIC(Win32UtilsHelper, g_utilsHelper)

static const QString qDwmRegistryKey = QString::fromWCharArray(kDwmRegistryKey);
static const QString qPersonalizeRegistryKey = QString::fromWCharArray(kPersonalizeRegistryKey);
static const QString qDwmColorKeyName = QString::fromWCharArray(kDwmColorKeyName);
FRAMELESSHELPER_STRING_CONSTANT2(SuccessMessageText, "The operation completed successfully.")
FRAMELESSHELPER_STRING_CONSTANT2(FormatMessageEmptyResult, "\"FormatMessageW()\" returned empty string.")
FRAMELESSHELPER_STRING_CONSTANT2(ErrorMessageTemplate, "Function \"%1()\" failed with error code %2: %3.")
FRAMELESSHELPER_STRING_CONSTANT(Composition)
FRAMELESSHELPER_STRING_CONSTANT(ColorizationColor)
FRAMELESSHELPER_STRING_CONSTANT(AppsUseLightTheme)
FRAMELESSHELPER_STRING_CONSTANT(WindowsCustomMargins)
FRAMELESSHELPER_STRING_CONSTANT(user32)
FRAMELESSHELPER_STRING_CONSTANT(dwmapi)
FRAMELESSHELPER_STRING_CONSTANT(winmm)
FRAMELESSHELPER_STRING_CONSTANT(shcore)
FRAMELESSHELPER_STRING_CONSTANT(d2d1)
FRAMELESSHELPER_STRING_CONSTANT(uxtheme)
FRAMELESSHELPER_STRING_CONSTANT(GetWindowRect)
FRAMELESSHELPER_STRING_CONSTANT(DwmIsCompositionEnabled)
FRAMELESSHELPER_STRING_CONSTANT(SetWindowPos)
FRAMELESSHELPER_STRING_CONSTANT(DwmExtendFrameIntoClientArea)
FRAMELESSHELPER_STRING_CONSTANT(DwmGetColorizationColor)
FRAMELESSHELPER_STRING_CONSTANT(PostMessageW)
FRAMELESSHELPER_STRING_CONSTANT(MonitorFromWindow)
FRAMELESSHELPER_STRING_CONSTANT(GetMonitorInfoW)
FRAMELESSHELPER_STRING_CONSTANT(GetWindowPlacement)
FRAMELESSHELPER_STRING_CONSTANT(QueryPerformanceFrequency)
FRAMELESSHELPER_STRING_CONSTANT(QueryPerformanceCounter)
FRAMELESSHELPER_STRING_CONSTANT(DwmGetCompositionTimingInfo)
FRAMELESSHELPER_STRING_CONSTANT(SystemParametersInfoW)
#ifdef Q_PROCESSOR_X86_64
  FRAMELESSHELPER_STRING_CONSTANT(GetClassLongPtrW)
  FRAMELESSHELPER_STRING_CONSTANT(SetClassLongPtrW)
  FRAMELESSHELPER_STRING_CONSTANT(GetWindowLongPtrW)
  FRAMELESSHELPER_STRING_CONSTANT(SetWindowLongPtrW)
#else
  // WinUser.h defines G/SetClassLongPtr as G/SetClassLong due to the
  // "Ptr" suffixed APIs are not available on 32-bit platforms, so we
  // have to add the following workaround. Undefine the macros and then
  // redefine them is also an option but the following solution is more simple.
  FRAMELESSHELPER_STRING_CONSTANT2(GetClassLongPtrW, "GetClassLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetClassLongPtrW, "SetClassLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(GetWindowLongPtrW, "GetWindowLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetWindowLongPtrW, "SetWindowLongW")
#endif
FRAMELESSHELPER_STRING_CONSTANT(ReleaseCapture)

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
        return kSuccessMessageText;
    }
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buf), 0, nullptr) == 0) {
        return kFormatMessageEmptyResult;
    }
    const QString errorText = QString::fromWCharArray(buf).trimmed();
    LocalFree(buf);
    buf = nullptr;
    return kErrorMessageTemplate.arg(function, QString::number(code), errorText);
}

[[nodiscard]] static inline QString __getSystemErrorMessage(const QString &function, const HRESULT hr)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return {};
    }
    if (SUCCEEDED(hr)) {
        return kSuccessMessageText;
    }
    const DWORD dwError = HRESULT_CODE(hr);
    return __getSystemErrorMessage(function, dwError);
}

[[nodiscard]] static inline int getSystemMetrics2(const WId windowId, const int index,
                                                  const bool horizontal, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return 0;
    }
    const UINT windowDpi = Utils::getWindowDpi(windowId, horizontal);
    static const auto pGetSystemMetricsForDpi =
        reinterpret_cast<decltype(&GetSystemMetricsForDpi)>(
            QSystemLibrary::resolve(kuser32, "GetSystemMetricsForDpi"));
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

[[nodiscard]] bool shouldAppsUseDarkMode_windows()
{
    // The global dark mode was first introduced in Windows 10 1607.
    if (!Utils::isWin101607OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, qPersonalizeRegistryKey);
        const auto result = registry.dwordValue(kAppsUseLightTheme);
        return (result.second && (result.first == 0));
    };
    // Starting from Windows 10 1903, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    return resultFromRegistry();
}

[[nodiscard]] static inline LRESULT CALLBACK SystemMenuHookWindowProc
    (const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    Q_ASSERT(hWnd);
    if (!hWnd) {
        return 0;
    }
    const auto windowId = reinterpret_cast<WId>(hWnd);
    g_utilsHelper()->mutex.lock();
    if (!g_utilsHelper()->data.contains(windowId)) {
        g_utilsHelper()->mutex.unlock();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    const Win32UtilsHelperData data = g_utilsHelper()->data.value(windowId);
    g_utilsHelper()->mutex.unlock();
    const auto getGlobalPosFromMouse = [lParam]() -> QPoint {
        return {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    };
    const auto getGlobalPosFromKeyboard = [hWnd, windowId, &data]() -> QPoint {
        RECT windowPos = {};
        if (GetWindowRect(hWnd, &windowPos) == FALSE) {
            qWarning() << Utils::getSystemErrorMessage(kGetWindowRect);
            return {};
        }
        const bool maxOrFull = (IsMaximized(hWnd) ||
               ((data.options & Option::DontTreatFullScreenAsZoomed) ? false : Utils::isFullScreen(windowId)));
        const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
        const bool frameBorderVisible = Utils::isWindowFrameBorderVisible();
        const int horizontalOffset = ((maxOrFull || !frameBorderVisible) ? 0 : frameSizeX);
        const int verticalOffset = [windowId, frameBorderVisible, maxOrFull]() -> int {
            const int titleBarHeight = Utils::getTitleBarHeight(windowId, true);
            if (!frameBorderVisible) {
                return titleBarHeight;
            }
            const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
            if (Utils::isWin11OrGreater()) {
                if (maxOrFull) {
                    return (titleBarHeight + frameSizeY);
                }
                return titleBarHeight;
            }
            if (maxOrFull) {
                return titleBarHeight;
            }
            return (titleBarHeight - frameSizeY);
        }();
        return {windowPos.left + horizontalOffset, windowPos.top + verticalOffset};
    };
    bool shouldShowSystemMenu = false;
    bool broughtByKeyboard = false;
    QPoint globalPos = {};
    if (uMsg == WM_NCRBUTTONUP) {
        if (wParam == HTCAPTION) {
            shouldShowSystemMenu = true;
            globalPos = getGlobalPosFromMouse();
        }
    } else if (uMsg == WM_SYSCOMMAND) {
        const WPARAM filteredWParam = (wParam & 0xFFF0);
        if ((filteredWParam == SC_KEYMENU) && (lParam == VK_SPACE)) {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            globalPos = getGlobalPosFromKeyboard();
        }
    } else if ((uMsg == WM_KEYDOWN) || (uMsg == WM_SYSKEYDOWN)) {
        const bool altPressed = ((wParam == VK_MENU) || (GetKeyState(VK_MENU) < 0));
        const bool spacePressed = ((wParam == VK_SPACE) || (GetKeyState(VK_SPACE) < 0));
        if (altPressed && spacePressed) {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            globalPos = getGlobalPosFromKeyboard();
        }
    }
    if (shouldShowSystemMenu) {
        Utils::showSystemMenu(windowId, globalPos, data.offset,
                              broughtByKeyboard, data.options, data.isWindowFixedSize);
        // QPA's internal code will handle system menu events separately, and its
        // behavior is not what we would want to see because it doesn't know our
        // window doesn't have any window frame now, so return early here to avoid
        // entering Qt's own handling logic.
        return 0; // Return 0 means we have handled this event.
    }
    Q_ASSERT(data.originalWindowProc);
    if (data.originalWindowProc) {
        // Hand over to Qt's original window proc function for events we are not
        // interested in.
        return CallWindowProcW(data.originalWindowProc, hWnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

bool Utils::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8);
#endif
    return result;
}

bool Utils::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1);
#endif
    return result;
}

bool Utils::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}

bool Utils::isWin11OrGreater()
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

bool Utils::isDwmCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    const auto resultFromRegistry = []() -> bool {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, qDwmRegistryKey);
        const auto result = registry.dwordValue(kComposition);
        return (result.second && (result.first != 0));
    };
    static const auto pDwmIsCompositionEnabled =
        reinterpret_cast<decltype(&DwmIsCompositionEnabled)>(
            QSystemLibrary::resolve(kdwmapi, "DwmIsCompositionEnabled"));
    if (!pDwmIsCompositionEnabled) {
        return resultFromRegistry();
    }
    BOOL enabled = FALSE;
    const HRESULT hr = pDwmIsCompositionEnabled(&enabled);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmIsCompositionEnabled, hr);
        return resultFromRegistry();
    }
    return (enabled != FALSE);
}

void Utils::triggerFrameChange(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    static constexpr const UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        qWarning() << getSystemErrorMessage(kSetWindowPos);
    }
}

void Utils::updateWindowFrameMargins(const WId windowId, const bool reset)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    // We can't extend the window frame when DWM composition is disabled.
    // No need to try further in this case.
    if (!isDwmCompositionEnabled()) {
        return;
    }
    static const auto pDwmExtendFrameIntoClientArea =
        reinterpret_cast<decltype(&DwmExtendFrameIntoClientArea)>(
            QSystemLibrary::resolve(kdwmapi, "DwmExtendFrameIntoClientArea"));
    if (!pDwmExtendFrameIntoClientArea) {
        return;
    }
    const MARGINS margins = [reset]() -> MARGINS {
        if (reset || isWindowFrameBorderVisible()) {
            return {0, 0, 0, 0};
        } else {
            return {1, 1, 1, 1};
        }
    }();
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    const HRESULT hr = pDwmExtendFrameIntoClientArea(hwnd, &margins);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmExtendFrameIntoClientArea, hr);
        return;
    }
    triggerFrameChange(windowId);
}

void Utils::updateInternalWindowFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const WId windowId = window->winId();
    const QMargins margins = [enable, windowId]() -> QMargins {
        if (!enable) {
            return {};
        }
        const int titleBarHeight = getTitleBarHeight(windowId, true);
        if (isWindowFrameBorderVisible()) {
            return {0, -titleBarHeight, 0, 0};
        } else {
            const int frameSizeX = getResizeBorderThickness(windowId, true, true);
            const int frameSizeY = getResizeBorderThickness(windowId, false, true);
            return {-frameSizeX, -titleBarHeight, -frameSizeX, -frameSizeY};
        }
    }();
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty("_q_windowsCustomMargins", marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (QPlatformWindow *platformWindow = window->handle()) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, kWindowsCustomMargins, marginsVar);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
        return;
    }
#else
    if (const auto platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(window->handle())) {
        platformWindow->setCustomMargins(margins);
    } else {
        qWarning() << "Failed to retrieve the platform window.";
        return;
    }
#endif
    triggerFrameChange(windowId);
}

QString Utils::getSystemErrorMessage(const QString &function)
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

QColor Utils::getDwmColorizationColor()
{
    const auto resultFromRegistry = []() -> QColor {
        const QWinRegistryKey registry(HKEY_CURRENT_USER, qDwmRegistryKey);
        const auto result = registry.dwordValue(kColorizationColor);
        return (result.second ? QColor::fromRgba(result.first) : kDefaultDarkGrayColor);
    };
    static const auto pDwmGetColorizationColor =
        reinterpret_cast<decltype(&DwmGetColorizationColor)>(
            QSystemLibrary::resolve(kdwmapi, "DwmGetColorizationColor"));
    if (!pDwmGetColorizationColor) {
        return resultFromRegistry();
    }
    DWORD color = 0;
    BOOL opaque = FALSE;
    const HRESULT hr = pDwmGetColorizationColor(&color, &opaque);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmGetColorizationColor, hr);
        return resultFromRegistry();
    }
    return QColor::fromRgba(color);
}

DwmColorizationArea Utils::getDwmColorizationArea()
{
    // It's a Win10 only feature. (TO BE VERIFIED)
    if (!isWin10OrGreater()) {
        return DwmColorizationArea::None_;
    }
    const QWinRegistryKey themeRegistry(HKEY_CURRENT_USER, qPersonalizeRegistryKey);
    const auto themeValue = themeRegistry.dwordValue(qDwmColorKeyName);
    const QWinRegistryKey dwmRegistry(HKEY_CURRENT_USER, qDwmRegistryKey);
    const auto dwmValue = dwmRegistry.dwordValue(qDwmColorKeyName);
    const bool theme = themeValue.second && (themeValue.first != 0);
    const bool dwm = dwmValue.second && (dwmValue.first != 0);
    if (theme && dwm) {
        return DwmColorizationArea::All;
    } else if (theme) {
        return DwmColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return DwmColorizationArea::TitleBar_WindowBorder;
    }
    return DwmColorizationArea::None_;
}

void Utils::showSystemMenu(const WId windowId, const QPoint &pos, const QPoint &offset,
                           const bool selectFirstEntry, const Options options,
                           const IsWindowFixedSizeCallback &isWindowFixedSize)
{
    Q_ASSERT(windowId);
    Q_ASSERT(isWindowFixedSize);
    if (!windowId || !isWindowFixedSize) {
        return;
    }
    const auto hWnd = reinterpret_cast<HWND>(windowId);
    const HMENU hMenu = GetSystemMenu(hWnd, FALSE);
    if (!hMenu) {
        // The corresponding window doesn't have a system menu, most likely due to the
        // lack of the "WS_SYSMENU" window style. This situation should not be treated
        // as an error so just ignore it and return early.
        return;
    }
    const bool maxOrFull = (IsMaximized(hWnd) ||
           ((options & Option::DontTreatFullScreenAsZoomed) ? false : isFullScreen(windowId)));
    const bool fixedSize = isWindowFixedSize();
    EnableMenuItem(hMenu, SC_RESTORE, (MF_BYCOMMAND | ((maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    // The first menu item should be selected by default if the menu is brought
    // by keyboard. I don't know how to pre-select a menu item but it seems
    // highlight can do the job. However, there's an annoying issue if we do
    // this manually: the highlighted menu item is really only highlighted,
    // not selected, so even if the mouse cursor hovers on other menu items
    // or the user navigates to other menu items through keyboard, the original
    // highlight bar will not move accordingly, the OS will generate another
    // highlight bar to indicate the current selected menu item, which will make
    // the menu look kind of weird. Currently I don't know how to fix this issue.
    HiliteMenuItem(hWnd, hMenu, SC_RESTORE, (MF_BYCOMMAND | (selectFirstEntry ? MFS_HILITE : MFS_UNHILITE)));
    EnableMenuItem(hMenu, SC_MOVE, (MF_BYCOMMAND | ((!maxOrFull && !(options & Option::DisableDragging)) ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_SIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_MINIMIZE, (MF_BYCOMMAND | MFS_ENABLED));
    EnableMenuItem(hMenu, SC_MAXIMIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_CLOSE, (MF_BYCOMMAND | MFS_ENABLED));
    // The default menu item will appear in bold font. There can only be one default
    // menu item per menu at most. Set the item ID to "UINT_MAX" (or simply "-1")
    // can clear the default item for the given menu.
    SetMenuDefaultItem(hMenu, SC_CLOSE, FALSE);
    // If you need to adjust the menu popup position (such as in a fully
    // customized window frame), you should pass in the "offset" parameter.
    // But it will not be needed when the window is maximized or fullscreen
    // because in that case the menu should always align to the left edge
    // of the screen.
    const QPoint adjustment = (maxOrFull ? QPoint(0, 0) : offset);
    const int xPos = (pos.x() + adjustment.x());
    const int yPos = (pos.y() + adjustment.y());
    const int ret = TrackPopupMenu(hMenu, (TPM_RETURNCMD | (QGuiApplication::isRightToLeft()
                            ? TPM_RIGHTALIGN : TPM_LEFTALIGN)), xPos, yPos, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            qWarning() << getSystemErrorMessage(kPostMessageW);
        }
    }
}

bool Utils::isFullScreen(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    RECT wndRect = {};
    if (GetWindowRect(hwnd, &wndRect) == FALSE) {
        qWarning() << getSystemErrorMessage(kGetWindowRect);
        return false;
    }
    // According to Microsoft Docs, we should compare to the primary screen's geometry
    // (if we can't determine the correct screen of our window).
    const HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    if (!mon) {
        qWarning() << getSystemErrorMessage(kMonitorFromWindow);
        return false;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        qWarning() << getSystemErrorMessage(kGetMonitorInfoW);
        return false;
    }
    // Compare to the full area of the screen, not the work area.
    const RECT scrRect = mi.rcMonitor;
    return ((wndRect.left == scrRect.left) && (wndRect.top == scrRect.top)
            && (wndRect.right == scrRect.right) && (wndRect.bottom == scrRect.bottom));
}

bool Utils::isWindowNoState(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp); // This line is important! Don't miss it!
    if (GetWindowPlacement(hwnd, &wp) == FALSE) {
        qWarning() << getSystemErrorMessage(kGetWindowPlacement);
        return false;
    }
    return ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE));
}

void Utils::syncWmPaintWithDwm()
{
    // No need to sync with DWM if DWM composition is disabled.
    if (!isDwmCompositionEnabled()) {
        return;
    }
    QSystemLibrary winmmLib(kwinmm);
    static const auto ptimeGetDevCaps =
        reinterpret_cast<decltype(&timeGetDevCaps)>(winmmLib.resolve("timeGetDevCaps"));
    static const auto ptimeBeginPeriod =
        reinterpret_cast<decltype(&timeBeginPeriod)>(winmmLib.resolve("timeBeginPeriod"));
    static const auto ptimeEndPeriod =
        reinterpret_cast<decltype(&timeEndPeriod)>(winmmLib.resolve("timeEndPeriod"));
    static const auto pDwmGetCompositionTimingInfo =
        reinterpret_cast<decltype(&DwmGetCompositionTimingInfo)>(
            QSystemLibrary::resolve(kdwmapi, "DwmGetCompositionTimingInfo"));
    if (!ptimeGetDevCaps || !ptimeBeginPeriod || !ptimeEndPeriod || !pDwmGetCompositionTimingInfo) {
        return;
    }
    // Dirty hack to workaround the resize flicker caused by DWM.
    LARGE_INTEGER freq = {};
    if (QueryPerformanceFrequency(&freq) == FALSE) {
        qWarning() << getSystemErrorMessage(kQueryPerformanceFrequency);
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
        qWarning() << getSystemErrorMessage(kQueryPerformanceCounter);
        return;
    }
    // ask DWM where the vertical blank falls
    DWM_TIMING_INFO dti;
    SecureZeroMemory(&dti, sizeof(dti));
    dti.cbSize = sizeof(dti);
    const HRESULT hr = pDwmGetCompositionTimingInfo(nullptr, &dti);
    if (FAILED(hr)) {
        qWarning() << getSystemErrorMessage(kDwmGetCompositionTimingInfo);
        return;
    }
    LARGE_INTEGER now1 = {};
    if (QueryPerformanceCounter(&now1) == FALSE) {
        qWarning() << getSystemErrorMessage(kQueryPerformanceCounter);
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
    const qreal m_ms = (1000.0 * qreal(m) / qreal(freq.QuadPart));
    Sleep(static_cast<DWORD>(qRound(m_ms)));
    if (ptimeEndPeriod(ms_granularity) != TIMERR_NOERROR) {
        qWarning() << "timeEndPeriod() failed.";
    }
}

bool Utils::isWin101607OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 14393));
#else
    static const bool result = isWindowsVersionOrGreater(10, 0, 14393);
#endif
    return result;
}

bool Utils::isWin101809OrGreater()
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

bool Utils::isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kSystemParametersInfoW);
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

quint32 Utils::getPrimaryScreenDpi(const bool horizontal)
{
    static const auto pGetDpiForMonitor =
        reinterpret_cast<decltype(&GetDpiForMonitor)>(
            QSystemLibrary::resolve(kshcore, "GetDpiForMonitor"));
    if (pGetDpiForMonitor) {
        const HMONITOR monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
        if (monitor) {
            UINT dpiX = 0, dpiY = 0;
            if (SUCCEEDED(pGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                return (horizontal ? dpiX : dpiY);
            }
        }
    }
    static const auto pD2D1CreateFactory =
        reinterpret_cast<HRESULT(WINAPI *)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void **)>(
            QSystemLibrary::resolve(kd2d1, "D2D1CreateFactory"));
    if (pD2D1CreateFactory) {
        CComPtr<ID2D1Factory> d2dFactory = nullptr;
        if (SUCCEEDED(pD2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory),
                                         nullptr, reinterpret_cast<void **>(&d2dFactory)))) {
            if (SUCCEEDED(d2dFactory->ReloadSystemMetrics())) {
                FLOAT dpiX = 0.0, dpiY = 0.0;
                QT_WARNING_PUSH
                QT_WARNING_DISABLE_DEPRECATED
                d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
                QT_WARNING_POP
                return (horizontal ? quint32(qRound(dpiX)) : quint32(qRound(dpiY)));
            }
        }
    }
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

quint32 Utils::getWindowDpi(const WId windowId, const bool horizontal)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    QSystemLibrary user32Lib(kuser32);
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
            QSystemLibrary::resolve(kshcore, "GetDpiForMonitor"));
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

quint32 Utils::getResizeBorderThickness(const WId windowId, const bool horizontal, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return 0;
    }
    if (horizontal) {
        return (getSystemMetrics2(windowId, SM_CXSIZEFRAME, true, scaled)
                + getSystemMetrics2(windowId, SM_CXPADDEDBORDER, true, scaled));
    } else {
        return (getSystemMetrics2(windowId, SM_CYSIZEFRAME, false, scaled)
                + getSystemMetrics2(windowId, SM_CYPADDEDBORDER, false, scaled));
    }
}

quint32 Utils::getCaptionHeight(const WId windowId, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return 0;
    }
    return getSystemMetrics2(windowId, SM_CYCAPTION, false, scaled);
}

quint32 Utils::getTitleBarHeight(const WId windowId, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return 0;
    }
    return (getCaptionHeight(windowId, scaled) + getResizeBorderThickness(windowId, false, scaled));
}

quint32 Utils::getFrameBorderThickness(const WId windowId, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return 0;
    }
    // There's no window frame border before Windows 10.
    if (!isWin10OrGreater()) {
        return 0;
    }
    static const auto pDwmGetWindowAttribute =
        reinterpret_cast<decltype(&DwmGetWindowAttribute)>(
            QSystemLibrary::resolve(kdwmapi, "DwmGetWindowAttribute"));
    if (!pDwmGetWindowAttribute) {
        return 0;
    }
    const UINT dpi = getWindowDpi(windowId, true);
    const qreal scaleFactor = (qreal(dpi) / qreal(USER_DEFAULT_SCREEN_DPI));
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    UINT value = 0;
    if (SUCCEEDED(pDwmGetWindowAttribute(hwnd, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value)))) {
        const qreal dpr = (scaled ? 1.0 : scaleFactor);
        return static_cast<int>(qRound(qreal(value) / dpr));
    } else {
        const qreal dpr = (scaled ? scaleFactor : 1.0);
        return static_cast<int>(qRound(qreal(kDefaultWindowFrameBorderThickness) * dpr));
    }
}

QColor Utils::getFrameBorderColor(const bool active)
{
    // There's no window frame border before Windows 10.
    // So we just return a default value which is based on most window managers.
    if (!isWin10OrGreater()) {
        return (active ? kDefaultBlackColor : kDefaultDarkGrayColor);
    }
    const bool dark = shouldAppsUseDarkMode();
    if (active) {
        if (isFrameBorderColorized()) {
            return getDwmColorizationColor();
        } else {
            return kDefaultFrameBorderActiveColor;
        }
    } else {
        return (dark ? kDefaultFrameBorderInactiveColorDark : kDefaultFrameBorderInactiveColorLight);
    }
}

void Utils::updateWindowFrameBorderColor(const WId windowId, const bool dark)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    // There's no global dark theme before Win10 1607.
    if (!isWin101607OrGreater()) {
        return;
    }
    static const auto pDwmSetWindowAttribute =
        reinterpret_cast<decltype(&DwmSetWindowAttribute)>(
            QSystemLibrary::resolve(kdwmapi, "DwmSetWindowAttribute"));
    if (!pDwmSetWindowAttribute) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    const BOOL value = (dark ? TRUE : FALSE);
    // Whether dark window frame is available or not depends on the runtime system version,
    // it's totally OK if it's not available, so just ignore the errors.
    pDwmSetWindowAttribute(hwnd, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &value, sizeof(value));
    pDwmSetWindowAttribute(hwnd, _DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void Utils::fixupQtInternals(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
#if 0
    SetLastError(ERROR_SUCCESS);
    const auto oldClassStyle = static_cast<DWORD>(GetClassLongPtrW(hwnd, GCL_STYLE));
    if (oldClassStyle == 0) {
        qWarning() << getSystemErrorMessage(kGetClassLongPtrW);
        return;
    }
    const DWORD newClassStyle = (oldClassStyle | CS_HREDRAW | CS_VREDRAW);
    SetLastError(ERROR_SUCCESS);
    if (SetClassLongPtrW(hwnd, GCL_STYLE, static_cast<LONG_PTR>(newClassStyle)) == 0) {
        qWarning() << getSystemErrorMessage(kSetClassLongPtrW);
        return;
    }
#endif
    SetLastError(ERROR_SUCCESS);
    const auto oldWindowStyle = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (oldWindowStyle == 0) {
        qWarning() << getSystemErrorMessage(kGetWindowLongPtrW);
        return;
    }
    // Qt by default adds the "WS_POPUP" flag to all Win32 windows it created and maintained,
    // which is not a good thing (although it won't cause any obvious issues in most cases
    // either), because popup windows have some different behavior with normal overlapped
    // windows, for example, it will affect DWM's default policy. And Qt will also not add
    // the "WS_OVERLAPPED" flag to the windows in some cases, which also causes some trouble
    // for us. To avoid some weird bugs, we do the correction here: remove the WS_POPUP flag
    // and add the WS_OVERLAPPED flag, unconditionally. If your window really don't need this
    // correction, it also means you should not use this framework, because without this
    // correction, our core frameless functionality will be broken in some degree.
    const DWORD newWindowStyle = ((oldWindowStyle & ~WS_POPUP) | WS_OVERLAPPED);
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWL_STYLE, static_cast<LONG_PTR>(newWindowStyle)) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    triggerFrameChange(windowId);
}

void Utils::startSystemMove(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    sendMouseReleaseEvent();
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, 0xF012 /*SC_DRAGMOVE*/, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kPostMessageW);
    }
#endif
}

void Utils::startSystemResize(QWindow *window, const Qt::Edges edges)
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
    sendMouseReleaseEvent();
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, qtEdgesToWin32Orientation(edges), 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kPostMessageW);
    }
#endif
}

bool Utils::isWindowFrameBorderVisible()
{
    static const bool result = []() -> bool {
        if (FramelessWindowsManager::usePureQtImplementation()) {
            return false;
        }
        // If we preserve the window frame border on systems prior to Windows 10,
        // the window will look rather ugly and I guess no one would like to see
        // such weired windows. But for the ones who really want to see what the
        // window look like, I still provide a way to enter such scenarios.
        if (qEnvironmentVariableIntValue(kForceShowFrameBorderFlag) != 0) {
            return true;
        }
        if (qEnvironmentVariableIntValue(kForceHideFrameBorderFlag) != 0) {
            return false;
        }
        const QString iniFilePath = QCoreApplication::applicationDirPath() + u'/' + kConfigFileName;
        QSettings settings(iniFilePath, QSettings::IniFormat);
        if (settings.value(kForceShowFrameBorderKeyPath, false).toBool()) {
            return true;
        }
        if (settings.value(kForceHideFrameBorderKeyPath, false).toBool()) {
            return false;
        }
        return isWin10OrGreater();
    }();
    return result;
}

bool Utils::isTitleBarColorized()
{
    // CHECK: is it supported on win7?
    if (!isWin10OrGreater()) {
        return false;
    }
    const DwmColorizationArea area = getDwmColorizationArea();
    return ((area == DwmColorizationArea::TitleBar_WindowBorder) || (area == DwmColorizationArea::All));
}

bool Utils::isFrameBorderColorized()
{
    return isTitleBarColorized();
}

void Utils::installSystemMenuHook(const WId windowId, const Options options, const QPoint &offset,
                                  const IsWindowFixedSizeCallback &isWindowFixedSize)
{
    Q_ASSERT(windowId);
    Q_ASSERT(isWindowFixedSize);
    if (!windowId || !isWindowFixedSize) {
        return;
    }
    QMutexLocker locker(&g_utilsHelper()->mutex);
    if (g_utilsHelper()->data.contains(windowId)) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    SetLastError(ERROR_SUCCESS);
    const auto originalWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    Q_ASSERT(originalWindowProc);
    if (!originalWindowProc) {
        qWarning() << getSystemErrorMessage(kGetWindowLongPtrW);
        return;
    }
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SystemMenuHookWindowProc)) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    //triggerFrameChange(windowId);
    Win32UtilsHelperData data = {};
    data.originalWindowProc = originalWindowProc;
    data.options = options;
    data.offset = offset;
    data.isWindowFixedSize = isWindowFixedSize;
    g_utilsHelper()->data.insert(windowId, data);
}

void Utils::uninstallSystemMenuHook(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    QMutexLocker locker(&g_utilsHelper()->mutex);
    if (!g_utilsHelper()->data.contains(windowId)) {
        return;
    }
    const Win32UtilsHelperData data = g_utilsHelper()->data.value(windowId);
    Q_ASSERT(data.originalWindowProc);
    if (!data.originalWindowProc) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(data.originalWindowProc)) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    //triggerFrameChange(windowId);
    g_utilsHelper()->data.remove(windowId);
}

void Utils::sendMouseReleaseEvent()
{
    if (ReleaseCapture() == FALSE) {
        qWarning() << getSystemErrorMessage(kReleaseCapture);
    }
}

void Utils::tryToBeCompatibleWithQtFramelessWindowHint(const WId windowId,
                                                       const GetWindowFlagsCallback &getWindowFlags,
                                                       const SetWindowFlagsCallback &setWindowFlags,
                                                       const bool enable)
{
    Q_ASSERT(windowId);
    Q_ASSERT(getWindowFlags);
    Q_ASSERT(setWindowFlags);
    if (!windowId || !getWindowFlags || !setWindowFlags) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    SetLastError(ERROR_SUCCESS);
    const LONG_PTR originalWindowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (originalWindowStyle == 0) {
        qWarning() << getSystemErrorMessage(kGetWindowLongPtrW);
        return;
    }
    const Qt::WindowFlags originalWindowFlags = getWindowFlags();
    const Qt::WindowFlags newWindowFlags = (enable ? (originalWindowFlags | Qt::FramelessWindowHint)
                                                   : (originalWindowFlags & ~Qt::FramelessWindowHint));
    setWindowFlags(newWindowFlags);
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWL_STYLE, originalWindowStyle) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    triggerFrameChange(windowId);
}

void Utils::setAeroSnappingEnabled(const WId windowId, const bool enable)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    SetLastError(ERROR_SUCCESS);
    const auto oldWindowStyle = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (oldWindowStyle == 0) {
        qWarning() << getSystemErrorMessage(kGetWindowLongPtrW);
        return;
    }
    const DWORD newWindowStyle = [enable, oldWindowStyle]() -> DWORD {
        if (enable) {
            return ((oldWindowStyle & ~WS_POPUP) | WS_THICKFRAME);
        } else {
            return ((oldWindowStyle & ~WS_THICKFRAME) | WS_POPUP);
        }
    }();
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWL_STYLE, static_cast<LONG_PTR>(newWindowStyle)) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    triggerFrameChange(windowId);
}

void Utils::tryToEnableHighestDpiAwarenessLevel()
{
    static const auto pSetProcessDpiAwarenessContext =
        reinterpret_cast<decltype(&SetProcessDpiAwarenessContext)>(
            QSystemLibrary::resolve(kuser32, "SetProcessDpiAwarenessContext"));
    if (pSetProcessDpiAwarenessContext) {
        if (pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE) {
            return;
        }
        const DWORD dwError = GetLastError();
        // "ERROR_ACCESS_DENIED" means set externally (mostly due to manifest file).
        if (dwError == ERROR_ACCESS_DENIED) {
            return;
        }
        if (pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE) != FALSE) {
            return;
        }
        if (pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE) != FALSE) {
            return;
        }
        if (pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED) != FALSE) {
            return;
        }
    }
    static const auto pSetProcessDpiAwareness =
        reinterpret_cast<decltype(&SetProcessDpiAwareness)>(
            QSystemLibrary::resolve(kshcore, "SetProcessDpiAwareness"));
    if (pSetProcessDpiAwareness) {
        // This enum value is our own extension, so don't check for "E_ACCESSDENIED"
        // because it won't appear in anywhere outside of our own code.
        if (SUCCEEDED(pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE_V2))) {
            return;
        }
        const HRESULT hr = pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        if (SUCCEEDED(hr)) {
            return;
        }
        // "E_ACCESSDENIED" means set externally (mostly due to manifest file).
        if (hr == E_ACCESSDENIED) {
            return;
        }
        if (SUCCEEDED(pSetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE))) {
            return;
        }
    }
    SetProcessDPIAware();
}

SystemTheme Utils::getSystemTheme()
{
    if (isHighContrastModeEnabled()) {
        return SystemTheme::HighContrast;
    }
    if (isWin101607OrGreater() && shouldAppsUseDarkMode()) {
        return SystemTheme::Dark;
    }
    return SystemTheme::Light;
}

void Utils::updateGlobalWin32ControlsTheme(const WId windowId, const bool dark)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    // There's no global dark theme for common Win32 controls before Win10 1809.
    if (!isWin101809OrGreater()) {
        return;
    }
    static const auto pSetWindowTheme =
        reinterpret_cast<decltype(&SetWindowTheme)>(
            QSystemLibrary::resolve(kuxtheme, "SetWindowTheme"));
    if (!pSetWindowTheme) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    // The result depends on the runtime system version, no need to check.
    pSetWindowTheme(hwnd, (dark ? kSystemDarkThemeResourceName : kSystemLightThemeResourceName), nullptr);
}

FRAMELESSHELPER_END_NAMESPACE
