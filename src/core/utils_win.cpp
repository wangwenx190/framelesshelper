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
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qpa/qplatformnativeinterface.h>
#else
#  include <QtGui/qpa/qplatformwindow_p.h>
#endif
#include "framelessmanager.h"
#include "framelesshelper_windows.h"
#include "framelessconfig_p.h"
#include "sysapiloader_p.h"
#include <d2d1.h>

Q_DECLARE_METATYPE(QMargins)

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct Win32UtilsHelperData
{
    WNDPROC originalWindowProc = nullptr;
    IsWindowFixedSizeCallback isWindowFixedSize = nullptr;
    IsInsideTitleBarDraggableAreaCallback isInTitleBarArea = nullptr;
    GetWindowDevicePixelRatioCallback getDevicePixelRatio = nullptr;
};

struct Win32UtilsHelper
{
    QMutex mutex;
    QHash<WId, Win32UtilsHelperData> data = {};
};

Q_GLOBAL_STATIC(Win32UtilsHelper, g_utilsHelper)

static constexpr const wchar_t DUMMY_WINDOW_CLASS_NAME[] = L"FRAMELESSHELPER_DUMMY_WINDOW_CLASS";
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
#else // Q_PROCESSOR_X86_64
  // WinUser.h defines G/SetClassLongPtr as G/SetClassLong due to the
  // "Ptr" suffixed APIs are not available on 32-bit platforms, so we
  // have to add the following workaround. Undefine the macros and then
  // redefine them is also an option but the following solution is more simple.
  FRAMELESSHELPER_STRING_CONSTANT2(GetClassLongPtrW, "GetClassLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetClassLongPtrW, "SetClassLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(GetWindowLongPtrW, "GetWindowLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetWindowLongPtrW, "SetWindowLongW")
#endif // Q_PROCESSOR_X86_64
FRAMELESSHELPER_STRING_CONSTANT(ReleaseCapture)
FRAMELESSHELPER_STRING_CONSTANT(SetWindowTheme)
FRAMELESSHELPER_STRING_CONSTANT(SetProcessDpiAwarenessContext)
FRAMELESSHELPER_STRING_CONSTANT(SetProcessDpiAwareness)
FRAMELESSHELPER_STRING_CONSTANT(SetProcessDPIAware)
FRAMELESSHELPER_STRING_CONSTANT(GetDpiForMonitor)
FRAMELESSHELPER_STRING_CONSTANT(MonitorFromPoint)
FRAMELESSHELPER_STRING_CONSTANT(D2D1CreateFactory)
FRAMELESSHELPER_STRING_CONSTANT(ReloadSystemMetrics)
FRAMELESSHELPER_STRING_CONSTANT(GetDC)
FRAMELESSHELPER_STRING_CONSTANT(ReleaseDC)
FRAMELESSHELPER_STRING_CONSTANT(GetDeviceCaps)
FRAMELESSHELPER_STRING_CONSTANT(DwmSetWindowAttribute)
FRAMELESSHELPER_STRING_CONSTANT(EnableMenuItem)
FRAMELESSHELPER_STRING_CONSTANT(SetMenuDefaultItem)
FRAMELESSHELPER_STRING_CONSTANT(HiliteMenuItem)
FRAMELESSHELPER_STRING_CONSTANT(TrackPopupMenu)
FRAMELESSHELPER_STRING_CONSTANT(ClientToScreen)
FRAMELESSHELPER_STRING_CONSTANT2(HKEY_CURRENT_USER, "HKEY_CURRENT_USER")
FRAMELESSHELPER_STRING_CONSTANT(DwmEnableBlurBehindWindow)
FRAMELESSHELPER_STRING_CONSTANT(SetWindowCompositionAttribute)
FRAMELESSHELPER_STRING_CONSTANT(GetSystemMetricsForDpi)
FRAMELESSHELPER_STRING_CONSTANT(timeGetDevCaps)
FRAMELESSHELPER_STRING_CONSTANT(timeBeginPeriod)
FRAMELESSHELPER_STRING_CONSTANT(timeEndPeriod)
FRAMELESSHELPER_STRING_CONSTANT(GetDpiForWindow)
FRAMELESSHELPER_STRING_CONSTANT(GetSystemDpiForProcess)
FRAMELESSHELPER_STRING_CONSTANT(GetDpiForSystem)
FRAMELESSHELPER_STRING_CONSTANT(DwmGetWindowAttribute)
FRAMELESSHELPER_STRING_CONSTANT(ntdll)
FRAMELESSHELPER_STRING_CONSTANT(RtlGetVersion)
FRAMELESSHELPER_STRING_CONSTANT(GetModuleHandleW)
FRAMELESSHELPER_STRING_CONSTANT(RegisterClassExW)
FRAMELESSHELPER_STRING_CONSTANT(CreateWindowExW)

template <typename T>
class HumbleComPtr
{
    Q_DISABLE_COPY_MOVE(HumbleComPtr)

public:
    HumbleComPtr() = default;

    HumbleComPtr(std::nullptr_t ptr)
    {
        Q_UNUSED(ptr);
    }

    ~HumbleComPtr()
    {
        if (p) {
            p->Release();
            p = nullptr;
        }
    }

    [[nodiscard]] operator T*() const
    {
        return p;
    }

    [[nodiscard]] T &operator*() const
    {
        Q_ASSERT(p);
        return *p;
    }

    // The assert on operator& usually indicates a bug. If this is really
    // what is needed, however, take the address of the p member explicitly.
    [[nodiscard]] T **operator&()
    {
        Q_ASSERT(false);
        return &p;
    }

    [[nodiscard]] T *operator->() const
    {
        Q_ASSERT(p);
        return p;
    }

    [[nodiscard]] bool operator!() const
    {
        return (p == nullptr);
    }

private:
    T *p = nullptr;
};

[[nodiscard]] static inline HWND ensureDummyWindow()
{
    static const HWND hwnd = []() -> HWND {
        const HMODULE instance = GetModuleHandleW(nullptr);
        if (!instance) {
            qWarning() << Utils::getSystemErrorMessage(kGetModuleHandleW);
            return nullptr;
        }
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.lpfnWndProc = DefWindowProcW;
        wcex.hInstance = instance;
        wcex.lpszClassName = DUMMY_WINDOW_CLASS_NAME;
        const ATOM atom = RegisterClassExW(&wcex);
        if (!atom) {
            qWarning() << Utils::getSystemErrorMessage(kRegisterClassExW);
            return nullptr;
        }
        const HWND window = CreateWindowExW(0, DUMMY_WINDOW_CLASS_NAME, nullptr,
            WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
        if (!window) {
            qWarning() << Utils::getSystemErrorMessage(kCreateWindowExW);
            return nullptr;
        }
        return window;
    }();
    return hwnd;
}

[[nodiscard]] static inline bool doCompareWindowsVersion(const VersionNumber &targetOsVer)
{
    static const std::optional<VersionNumber> currentOsVer = []() -> std::optional<VersionNumber> {
        if (API_NT_AVAILABLE(RtlGetVersion)) {
            using RtlGetVersionPtr = NTSTATUS(WINAPI *)(PRTL_OSVERSIONINFOW);
            const auto pRtlGetVersion =
                reinterpret_cast<RtlGetVersionPtr>(SysApiLoader::instance()->get(kRtlGetVersion));
            RTL_OSVERSIONINFOEXW osvi;
            SecureZeroMemory(&osvi, sizeof(osvi));
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (pRtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) == STATUS_SUCCESS) {
                return VersionNumber{int(osvi.dwMajorVersion), int(osvi.dwMinorVersion), int(osvi.dwBuildNumber)};
            }
        }
        return std::nullopt;
    }();
    if (currentOsVer.has_value()) {
        return (currentOsVer >= targetOsVer);
    }
    // We can fallback to "VerifyVersionInfoW" if we can't determine the current system
    // version, but this function will be affected by the manifest file of your application.
    // For example, if you don't claim your application supports Windows 10 explicitly
    // in the manifest file, Windows will assume your application only supports up to Windows
    // 8.1, so this function will be told the current system is at most Windows 8.1, to keep
    // good backward-compatiability. This behavior usually won't cause any issues if you
    // always use an appropriate manifest file for your application, however, it does cause
    // some issues for people who don't use the manifest file at all. There have been some
    // bug reports about it already.
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = targetOsVer.major;
    osvi.dwMinorVersion = targetOsVer.minor;
    osvi.dwBuildNumber = targetOsVer.patch;
    DWORDLONG dwlConditionMask = 0;
    const auto op = VER_GREATER_EQUAL;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, (VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER), dwlConditionMask) != FALSE);
}

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
    if (API_USER_AVAILABLE(GetSystemMetricsForDpi)) {
        const UINT dpi = (scaled ? windowDpi : USER_DEFAULT_SCREEN_DPI);
        return API_CALL_FUNCTION(GetSystemMetricsForDpi, index, dpi);
    } else {
        // The returned value is already scaled, we need to divide the dpr to get the unscaled value.
        const qreal dpr = (scaled ? 1.0 : (qreal(windowDpi) / qreal(USER_DEFAULT_SCREEN_DPI)));
        return qRound(qreal(GetSystemMetrics(index)) / dpr);
    }
}

[[maybe_unused]] [[nodiscard]] static inline
    DWORD qtEdgesToWin32Orientation(const Qt::Edges edges)
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
    const auto getNativePosFromMouse = [lParam]() -> QPoint {
        return {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    };
    const auto getNativeGlobalPosFromKeyboard = [hWnd, windowId]() -> QPoint {
        RECT windowPos = {};
        if (GetWindowRect(hWnd, &windowPos) == FALSE) {
            qWarning() << Utils::getSystemErrorMessage(kGetWindowRect);
            return {};
        }
        const bool maxOrFull = (IsMaximized(hWnd) || Utils::isFullScreen(windowId));
        const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
        const bool frameBorderVisible = Utils::isWindowFrameBorderVisible();
        const int horizontalOffset = ((maxOrFull || !frameBorderVisible) ? 0 : frameSizeX);
        const int verticalOffset = [windowId, frameBorderVisible, maxOrFull]() -> int {
            const int titleBarHeight = Utils::getTitleBarHeight(windowId, true);
            if (!frameBorderVisible) {
                return titleBarHeight;
            }
            const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
            static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
            if (isWin11OrGreater) {
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
    QPoint nativeGlobalPos = {};
    switch (uMsg) {
    case WM_RBUTTONUP: {
        const QPoint nativeLocalPos = getNativePosFromMouse();
        const qreal dpr = data.getDevicePixelRatio();
        const QPoint qtScenePos = QPointF(QPointF(nativeLocalPos) / dpr).toPoint();
        if (data.isInTitleBarArea(qtScenePos)) {
            POINT pos = {nativeLocalPos.x(), nativeLocalPos.y()};
            if (ClientToScreen(hWnd, &pos) == FALSE) {
                qWarning() << Utils::getSystemErrorMessage(kClientToScreen);
                break;
            }
            shouldShowSystemMenu = true;
            nativeGlobalPos = {pos.x, pos.y};
        }
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            shouldShowSystemMenu = true;
            nativeGlobalPos = getNativePosFromMouse();
        }
    } break;
    case WM_SYSCOMMAND: {
        const WPARAM filteredWParam = (wParam & 0xFFF0);
        if ((filteredWParam == SC_KEYMENU) && (lParam == VK_SPACE)) {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            nativeGlobalPos = getNativeGlobalPosFromKeyboard();
        }
    } break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        const bool altPressed = ((wParam == VK_MENU) || (GetKeyState(VK_MENU) < 0));
        const bool spacePressed = ((wParam == VK_SPACE) || (GetKeyState(VK_SPACE) < 0));
        if (altPressed && spacePressed) {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            nativeGlobalPos = getNativeGlobalPosFromKeyboard();
        }
    } break;
    default:
        break;
    }
    if (shouldShowSystemMenu) {
        Utils::showSystemMenu(windowId, nativeGlobalPos, broughtByKeyboard, data.isWindowFixedSize);
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

bool Utils::isWindowsVersionOrGreater(const WindowsVersion version)
{
    return doCompareWindowsVersion(WindowsVersions[static_cast<int>(version)]);
}

bool Utils::isDwmCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    static const bool isWin8OrGreater = isWindowsVersionOrGreater(WindowsVersion::_8);
    if (isWin8OrGreater) {
        return true;
    }
    const auto resultFromRegistry = []() -> bool {
        static const QString keyPath = kHKEY_CURRENT_USER + u'\\' + qDwmRegistryKey;
        const QSettings registry(keyPath, QSettings::NativeFormat);
        bool ok = false;
        const DWORD value = registry.value(kComposition).toULongLong(&ok);
        return (ok && (value != 0));
    };
    if (!API_DWM_AVAILABLE(DwmIsCompositionEnabled)) {
        return resultFromRegistry();
    }
    BOOL enabled = FALSE;
    const HRESULT hr = API_CALL_FUNCTION(DwmIsCompositionEnabled, &enabled);
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
    if (!API_DWM_AVAILABLE(DwmExtendFrameIntoClientArea)) {
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
    const HRESULT hr = API_CALL_FUNCTION(DwmExtendFrameIntoClientArea, hwnd, &margins);
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
        static const QString keyPath = kHKEY_CURRENT_USER + u'\\' + qDwmRegistryKey;
        const QSettings registry(keyPath, QSettings::NativeFormat);
        bool ok = false;
        const DWORD value = registry.value(kColorizationColor).toULongLong(&ok);
        return (ok ? QColor::fromRgba(value) : kDefaultDarkGrayColor);
    };
    if (!API_DWM_AVAILABLE(DwmGetColorizationColor)) {
        return resultFromRegistry();
    }
    DWORD color = 0;
    BOOL opaque = FALSE;
    const HRESULT hr = API_CALL_FUNCTION(DwmGetColorizationColor, &color, &opaque);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmGetColorizationColor, hr);
        return resultFromRegistry();
    }
    return QColor::fromRgba(color);
}

DwmColorizationArea Utils::getDwmColorizationArea()
{
    // It's a Win10 only feature. (TO BE VERIFIED)
    static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
    if (!isWin10OrGreater) {
        return DwmColorizationArea::None_;
    }
    static const QString themeKeyPath = kHKEY_CURRENT_USER + u'\\' + qPersonalizeRegistryKey;
    const QSettings themeRegistry(themeKeyPath, QSettings::NativeFormat);
    bool themeOk = false;
    const DWORD themeValue = themeRegistry.value(qDwmColorKeyName).toULongLong(&themeOk);
    static const QString dwmKeyPath = kHKEY_CURRENT_USER + u'\\' + qDwmRegistryKey;
    const QSettings dwmRegistry(dwmKeyPath, QSettings::NativeFormat);
    bool dwmOk = false;
    const DWORD dwmValue = dwmRegistry.value(qDwmColorKeyName).toULongLong(&dwmOk);
    const bool theme = (themeOk && (themeValue != 0));
    const bool dwm = (dwmOk && (dwmValue != 0));
    if (theme && dwm) {
        return DwmColorizationArea::All;
    } else if (theme) {
        return DwmColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return DwmColorizationArea::TitleBar_WindowBorder;
    }
    return DwmColorizationArea::None_;
}

void Utils::showSystemMenu(const WId windowId, const QPoint &pos, const bool selectFirstEntry,
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

    // Tweak the menu items according to the current window status.
    const bool maxOrFull = (IsMaximized(hWnd) || isFullScreen(windowId));
    const bool fixedSize = isWindowFixedSize();
    EnableMenuItem(hMenu, SC_RESTORE, (MF_BYCOMMAND | ((maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    // The first menu item should be selected by default if the menu is brought
    // up by keyboard. I don't know how to pre-select a menu item but it seems
    // highlight can do the job. However, there's an annoying issue if we do
    // this manually: the highlighted menu item is really only highlighted,
    // not selected, so even if the mouse cursor hovers on other menu items
    // or the user navigates to other menu items through keyboard, the original
    // highlight bar will not move accordingly, the OS will generate another
    // highlight bar to indicate the current selected menu item, which will make
    // the menu look kind of weird. Currently I don't know how to fix this issue.
    HiliteMenuItem(hWnd, hMenu, SC_RESTORE, (MF_BYCOMMAND | (selectFirstEntry ? MFS_HILITE : MFS_UNHILITE)));
    EnableMenuItem(hMenu, SC_MOVE, (MF_BYCOMMAND | (!maxOrFull ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_SIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_MINIMIZE, (MF_BYCOMMAND | MFS_ENABLED));
    EnableMenuItem(hMenu, SC_MAXIMIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize) ? MFS_ENABLED : MFS_DISABLED)));
    EnableMenuItem(hMenu, SC_CLOSE, (MF_BYCOMMAND | MFS_ENABLED));

    // The default menu item will appear in bold font. There can only be one default
    // menu item per menu at most. Set the item ID to "UINT_MAX" (or simply "-1")
    // can clear the default item for the given menu.
    SetMenuDefaultItem(hMenu, SC_CLOSE, FALSE);

    // Popup the system menu at the required position.
    const int result = TrackPopupMenu(hMenu, (TPM_RETURNCMD | (QGuiApplication::isRightToLeft()
                            ? TPM_RIGHTALIGN : TPM_LEFTALIGN)), pos.x(), pos.y(), 0, hWnd, nullptr);
    if (result == 0) {
        // The user canceled the menu, no need to continue.
        return;
    }

    // Send the command that the user choses to the corresponding window.
    if (PostMessageW(hWnd, WM_SYSCOMMAND, result, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kPostMessageW);
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
    if (!API_WINMM_AVAILABLE(timeGetDevCaps)) {
        return;
    }
    if (!API_WINMM_AVAILABLE(timeBeginPeriod)) {
        return;
    }
    if (!API_WINMM_AVAILABLE(timeEndPeriod)) {
        return;
    }
    if (!API_DWM_AVAILABLE(DwmGetCompositionTimingInfo)) {
        return;
    }
    // Dirty hack to workaround the resize flicker caused by DWM.
    LARGE_INTEGER freq = {};
    if (QueryPerformanceFrequency(&freq) == FALSE) {
        qWarning() << getSystemErrorMessage(kQueryPerformanceFrequency);
        return;
    }
    TIMECAPS tc = {};
    if (API_CALL_FUNCTION(timeGetDevCaps, &tc, sizeof(tc)) != MMSYSERR_NOERROR) {
        qWarning() << "timeGetDevCaps() failed.";
        return;
    }
    const UINT ms_granularity = tc.wPeriodMin;
    if (API_CALL_FUNCTION(timeBeginPeriod, ms_granularity) != TIMERR_NOERROR) {
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
    const HRESULT hr = API_CALL_FUNCTION(DwmGetCompositionTimingInfo, nullptr, &dti);
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
    if (API_CALL_FUNCTION(timeEndPeriod, ms_granularity) != TIMERR_NOERROR) {
        qWarning() << "timeEndPeriod() failed.";
    }
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
    if (API_SHCORE_AVAILABLE(GetDpiForMonitor)) {
        const HMONITOR monitor = []() -> HMONITOR {
            const HWND window = ensureDummyWindow();
            if (window) {
                return MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
            }
            static constexpr const int kTaskBarSize = 100;
            return MonitorFromPoint(POINT{kTaskBarSize, kTaskBarSize}, MONITOR_DEFAULTTOPRIMARY);
        }();
        if (monitor) {
            UINT dpiX = 0, dpiY = 0;
            const HRESULT hr = API_CALL_FUNCTION(GetDpiForMonitor, monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (SUCCEEDED(hr) && (dpiX > 0) && (dpiY > 0)) {
                return (horizontal ? dpiX : dpiY);
            } else {
                qWarning() << __getSystemErrorMessage(kGetDpiForMonitor, hr);
            }
        } else {
            qWarning() << getSystemErrorMessage(kMonitorFromPoint);
        }
    }
    if (API_D2D_AVAILABLE(D2D1CreateFactory)) {
        using D2D1CreateFactoryPtr =
            HRESULT(WINAPI *)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void **);
        const auto pD2D1CreateFactory =
            reinterpret_cast<D2D1CreateFactoryPtr>(SysApiLoader::instance()->get(kD2D1CreateFactory));
        HumbleComPtr<ID2D1Factory> d2dFactory = nullptr;
        HRESULT hr = pD2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory),
                                        nullptr, reinterpret_cast<void **>(&d2dFactory));
        if (SUCCEEDED(hr)) {
            hr = d2dFactory->ReloadSystemMetrics();
            if (SUCCEEDED(hr)) {
                FLOAT dpiX = 0.0f, dpiY = 0.0f;
                QT_WARNING_PUSH
                QT_WARNING_DISABLE_DEPRECATED
                d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
                QT_WARNING_POP
                if ((dpiX > 0.0f) && (dpiY > 0.0f)) {
                    return (horizontal ? quint32(qRound(dpiX)) : quint32(qRound(dpiY)));
                } else {
                    qWarning() << "GetDesktopDpi() failed.";
                }
            } else {
                qWarning() << __getSystemErrorMessage(kReloadSystemMetrics, hr);
            }
        } else {
            qWarning() << __getSystemErrorMessage(kD2D1CreateFactory, hr);
        }
    }
    const HDC hdc = GetDC(nullptr);
    if (hdc) {
        bool valid = false;
        const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        if ((dpiX > 0) && (dpiY > 0)) {
            valid = true;
        } else {
            qWarning() << getSystemErrorMessage(kGetDeviceCaps);
        }
        if (ReleaseDC(nullptr, hdc) == 0) {
            qWarning() << getSystemErrorMessage(kReleaseDC);
        }
        if (valid) {
            return (horizontal ? dpiX : dpiY);
        }
    } else {
        qWarning() << getSystemErrorMessage(kGetDC);
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
    if (API_USER_AVAILABLE(GetDpiForWindow)) {
        const UINT dpi = API_CALL_FUNCTION(GetDpiForWindow, hwnd);
        if (dpi > 0) {
            return dpi;
        } else {
            qWarning() << getSystemErrorMessage(kGetDpiForWindow);
        }
    }
    if (API_USER_AVAILABLE(GetSystemDpiForProcess)) {
        const UINT dpi = API_CALL_FUNCTION(GetSystemDpiForProcess, GetCurrentProcess());
        if (dpi > 0) {
            return dpi;
        } else {
            qWarning() << getSystemErrorMessage(kGetSystemDpiForProcess);
        }
    }
    if (API_USER_AVAILABLE(GetDpiForSystem)) {
        const UINT dpi = API_CALL_FUNCTION(GetDpiForSystem);
        if (dpi > 0) {
            return dpi;
        } else {
            qWarning() << getSystemErrorMessage(kGetDpiForSystem);
        }
    }
    if (API_SHCORE_AVAILABLE(GetDpiForMonitor)) {
        const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (monitor) {
            UINT dpiX = 0, dpiY = 0;
            const HRESULT hr = API_CALL_FUNCTION(GetDpiForMonitor, monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (SUCCEEDED(hr) && (dpiX > 0) && (dpiY > 0)) {
                return (horizontal ? dpiX : dpiY);
            } else {
                qWarning() << __getSystemErrorMessage(kGetDpiForMonitor, hr);
            }
        } else {
            qWarning() << getSystemErrorMessage(kMonitorFromWindow);
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
    static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
    if (!isWin10OrGreater) {
        return 0;
    }
    if (!API_DWM_AVAILABLE(DwmGetWindowAttribute)) {
        return 0;
    }
    const UINT dpi = getWindowDpi(windowId, true);
    const qreal scaleFactor = (qreal(dpi) / qreal(USER_DEFAULT_SCREEN_DPI));
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    UINT value = 0;
    const HRESULT hr = API_CALL_FUNCTION(DwmGetWindowAttribute, hwnd,
        _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        const qreal dpr = (scaled ? 1.0 : scaleFactor);
        return qRound(qreal(value) / dpr);
    } else {
        const qreal dpr = (scaled ? scaleFactor : 1.0);
        return qRound(qreal(kDefaultWindowFrameBorderThickness) * dpr);
    }
}

QColor Utils::getFrameBorderColor(const bool active)
{
    // There's no window frame border before Windows 10.
    // So we just return a default value which is based on most window managers.
    static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
    if (!isWin10OrGreater) {
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
    static const bool isWin10RS1OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1607);
    if (!isWin10RS1OrGreater) {
        return;
    }
    if (!API_DWM_AVAILABLE(DwmSetWindowAttribute)) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    static const bool isWin1020H1OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_2004);
    const DWORD mode = (isWin1020H1OrGreater ? _DWMWA_USE_IMMERSIVE_DARK_MODE : _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1);
    const BOOL value = (dark ? TRUE : FALSE);
    const HRESULT hr = API_CALL_FUNCTION(DwmSetWindowAttribute, hwnd, mode, &value, sizeof(value));
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
    }
}

void Utils::fixupQtInternals(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    SetLastError(ERROR_SUCCESS);
    const auto oldClassStyle = static_cast<DWORD>(GetClassLongPtrW(hwnd, GCL_STYLE));
    if (oldClassStyle == 0) {
        qWarning() << getSystemErrorMessage(kGetClassLongPtrW);
        return;
    }
    // CS_HREDRAW/CS_VREDRAW will trigger a repaint event when the window size changes
    // horizontally/vertically, which will cause flicker and jitter during window
    // resizing, mostly for the applications which do all the painting by themselves.
    // So we remove these flags from the window class here, Qt by default won't add them
    // but let's make it extra safe.
    const DWORD newClassStyle = (oldClassStyle & ~(CS_HREDRAW | CS_VREDRAW));
    SetLastError(ERROR_SUCCESS);
    if (SetClassLongPtrW(hwnd, GCL_STYLE, static_cast<LONG_PTR>(newClassStyle)) == 0) {
        qWarning() << getSystemErrorMessage(kSetClassLongPtrW);
        return;
    }
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
    // and add the WS_OVERLAPPED flag, unconditionally.
    const DWORD newWindowStyle = ((oldWindowStyle & ~WS_POPUP) | WS_OVERLAPPED);
    SetLastError(ERROR_SUCCESS);
    if (SetWindowLongPtrW(hwnd, GWL_STYLE, static_cast<LONG_PTR>(newWindowStyle)) == 0) {
        qWarning() << getSystemErrorMessage(kSetWindowLongPtrW);
        return;
    }
    triggerFrameChange(windowId);
}

void Utils::startSystemMove(QWindow *window, const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    if (ReleaseCapture() == FALSE) {
        qWarning() << getSystemErrorMessage(kReleaseCapture);
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, 0xF012 /*SC_DRAGMOVE*/, 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kPostMessageW);
    }
#endif
}

void Utils::startSystemResize(QWindow *window, const Qt::Edges edges, const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemResize(edges);
#else
    if (ReleaseCapture() == FALSE) {
        qWarning() << getSystemErrorMessage(kReleaseCapture);
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (PostMessageW(hwnd, WM_SYSCOMMAND, qtEdgesToWin32Orientation(edges), 0) == FALSE) {
        qWarning() << getSystemErrorMessage(kPostMessageW);
    }
#endif
}

bool Utils::isWindowFrameBorderVisible()
{
    static const bool result = []() -> bool {
        const FramelessConfig * const config = FramelessConfig::instance();
        if (config->isSet(Option::ForceShowWindowFrameBorder)) {
            return true;
        }
        if (config->isSet(Option::ForceHideWindowFrameBorder)) {
            return false;
        }
        static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
        return isWin10OrGreater;
    }();
    return result;
}

bool Utils::isTitleBarColorized()
{
    // CHECK: is it supported on win7?
    static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
    if (!isWin10OrGreater) {
        return false;
    }
    const DwmColorizationArea area = getDwmColorizationArea();
    return ((area == DwmColorizationArea::TitleBar_WindowBorder) || (area == DwmColorizationArea::All));
}

bool Utils::isFrameBorderColorized()
{
    return isTitleBarColorized();
}

void Utils::installSystemMenuHook(const WId windowId,
                                  const IsWindowFixedSizeCallback &isWindowFixedSize,
                                  const IsInsideTitleBarDraggableAreaCallback &isInTitleBarArea,
                                  const GetWindowDevicePixelRatioCallback &getDevicePixelRatio)
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
    //triggerFrameChange(windowId); // Crash
    Win32UtilsHelperData data = {};
    data.originalWindowProc = originalWindowProc;
    data.isWindowFixedSize = isWindowFixedSize;
    data.isInTitleBarArea = isInTitleBarArea;
    data.getDevicePixelRatio = getDevicePixelRatio;
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
    //triggerFrameChange(windowId); // Crash
    g_utilsHelper()->data.remove(windowId);
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
    // The trick is to restore the window style. Qt mainly adds the "WS_EX_LAYERED"
    // flag to the extended window style.
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
    // The key is the existence of the "WS_THICKFRAME" flag.
    // But we should also disallow window maximize if Aero Snapping is disabled.
    static constexpr const DWORD resizableFlags = (WS_THICKFRAME | WS_MAXIMIZEBOX);
    const DWORD newWindowStyle = [enable, oldWindowStyle]() -> DWORD {
        if (enable) {
            return ((oldWindowStyle & ~WS_POPUP) | resizableFlags);
        } else {
            return ((oldWindowStyle & ~resizableFlags) | WS_POPUP);
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
    if (API_USER_AVAILABLE(SetProcessDpiAwarenessContext)) {
        const auto SetProcessDpiAwarenessContext2 = [](const DPI_AWARENESS_CONTEXT context) -> bool {
            Q_ASSERT(context);
            if (!context) {
                return false;
            }
            if (API_CALL_FUNCTION(SetProcessDpiAwarenessContext, context) != FALSE) {
                return true;
            }
            const DWORD dwError = GetLastError();
            // "ERROR_ACCESS_DENIED" means set externally (mostly due to manifest file).
            // Any attempt to change the DPI awareness level through API will always fail,
            // so we treat this situation as succeeded.
            if (dwError == ERROR_ACCESS_DENIED) {
                qDebug() << "FramelessHelper doesn't have access to change the current process's DPI awareness level,"
                            " most likely due to it has been set externally already.";
                return true;
            }
            qWarning() << __getSystemErrorMessage(kSetProcessDpiAwarenessContext, dwError);
            return false;
        };
        if (SetProcessDpiAwarenessContext2(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            return;
        }
        if (SetProcessDpiAwarenessContext2(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
            return;
        }
        if (SetProcessDpiAwarenessContext2(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE)) {
            return;
        }
        if (SetProcessDpiAwarenessContext2(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED)) {
            return;
        }
    }
    if (API_SHCORE_AVAILABLE(SetProcessDpiAwareness)) {
        const auto SetProcessDpiAwareness2 = [](const PROCESS_DPI_AWARENESS pda) -> bool {
            const HRESULT hr = API_CALL_FUNCTION(SetProcessDpiAwareness, pda);
            if (SUCCEEDED(hr)) {
                return true;
            }
            // "E_ACCESSDENIED" means set externally (mostly due to manifest file).
            // Any attempt to change the DPI awareness level through API will always fail,
            // so we treat this situation as succeeded.
            if (hr == E_ACCESSDENIED) {
                qDebug() << "FramelessHelper doesn't have access to change the current process's DPI awareness level,"
                            " most likely due to it has been set externally already.";
                return true;
            }
            qWarning() << __getSystemErrorMessage(kSetProcessDpiAwareness, hr);
            return false;
        };
        if (SetProcessDpiAwareness2(PROCESS_PER_MONITOR_DPI_AWARE_V2)) {
            return;
        }
        if (SetProcessDpiAwareness2(PROCESS_PER_MONITOR_DPI_AWARE)) {
            return;
        }
        if (SetProcessDpiAwareness2(PROCESS_SYSTEM_DPI_AWARE)) {
            return;
        }
    }
    // Some really old MinGW SDK may lack this function, we workaround this
    // issue by always load it dynamically at runtime.
    if (API_USER_AVAILABLE(SetProcessDPIAware)) {
        if (API_CALL_FUNCTION(SetProcessDPIAware) == FALSE) {
            qWarning() << getSystemErrorMessage(kSetProcessDPIAware);
        }
    }
}

SystemTheme Utils::getSystemTheme()
{
    if (isHighContrastModeEnabled()) {
        return SystemTheme::HighContrast;
    }
    static const bool isWin10RS1OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1607);
    if (isWin10RS1OrGreater && shouldAppsUseDarkMode()) {
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
    static const bool isWin10RS5OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1809);
    if (!isWin10RS5OrGreater) {
        return;
    }
    if (!API_THEME_AVAILABLE(SetWindowTheme)) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    const HRESULT hr = API_CALL_FUNCTION(SetWindowTheme, hwnd,
        (dark ? kSystemDarkThemeResourceName : kSystemLightThemeResourceName), nullptr);
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kSetWindowTheme, hr);
    }
}

bool Utils::shouldAppsUseDarkMode_windows()
{
    // The global dark mode was first introduced in Windows 10 1607.
    static const bool isWin10RS1OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1607);
    if (!isWin10RS1OrGreater) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        static const QString keyPath = kHKEY_CURRENT_USER + u'\\' + qPersonalizeRegistryKey;
        const QSettings registry(keyPath, QSettings::NativeFormat);
        bool ok = false;
        const DWORD value = registry.value(kAppsUseLightTheme).toULongLong(&ok);
        return (ok && (value == 0));
    };
    // Starting from Windows 10 1903, "ShouldAppsUseDarkMode()" (exported by UXTHEME.DLL,
    // ordinal number 132) always return "TRUE" (actually, a random non-zero number at
    // runtime), so we can't use it due to this unreliability. In this case, we just simply
    // read the user's setting from the registry instead, it's not elegant but at least
    // it works well.
    // However, reverse engineering of Win11's Task Manager reveals that Microsoft still
    // uses this function internally to determine the system theme, and the Task Manager
    // can correctly respond to the theme change message indeed. Is it fixed silently
    // in some unknown Windows versions? To be checked.
    return resultFromRegistry();
}

void Utils::forceSquareCornersForWindow(const WId windowId, const bool force)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    // We cannot change the window corner style until Windows 11.
    static const bool isWin11OrGreater = isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
    if (!isWin11OrGreater) {
        return;
    }
    if (!API_DWM_AVAILABLE(DwmSetWindowAttribute)) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    const _DWM_WINDOW_CORNER_PREFERENCE wcp = (force ? _DWMWCP_DONOTROUND : _DWMWCP_ROUND);
    const HRESULT hr = API_CALL_FUNCTION(DwmSetWindowAttribute, hwnd, _DWMWA_WINDOW_CORNER_PREFERENCE, &wcp, sizeof(wcp));
    if (FAILED(hr)) {
        qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
    }
}

bool Utils::setBlurBehindWindowEnabled(const WId windowId, const BlurMode mode, const QColor &color)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(windowId);
    static const bool isWin8OrGreater = isWindowsVersionOrGreater(WindowsVersion::_8);
    if (isWin8OrGreater) {
        if (!API_USER_AVAILABLE(SetWindowCompositionAttribute)) {
            return false;
        }
        if (!API_DWM_AVAILABLE(DwmSetWindowAttribute)) {
            return false;
        }
        if (!API_DWM_AVAILABLE(DwmExtendFrameIntoClientArea)) {
            return false;
        }
        const auto pSetWindowCompositionAttribute =
            reinterpret_cast<SetWindowCompositionAttributePtr>(
                SysApiLoader::instance()->get(kSetWindowCompositionAttribute));
        static const bool isBuild22523OrGreater = doCompareWindowsVersion({10, 0, 22523});
        static const bool isWin11OrGreater = isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
        static const bool isWin10OrGreater = isWindowsVersionOrGreater(WindowsVersion::_10_1507);
        const BlurMode blurMode = [mode]() -> BlurMode {
            if ((mode == BlurMode::Disable) || (mode == BlurMode::Windows_Aero)) {
                return mode;
            }
            if ((mode == BlurMode::Windows_Mica) && !isWin11OrGreater) {
                qWarning() << "The Mica material is not supported on your system, fallback to the Acrylic blur instead...";
                if (isWin10OrGreater) {
                    return BlurMode::Windows_Acrylic;
                }
                qWarning() << "The Acrylic blur is not supported on your system, fallback to the traditional DWM blur instead...";
                return BlurMode::Windows_Aero;
            }
            if ((mode == BlurMode::Windows_Acrylic) && !isWin10OrGreater) {
                qWarning() << "The Acrylic blur is not supported on your system, fallback to the traditional DWM blur instead...";
                return BlurMode::Windows_Aero;
            }
            if (mode == BlurMode::Default) {
                if (isWin11OrGreater) {
                    return BlurMode::Windows_Mica;
                }
                if (isWin10OrGreater) {
                    return BlurMode::Windows_Acrylic;
                }
                return BlurMode::Windows_Aero;
            }
            Q_ASSERT(false); // Really should NOT go here.
            return mode;
        }();
        if (blurMode == BlurMode::Disable) {
            if (isBuild22523OrGreater) {
                const _DWM_SYSTEMBACKDROP_TYPE dwmsbt = _DWMSBT_NONE;
                const HRESULT hr = API_CALL_FUNCTION(DwmSetWindowAttribute,
                    hwnd, _DWMWA_SYSTEMBACKDROP_TYPE, &dwmsbt, sizeof(dwmsbt));
                if (FAILED(hr)) {
                    qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
                }
            }
            if (isWin11OrGreater) {
                const BOOL enable = FALSE;
                HRESULT hr = API_CALL_FUNCTION(DwmSetWindowAttribute,
                    hwnd, _DWMWA_MICA_EFFECT, &enable, sizeof(enable));
                if (FAILED(hr)) {
                    qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
                }
                const MARGINS margins = {0, 0, 0, 0};
                hr = API_CALL_FUNCTION(DwmExtendFrameIntoClientArea, hwnd, &margins);
                if (FAILED(hr)) {
                    qWarning() << __getSystemErrorMessage(kDwmExtendFrameIntoClientArea, hr);
                }
            }
            ACCENT_POLICY policy;
            SecureZeroMemory(&policy, sizeof(policy));
            policy.State = ACCENT_DISABLED;
            WINDOWCOMPOSITIONATTRIBDATA wcad;
            SecureZeroMemory(&wcad, sizeof(wcad));
            wcad.Attrib = WCA_ACCENT_POLICY;
            wcad.pvData = &policy;
            wcad.cbData = sizeof(policy);
            if (pSetWindowCompositionAttribute(hwnd, &wcad) == FALSE) {
                qWarning() << getSystemErrorMessage(kSetWindowCompositionAttribute);
            }
            return true;
        } else {
            if (blurMode == BlurMode::Windows_Mica) {
                const MARGINS margins = {-1, -1, -1, -1};
                HRESULT hr = API_CALL_FUNCTION(DwmExtendFrameIntoClientArea, hwnd, &margins);
                if (SUCCEEDED(hr)) {
                    if (isBuild22523OrGreater) {
                        // ### FIXME: Is it necessary to enable the host backdrop brush in the first place? To be checked.
                        const BOOL enable = TRUE;
                        hr = API_CALL_FUNCTION(DwmSetWindowAttribute, hwnd,
                            _DWMWA_USE_HOSTBACKDROPBRUSH, &enable, sizeof(enable));
                        if (SUCCEEDED(hr)) {
                            const _DWM_SYSTEMBACKDROP_TYPE dwmsbt = _DWMSBT_MAINWINDOW; // Mica
                            hr = API_CALL_FUNCTION(DwmSetWindowAttribute, hwnd,
                                _DWMWA_SYSTEMBACKDROP_TYPE, &dwmsbt, sizeof(dwmsbt));
                            if (SUCCEEDED(hr)) {
                                return true;
                            } else {
                                qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
                            }
                        } else {
                            qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
                        }
                    } else {
                        const BOOL enable = TRUE;
                        hr = API_CALL_FUNCTION(DwmSetWindowAttribute,
                            hwnd, _DWMWA_MICA_EFFECT, &enable, sizeof(enable));
                        if (SUCCEEDED(hr)) {
                            return true;
                        } else {
                            qWarning() << __getSystemErrorMessage(kDwmSetWindowAttribute, hr);
                        }
                    }
                } else {
                    qWarning() << __getSystemErrorMessage(kDwmExtendFrameIntoClientArea, hr);
                }
            } else {
                ACCENT_POLICY policy;
                SecureZeroMemory(&policy, sizeof(policy));
                if (blurMode == BlurMode::Windows_Acrylic) {
                    policy.State = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                    policy.Flags = 2; // Magic number, this member must be set to 2.
                    const QColor gradientColor = [&color]() -> QColor {
                        if (color.isValid()) {
                            return color;
                        }
                        QColor clr = (shouldAppsUseDarkMode() ? kDefaultSystemDarkColor : kDefaultSystemLightColor);
                        clr.setAlpha(230); // 90% opacity.
                        return clr;
                    }();
                    // This API expects the #AABBGGRR format.
                    policy.GradientColor = DWORD(qRgba(gradientColor.blue(),
                        gradientColor.green(), gradientColor.red(), gradientColor.alpha()));
                } else if (blurMode == BlurMode::Windows_Aero) {
                    policy.State = ACCENT_ENABLE_BLURBEHIND;
                } else {
                    Q_ASSERT(false); // Really should NOT go here.
                }
                WINDOWCOMPOSITIONATTRIBDATA wcad;
                SecureZeroMemory(&wcad, sizeof(wcad));
                wcad.Attrib = WCA_ACCENT_POLICY;
                wcad.pvData = &policy;
                wcad.cbData = sizeof(policy);
                if (pSetWindowCompositionAttribute(hwnd, &wcad) != FALSE) {
                    if (!isWin11OrGreater) {
                        qDebug() << "Enabling the Acrylic blur for Win32 windows on Windows 10 "
                                    "is very buggy. The only recommended way by Microsoft is to "
                                    "use the XAML Island technology or use pure UWP instead. If "
                                    "you find your window becomes very laggy during moving and "
                                    "resizing, please disable the Acrylic blur immediately.";
                    }
                    return true;
                }
                qWarning() << getSystemErrorMessage(kSetWindowCompositionAttribute);
            }
        }
    } else {
        // We prefer to use "DwmEnableBlurBehindWindow" on Windows 7 because it behaves
        // better than the undocumented API.
        if (API_DWM_AVAILABLE(DwmEnableBlurBehindWindow)) {
            DWM_BLURBEHIND dwmbb;
            SecureZeroMemory(&dwmbb, sizeof(dwmbb));
            dwmbb.dwFlags = DWM_BB_ENABLE;
            dwmbb.fEnable = [mode]() -> BOOL {
                if (mode == BlurMode::Disable) {
                    return FALSE;
                }
                if ((mode != BlurMode::Default) && (mode != BlurMode::Windows_Aero)) {
                    qWarning() << "The only supported blur mode on Windows 7 is the traditional DWM blur.";
                }
                return TRUE;
            }();
            const HRESULT hr = API_CALL_FUNCTION(DwmEnableBlurBehindWindow, hwnd, &dwmbb);
            if (SUCCEEDED(hr)) {
                return true;
            }
            qWarning() << __getSystemErrorMessage(kDwmEnableBlurBehindWindow, hr);
        }
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
