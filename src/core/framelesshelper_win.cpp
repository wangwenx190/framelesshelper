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

#include "framelesshelper_win.h"
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qtimer.h>
#include <QtGui/qwindow.h>
#include "framelessmanager.h"
#include "framelessmanager_p.h"
#include "framelessconfig_p.h"
#include "utils.h"
#include "winverhelper_p.h"
#include "framelesshelper_windows.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessHelperWin, "wangwenx190.framelesshelper.core.impl.win")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessHelperWin)
#  define DEBUG qCDebug(lcFramelessHelperWin)
#  define WARNING qCWarning(lcFramelessHelperWin)
#  define CRITICAL qCCritical(lcFramelessHelperWin)
#endif

using namespace Global;

[[maybe_unused]] static constexpr const wchar_t kFallbackTitleBarWindowClassName[] =
    L"org.wangwenx190.FramelessHelper.FallbackTitleBarWindow\0";
FRAMELESSHELPER_BYTEARRAY_CONSTANT2(Win32MessageTypeName, "windows_generic_MSG")
FRAMELESSHELPER_STRING_CONSTANT(MonitorFromWindow)
FRAMELESSHELPER_STRING_CONSTANT(GetMonitorInfoW)
FRAMELESSHELPER_STRING_CONSTANT(ScreenToClient)
FRAMELESSHELPER_STRING_CONSTANT(ClientToScreen)
FRAMELESSHELPER_STRING_CONSTANT(GetClientRect)
#ifdef Q_PROCESSOR_X86_64
  FRAMELESSHELPER_STRING_CONSTANT(GetWindowLongPtrW)
  FRAMELESSHELPER_STRING_CONSTANT(SetWindowLongPtrW)
#else // Q_PROCESSOR_X86_64
  // WinUser.h defines G/SetClassLongPtr as G/SetClassLong due to the
  // "Ptr" suffixed APIs are not available on 32-bit platforms, so we
  // have to add the following workaround. Undefine the macros and then
  // redefine them is also an option but the following solution is more simple.
  FRAMELESSHELPER_STRING_CONSTANT2(GetWindowLongPtrW, "GetWindowLongW")
  FRAMELESSHELPER_STRING_CONSTANT2(SetWindowLongPtrW, "SetWindowLongW")
#endif // Q_PROCESSOR_X86_64
FRAMELESSHELPER_STRING_CONSTANT(RegisterClassExW)
FRAMELESSHELPER_STRING_CONSTANT(GetModuleHandleW)
FRAMELESSHELPER_STRING_CONSTANT(CreateWindowExW)
FRAMELESSHELPER_STRING_CONSTANT(SetLayeredWindowAttributes)
FRAMELESSHELPER_STRING_CONSTANT(SetWindowPos)
FRAMELESSHELPER_STRING_CONSTANT(TrackMouseEvent)
FRAMELESSHELPER_STRING_CONSTANT(FindWindowW)
FRAMELESSHELPER_STRING_CONSTANT(UnregisterClassW)
FRAMELESSHELPER_STRING_CONSTANT(DestroyWindow)
[[maybe_unused]] static constexpr const char kFallbackTitleBarErrorMessage[] =
    "FramelessHelper is unable to create the fallback title bar window, and thus the snap layout feature will be disabled"
    " unconditionally. You can ignore this error and continue running your application, nothing else will be affected, "
    "no need to worry. But if you really need the snap layout feature, please add a manifest file to your application and "
    "explicitly declare Windows 11 compatibility in it. If you just want to hide this error message, please use the "
    "FramelessConfig class to officially disable the snap layout feature for Windows 11.";
[[maybe_unused]] static constexpr const char kD3DWorkaroundEnvVar[] = "FRAMELESSHELPER_USE_D3D_WORKAROUND";

struct Win32HelperData
{
    SystemParameters params = {};
    bool trackingMouse = false;
    WId fallbackTitleBarWindowId = 0;
    Dpi dpi = {};
};

struct Win32Helper
{
    QMutex mutex;
    QScopedPointer<FramelessHelperWin> nativeEventFilter;
    QHash<WId, Win32HelperData> data = {};
    QHash<WId, WId> fallbackTitleBarToParentWindowMapping = {};
};

Q_GLOBAL_STATIC(Win32Helper, g_win32Helper)

[[nodiscard]] static inline QString hwnd2str(const WId windowId)
{
    // NULL handle is allowed here.
    return FRAMELESSHELPER_STRING_LITERAL("0x")
        + QString::number(windowId, 16).toUpper();
}

[[nodiscard]] static inline QString hwnd2str(const HWND hwnd)
{
    // NULL handle is allowed here.
    return hwnd2str(reinterpret_cast<WId>(hwnd));
}

[[nodiscard]] static inline LRESULT CALLBACK FallbackTitleBarWindowProc
    (const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    Q_ASSERT(hWnd);
    if (!hWnd) {
        return 0;
    }
    // WM_QUIT won't be posted to the WindowProc function.
    if (uMsg == WM_CLOSE) {
        if (DestroyWindow(hWnd) == FALSE) {
            WARNING << Utils::getSystemErrorMessage(kDestroyWindow);
        }
        return 0;
    }
    if (uMsg == WM_DESTROY) {
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    const auto windowId = reinterpret_cast<WId>(hWnd);
    g_win32Helper()->mutex.lock();
    if (!g_win32Helper()->fallbackTitleBarToParentWindowMapping.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    const WId parentWindowId = g_win32Helper()->fallbackTitleBarToParentWindowMapping.value(windowId);
    if (!g_win32Helper()->data.contains(parentWindowId)) {
        g_win32Helper()->mutex.unlock();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    const Win32HelperData data = g_win32Helper()->data.value(parentWindowId);
    g_win32Helper()->mutex.unlock();
    const auto parentWindowHandle = reinterpret_cast<HWND>(parentWindowId);
    // All mouse events: client area mouse events + non-client area mouse events.
    // Hit-testing event should not be considered as a mouse event.
    const bool isMouseEvent = (((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST)) ||
          ((uMsg >= WM_NCMOUSEMOVE) && (uMsg <= WM_NCXBUTTONDBLCLK)));
    const auto releaseButtons = [&data](const std::optional<SystemButtonType> exclude) -> void {
        static constexpr const auto defaultButtonState = ButtonState::Unspecified;
        const SystemButtonType button = exclude.value_or(SystemButtonType::Unknown);
        if (button != SystemButtonType::WindowIcon) {
            data.params.setSystemButtonState(SystemButtonType::WindowIcon, defaultButtonState);
        }
        if (button != SystemButtonType::Help) {
            data.params.setSystemButtonState(SystemButtonType::Help, defaultButtonState);
        }
        if (button != SystemButtonType::Minimize) {
            data.params.setSystemButtonState(SystemButtonType::Minimize, defaultButtonState);
        }
        if (button != SystemButtonType::Maximize) {
            data.params.setSystemButtonState(SystemButtonType::Maximize, defaultButtonState);
        }
        if (button != SystemButtonType::Restore) {
            data.params.setSystemButtonState(SystemButtonType::Restore, defaultButtonState);
        }
        if (button != SystemButtonType::Close) {
            data.params.setSystemButtonState(SystemButtonType::Close, defaultButtonState);
        }
    };
    const auto hoverButton = [&releaseButtons, &data](const SystemButtonType button) -> void {
        releaseButtons(button);
        data.params.setSystemButtonState(button, ButtonState::Hovered);
    };
    const auto pressButton = [&releaseButtons, &data](const SystemButtonType button) -> void {
        releaseButtons(button);
        data.params.setSystemButtonState(button, ButtonState::Pressed);
    };
    const auto clickButton = [&releaseButtons, &data](const SystemButtonType button) -> void {
        releaseButtons(button);
        data.params.setSystemButtonState(button, ButtonState::Clicked);
    };
    switch (uMsg) {
    case WM_NCHITTEST: {
        // Try to determine what part of the window is being hovered here. This
        // is absolutely critical to making sure the snap layout works!
        const POINT nativeGlobalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT nativeLocalPos = nativeGlobalPos;
        if (ScreenToClient(hWnd, &nativeLocalPos) == FALSE) {
            WARNING << Utils::getSystemErrorMessage(kScreenToClient);
            break;
        }
        const qreal devicePixelRatio = data.params.getWindowDevicePixelRatio();
        const QPoint qtScenePos = QPointF(QPointF(qreal(nativeLocalPos.x), qreal(nativeLocalPos.y)) / devicePixelRatio).toPoint();
        SystemButtonType buttonType = SystemButtonType::Unknown;
        if (data.params.isInsideSystemButtons(qtScenePos, &buttonType)) {
            switch (buttonType) {
            case SystemButtonType::Unknown:
                Q_ASSERT(false);
                break;
            case SystemButtonType::WindowIcon:
                return HTSYSMENU;
            case SystemButtonType::Help:
                return HTHELP;
            case SystemButtonType::Minimize:
                return HTREDUCE;
            case SystemButtonType::Maximize:
            case SystemButtonType::Restore:
                return HTZOOM;
            case SystemButtonType::Close:
                return HTCLOSE;
            }
        }
        // Returns "HTTRANSPARENT" to let the mouse event pass through this invisible
        // window to the parent window beneath it, otherwise all the controls under it
        // can't be hovered.
        return HTTRANSPARENT;
    }
    case WM_NCMOUSEMOVE: {
        // When we get this message, it's because the mouse moved when it was
        // over somewhere we said was the non-client area.
        //
        // We'll use this to communicate state to the title bar control, so that
        // it can update its visuals.
        // - If we're over a button, hover it.
        // - If we're over _anything else_, stop hovering the buttons.
        switch (wParam) {
        case HTTOP:
        case HTCAPTION: {
            releaseButtons(std::nullopt);
            // Pass caption-related nonclient messages to the parent window.
            // Make sure to do this for the HTTOP, which is the top resize
            // border, so we can resize the window on the top.
            return SendMessageW(parentWindowHandle, uMsg, wParam, lParam);
        }
        case HTSYSMENU:
            hoverButton(SystemButtonType::WindowIcon);
            break;
        case HTHELP:
            hoverButton(SystemButtonType::Help);
            break;
        case HTREDUCE:
            hoverButton(SystemButtonType::Minimize);
            break;
        case HTZOOM:
            hoverButton(SystemButtonType::Maximize);
            break;
        case HTCLOSE:
            hoverButton(SystemButtonType::Close);
            break;
        default:
            releaseButtons(std::nullopt);
            break;
        }
        // If we haven't previously asked for mouse tracking, request mouse
        // tracking. We need to do this so we can get the WM_NCMOUSELEAVE
        // message when the mouse leave the title bar. Otherwise, we won't always
        // get that message (especially if the user moves the mouse _real
        // fast_).
        if (!data.trackingMouse && ((wParam == HTSYSMENU) || (wParam == HTHELP)
               || (wParam == HTREDUCE) || (wParam == HTZOOM) || (wParam == HTCLOSE))) {
            TRACKMOUSEEVENT tme;
            SecureZeroMemory(&tme, sizeof(tme));
            tme.cbSize = sizeof(tme);
            // TME_NONCLIENT is absolutely critical here. In my experimentation,
            // we'd get WM_MOUSELEAVE messages after just a HOVER_DEFAULT
            // timeout even though we're not requesting TME_HOVER, which kinda
            // ruined the whole point of this.
            tme.dwFlags = (TME_LEAVE | TME_NONCLIENT);
            tme.hwndTrack = hWnd;
            tme.dwHoverTime = HOVER_DEFAULT; // We don't _really_ care about this.
            if (TrackMouseEvent(&tme) == FALSE) {
                WARNING << Utils::getSystemErrorMessage(kTrackMouseEvent);
                break;
            }
            const QMutexLocker locker(&g_win32Helper()->mutex);
            g_win32Helper()->data[parentWindowId].trackingMouse = true;
        }
    } break;
    case WM_NCMOUSELEAVE:
    case WM_MOUSELEAVE: {
        // When the mouse leaves the drag rect, make sure to dismiss any hover.
        releaseButtons(std::nullopt);
        const QMutexLocker locker(&g_win32Helper()->mutex);
        g_win32Helper()->data[parentWindowId].trackingMouse = false;
    } break;
    // NB: *Shouldn't be forwarding these* when they're not over the caption
    // because they can inadvertently take action using the system's default
    // metrics instead of our own.
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK: {
        // Manual handling for mouse clicks in the fallback title bar. If it's in a
        // caption button, then tell the title bar to "press" the button, which
        // should change its visual state.
        //
        // If it's not in a caption button, then just forward the message along
        // to the root HWND. Make sure to do this for the HTTOP, which is the
        // top resize border.
        switch (wParam) {
        case HTTOP:
        case HTCAPTION:
            // Pass caption-related nonclient messages to the parent window.
            return SendMessageW(parentWindowHandle, uMsg, wParam, lParam);
        // The buttons won't work as you'd expect; we need to handle those
        // ourselves.
        case HTSYSMENU:
            pressButton(SystemButtonType::WindowIcon);
            break;
        case HTHELP:
            pressButton(SystemButtonType::Help);
            break;
        case HTREDUCE:
            pressButton(SystemButtonType::Minimize);
            break;
        case HTZOOM:
            pressButton(SystemButtonType::Maximize);
            break;
        case HTCLOSE:
            pressButton(SystemButtonType::Close);
            break;
        default:
            break;
        }
        return 0;
    }
    case WM_NCLBUTTONUP: {
        // Manual handling for mouse RELEASES in the fallback title bar. If it's in a
        // caption button, then manually handle what we'd expect for that button.
        //
        // If it's not in a caption button, then just forward the message along
        // to the root HWND.
        switch (wParam) {
        case HTTOP:
        case HTCAPTION:
            // Pass caption-related nonclient messages to the parent window.
            return SendMessageW(parentWindowHandle, uMsg, wParam, lParam);
        // The buttons won't work as you'd expect; we need to handle those ourselves.
        case HTSYSMENU:
            clickButton(SystemButtonType::WindowIcon);
            break;
        case HTHELP:
            clickButton(SystemButtonType::Help);
            break;
        case HTREDUCE:
            clickButton(SystemButtonType::Minimize);
            break;
        case HTZOOM:
            clickButton(SystemButtonType::Maximize);
            break;
        case HTCLOSE:
            clickButton(SystemButtonType::Close);
            break;
        default:
            break;
        }
        return 0;
    }
    // Make sure to pass along right-clicks in this region to our parent window
    // - we don't need to handle these.
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCRBUTTONUP:
        return SendMessageW(parentWindowHandle, uMsg, wParam, lParam);
    default:
        break;
    }
    // Forward all the mouse events we don't handle here to the parent window,
    // this is a necessary step to make sure the child widgets/quick items can still
    // receive mouse events from our homemade title bar.
    if (isMouseEvent) {
        SendMessageW(parentWindowHandle, uMsg, wParam, lParam);
        return 0; // There's nothing to do in this invisible window, so ignore it.
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline bool resizeFallbackTitleBarWindow
    (const WId parentWindowId, const WId fallbackTitleBarWindowId, const bool hide)
{
    Q_ASSERT(parentWindowId);
    Q_ASSERT(fallbackTitleBarWindowId);
    if (!parentWindowId || !fallbackTitleBarWindowId) {
        return false;
    }
    const auto parentWindowHandle = reinterpret_cast<HWND>(parentWindowId);
    RECT parentWindowClientRect = {};
    if (GetClientRect(parentWindowHandle, &parentWindowClientRect) == FALSE) {
        WARNING << Utils::getSystemErrorMessage(kGetClientRect);
        return false;
    }
    const int titleBarHeight = Utils::getTitleBarHeight(parentWindowId, true);
    const auto fallbackTitleBarWindowHandle = reinterpret_cast<HWND>(fallbackTitleBarWindowId);
    const UINT flags = (SWP_NOACTIVATE | (hide ? SWP_HIDEWINDOW : SWP_SHOWWINDOW));
    // As you can see from the code, we only use the fallback title bar window to activate the
    // snap layout feature introduced in Windows 11. So you may wonder, why not just
    // limit it to the area of the three system buttons, instead of covering the
    // whole title bar area? Well, I've tried that solution already and unfortunately
    // it doesn't work. And according to my experiment, it won't work either even if we
    // only reduce the window width for some pixels. So we have to make it expand to the
    // full width of the parent window to let it occupy the whole top area, and this time
    // it finally works. Since our current solution works well, I have no interest in digging
    // into all the magic behind it.
    if (SetWindowPos(fallbackTitleBarWindowHandle, HWND_TOP, 0, 0,
            parentWindowClientRect.right, titleBarHeight, flags) == FALSE) {
        WARNING << Utils::getSystemErrorMessage(kSetWindowPos);
        return false;
    }
    return true;
}

[[nodiscard]] static inline bool createFallbackTitleBarWindow(const WId parentWindowId, const bool hide)
{
    Q_ASSERT(parentWindowId);
    if (!parentWindowId) {
        return false;
    }
    if (!WindowsVersionHelper::isWin10OrGreater()) {
        WARNING << "The fallback title bar window is only supported on Windows 10 and onwards.";
        return false;
    }
    const auto parentWindowHandle = reinterpret_cast<HWND>(parentWindowId);
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    Q_ASSERT(instance);
    if (!instance) {
        WARNING << Utils::getSystemErrorMessage(kGetModuleHandleW);
        return false;
    }
    static const auto fallbackTitleBarWindowClass = [instance]() -> bool {
        WNDCLASSEXW wcex = {};
        // First try to find out if we have registered the window class already.
        if (GetClassInfoExW(instance, kFallbackTitleBarWindowClassName, &wcex) != FALSE) {
            // Register the same window class for multiple times will fail.
            return true;
        }
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        // The "CS_DBLCLKS" style is necessary, don't remove it!
        wcex.style = CS_DBLCLKS;
        wcex.lpszClassName = kFallbackTitleBarWindowClassName;
        wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.lpfnWndProc = FallbackTitleBarWindowProc;
        wcex.hInstance = instance;
        if (RegisterClassExW(&wcex) != INVALID_ATOM) {
            FramelessHelper::Core::registerUninitializeHook([](){
                const HINSTANCE instance = GetModuleHandleW(nullptr);
                if (!instance) {
                    //WARNING << Utils::getSystemErrorMessage(kGetModuleHandleW);
                    return;
                }
                if (UnregisterClassW(kFallbackTitleBarWindowClassName, instance) == FALSE) {
                    //WARNING << Utils::getSystemErrorMessage(kUnregisterClassW);
                }
            });
            return true;
        }
        WARNING << Utils::getSystemErrorMessage(kRegisterClassExW);
        return false;
    }();
    Q_ASSERT(fallbackTitleBarWindowClass);
    if (!fallbackTitleBarWindowClass) {
        WARNING << "Failed to register the window class for the fallback title bar window.";
        return false;
    }
    const HWND fallbackTitleBarWindowHandle = CreateWindowExW((WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP),
                  kFallbackTitleBarWindowClassName, nullptr, WS_CHILD, 0, 0, 0, 0,
                  parentWindowHandle, nullptr, instance, nullptr);
    // Some users reported that when using MinGW, the following assert won't trigger any
    // message box and will just crash, that's absolutely not what we would want to see.
    // And many users think they encountered FramelessHelper bugs when they get the assert
    // error, so let's just remove this assert anyway. It is meant to give the user some
    // hint about how to use the snap layout feature, but now it seems things are going
    // to a wrong direction instead.
    //Q_ASSERT_X(fallbackTitleBarWindowHandle, __FUNCTION__, kFallbackTitleBarErrorMessage);
    if (!fallbackTitleBarWindowHandle) {
        WARNING << Utils::getSystemErrorMessage(kCreateWindowExW);
        WARNING << kFallbackTitleBarErrorMessage;
        return false;
    }
    // Layered windows won't become visible unless we call the SetLayeredWindowAttributes()
    // or UpdateLayeredWindow() function at least once.
    if (SetLayeredWindowAttributes(fallbackTitleBarWindowHandle, 0, 255, LWA_ALPHA) == FALSE) {
        WARNING << Utils::getSystemErrorMessage(kSetLayeredWindowAttributes);
        return false;
    }
    const auto fallbackTitleBarWindowId = reinterpret_cast<WId>(fallbackTitleBarWindowHandle);
    if (!resizeFallbackTitleBarWindow(parentWindowId, fallbackTitleBarWindowId, hide)) {
        WARNING << "Failed to re-position the fallback title bar window.";
        return false;
    }
    const QMutexLocker locker(&g_win32Helper()->mutex);
    g_win32Helper()->data[parentWindowId].fallbackTitleBarWindowId = fallbackTitleBarWindowId;
    g_win32Helper()->fallbackTitleBarToParentWindowMapping.insert(fallbackTitleBarWindowId, parentWindowId);
    return true;
}

FramelessHelperWin::FramelessHelperWin() : QAbstractNativeEventFilter() {}

FramelessHelperWin::~FramelessHelperWin() = default;

void FramelessHelperWin::addWindow(const SystemParameters &params)
{
    Q_ASSERT(params.isValid());
    if (!params.isValid()) {
        return;
    }
    const WId windowId = params.getWindowId();
    g_win32Helper()->mutex.lock();
    if (g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return;
    }
    Win32HelperData data = {};
    data.params = params;
    data.dpi = {Utils::getWindowDpi(windowId, true), Utils::getWindowDpi(windowId, false)};
    g_win32Helper()->data.insert(windowId, data);
    if (g_win32Helper()->nativeEventFilter.isNull()) {
        g_win32Helper()->nativeEventFilter.reset(new FramelessHelperWin);
        qApp->installNativeEventFilter(g_win32Helper()->nativeEventFilter.data());
    }
    g_win32Helper()->mutex.unlock();
    DEBUG.noquote() << "The DPI of window" << hwnd2str(windowId) << "is" << data.dpi;
    // Some Qt internals have to be corrected.
    Utils::maybeFixupQtInternals(windowId);
    // Qt maintains a frame margin internally, we need to update it accordingly
    // otherwise we'll get lots of warning messages when we change the window
    // geometry, it will also affect the final window geometry because QPA will
    // always take it into account when setting window size and position.
    Utils::updateInternalWindowFrameMargins(params.getWindowHandle(), true);
    // Tell DWM our preferred frame margin.
    Utils::updateWindowFrameMargins(windowId, false);
    // Tell DWM we don't use the window icon/caption/sysmenu, don't draw them.
    Utils::hideOriginalTitleBarElements(windowId);
    // Without this hack, the child windows can't get DPI change messages from
    // Windows, which means only the top level windows can be scaled to the correct
    // size, we of course don't want such thing from happening.
    Utils::fixupChildWindowsDpiMessage(windowId);
    if (WindowsVersionHelper::isWin10RS1OrGreater()) {
        // Tell DWM we may need dark theme non-client area (title bar & frame border).
        FramelessHelper::Core::setApplicationOSThemeAware();
        if (WindowsVersionHelper::isWin10RS5OrGreater()) {
            const bool dark = Utils::shouldAppsUseDarkMode();
            const auto isWidget = [&params]() -> bool {
                const auto widget = params.getWidgetHandle();
                return (widget && widget->isWidgetType());
            }();
            if (!isWidget) {
                // Tell UXTheme we may need dark theme controls.
                // Causes some QtWidgets paint incorrectly, so only apply to Qt Quick applications.
                Utils::updateGlobalWin32ControlsTheme(windowId, dark);
            }
            Utils::refreshWin32ThemeResources(windowId, dark);
            if (WindowsVersionHelper::isWin11OrGreater()) {
                // The fallback title bar window is only used to activate the Snap Layout feature
                // introduced in Windows 11, so it's not necessary to create it on systems below Win11.
                if (!FramelessConfig::instance()->isSet(Option::DisableWindowsSnapLayout)) {
                    if (!createFallbackTitleBarWindow(windowId, data.params.isWindowFixedSize())) {
                        WARNING << "Failed to create the fallback title bar window.";
                    }
                }
            }
        }
    }
}

void FramelessHelperWin::removeWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    g_win32Helper()->mutex.lock();
    if (!g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return;
    }
    g_win32Helper()->data.remove(windowId);
    if (g_win32Helper()->data.isEmpty()) {
        if (!g_win32Helper()->nativeEventFilter.isNull()) {
            qApp->removeNativeEventFilter(g_win32Helper()->nativeEventFilter.data());
            g_win32Helper()->nativeEventFilter.reset();
        }
    }
    HWND hwnd = nullptr;
    auto it = g_win32Helper()->fallbackTitleBarToParentWindowMapping.constBegin();
    while (it != g_win32Helper()->fallbackTitleBarToParentWindowMapping.constEnd()) {
        if (it.value() == windowId) {
            const WId key = it.key();
            hwnd = reinterpret_cast<HWND>(key);
            g_win32Helper()->fallbackTitleBarToParentWindowMapping.remove(key);
            break;
        }
        ++it;
    }
    g_win32Helper()->mutex.unlock();
    if (DestroyWindow(reinterpret_cast<HWND>(hwnd)) == FALSE) {
        WARNING << Utils::getSystemErrorMessage(kDestroyWindow);
    }
}

bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, QT_NATIVE_EVENT_RESULT_TYPE *result)
{
    if ((eventType != kWin32MessageTypeName) || !message || !result) {
        return false;
    }
    // QPA by default stores the global mouse position in the pt field,
    // but let's not reply on such Qt-specific extensions.
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    // Work-around a bug caused by typo which only exists in Qt 5.11.1
    const auto msg = *static_cast<MSG **>(message);
#else
    const auto msg = static_cast<LPMSG>(message);
#endif
    const HWND hWnd = msg->hwnd;
    if (!hWnd) {
        // Why sometimes the window handle is null? Is it designed to be like this?
        // Anyway, we should skip the entire processing in this case.
        return false;
    }
    const UINT uMsg = msg->message;
    // WM_QUIT won't be posted to the WindowProc function.
    if ((uMsg == WM_CLOSE) || (uMsg == WM_DESTROY)) {
        return false;
    }
    const auto windowId = reinterpret_cast<WId>(hWnd);
    g_win32Helper()->mutex.lock();
    if (!g_win32Helper()->data.contains(windowId)) {
        g_win32Helper()->mutex.unlock();
        return false;
    }
    const Win32HelperData data = g_win32Helper()->data.value(windowId);
    g_win32Helper()->mutex.unlock();
    const bool frameBorderVisible = Utils::isWindowFrameBorderVisible();
    const WPARAM wParam = msg->wParam;
    const LPARAM lParam = msg->lParam;
    switch (uMsg) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 9, 0)) // Qt has done this for us since 5.9.0
    case WM_NCCREATE: {
        // Enable automatic DPI scaling for the non-client area of the window,
        // such as the caption bar, the scrollbars, and the menu bar. We need
        // to do this explicitly and manually here (only inside WM_NCCREATE).
        // If we are using the PMv2 DPI awareness mode, the non-client area
        // of the window will be scaled by the OS automatically, so there will
        // be no need to do this in that case.
        Utils::enableNonClientAreaDpiScalingForWindow(windowId);
    } break;
#endif
    case WM_NCCALCSIZE: {
        // Windows是根据这个消息的返回值来设置窗口的客户区（窗口中真正显示的内容）
        // 和非客户区（标题栏、窗口边框、菜单栏和状态栏等Windows系统自行提供的部分
        // ，不过对于Qt来说，除了标题栏和窗口边框，非客户区基本也都是自绘的）的范
        // 围的，lParam里存放的就是新客户区的几何区域，默认是整个窗口的大小，正常
        // 的程序需要修改这个参数，告知系统窗口的客户区和非客户区的范围（一般来说可
        // 以完全交给Windows，让其自行处理，使用默认的客户区和非客户区），因此如果
        // 我们不修改lParam，就可以使客户区充满整个窗口，从而去掉标题栏和窗口边框
        // （因为这些东西都被客户区给盖住了。但边框阴影也会因此而丢失，不过我们会使
        // 用其他方式将其带回，请参考其他消息的处理，此处不过多提及）。但有个情况要
        // 特别注意，那就是窗口最大化后，窗口的实际尺寸会比屏幕的尺寸大一点，从而使
        // 用户看不到窗口的边界，这样用户就不能在窗口最大化后调整窗口的大小了（虽然
        // 这个做法听起来特别奇怪，但Windows确实就是这样做的），因此如果我们要自行
        // 处理窗口的非客户区，就要在窗口最大化后，将窗口边框的宽度和高度（一般是相
        // 等的）从客户区裁剪掉，否则我们窗口所显示的内容就会超出屏幕边界，显示不全。
        // 如果用户开启了任务栏自动隐藏，在窗口最大化后，还要考虑任务栏的位置。因为
        // 如果窗口最大化后，其尺寸和屏幕尺寸相等（因为任务栏隐藏了，所以窗口最大化
        // 后其实是充满了整个屏幕，变相的全屏了），Windows会认为窗口已经进入全屏的
        // 状态，从而导致自动隐藏的任务栏无法弹出。要避免这个状况，就要使窗口的尺寸
        // 小于屏幕尺寸。我下面的做法参考了火狐、Chromium和Windows Terminal
        // 如果没有开启任务栏自动隐藏，是不存在这个问题的，所以要先进行判断。
        // 一般情况下，*result设置为0（相当于DefWindowProc的返回值为0）就可以了，
        // 根据MSDN的说法，返回0意为此消息已经被程序自行处理了，让Windows跳过此消
        // 息，否则Windows会添加对此消息的默认处理，对于当前这个消息而言，就意味着
        // 标题栏和窗口边框又会回来，这当然不是我们想要的结果。根据MSDN，当wParam
        // 为FALSE时，只能返回0，但当其为TRUE时，可以返回0，也可以返回一个WVR_常
        // 量。根据Chromium的注释，当存在非客户区时，如果返回WVR_REDRAW会导致子
        // 窗口/子控件出现奇怪的bug（自绘控件错位），并且Lucas在Windows 10
        // 上成功复现，说明这个bug至今都没有解决。我查阅了大量资料，发现唯一的解决
        // 方案就是返回0。但如果不存在非客户区，且wParam为TRUE，最好返回
        // WVR_REDRAW，否则窗口在调整大小可能会产生严重的闪烁现象。
        // 虽然对大多数消息来说，返回0都代表让Windows忽略此消息，但实际上不同消息
        // 能接受的返回值是不一样的，请注意自行查阅MSDN。

        // Sent when the size and position of a window's client area must be
        // calculated. By processing this message, an application can
        // control the content of the window's client area when the size or
        // position of the window changes. If wParam is TRUE, lParam points
        // to an NCCALCSIZE_PARAMS structure that contains information an
        // application can use to calculate the new size and position of the
        // client rectangle. If wParam is FALSE, lParam points to a RECT
        // structure. On entry, the structure contains the proposed window
        // rectangle for the window. On exit, the structure should contain
        // the screen coordinates of the corresponding window client area.
        // The client area is the window's content area, the non-client area
        // is the area which is provided by the system, such as the title
        // bar, the four window borders, the frame shadow, the menu bar, the
        // status bar, the scroll bar, etc. But for Qt, it draws most of the
        // window area (client + non-client) itself. We now know that the
        // title bar and the window frame is in the non-client area and we
        // can set the scope of the client area in this message, so we can
        // remove the title bar and the window frame by let the non-client
        // area be covered by the client area (because we can't really get
        // rid of the non-client area, it will always be there, all we can
        // do is to hide it) , which means we should let the client area's
        // size the same with the whole window's size. So there is no room
        // for the non-client area and then the user won't be able to see it
        // again. But how to achieve this? Very easy, just leave lParam (the
        // re-calculated client area) untouched. But of course you can
        // modify lParam, then the non-client area will be seen and the
        // window borders and the window frame will show up. However, things
        // are quite different when you try to modify the top margin of the
        // client area. DWM will always draw the whole title bar no matter
        // what margin value you set for the top, unless you don't modify it
        // and remove the whole top area (the title bar + the one pixel
        // height window border). This can be confirmed in Windows
        // Terminal's source code, you can also try yourself to verify
        // it. So things will become quite complicated if you want to
        // preserve the four window borders.

        // If `wParam` is `FALSE`, `lParam` points to a `RECT` that contains
        // the proposed window rectangle for our window. During our
        // processing of the `WM_NCCALCSIZE` message, we are expected to
        // modify the `RECT` that `lParam` points to, so that its value upon
        // our return is the new client area. We must return 0 if `wParam`
        // is `FALSE`.
        // If `wParam` is `TRUE`, `lParam` points to a `NCCALCSIZE_PARAMS`
        // struct. This struct contains an array of 3 `RECT`s, the first of
        // which has the exact same meaning as the `RECT` that is pointed to
        // by `lParam` when `wParam` is `FALSE`. The remaining `RECT`s, in
        // conjunction with our return value, can
        // be used to specify portions of the source and destination window
        // rectangles that are valid and should be preserved. We opt not to
        // implement an elaborate client-area preservation technique, and
        // simply return 0, which means "preserve the entire old client area
        // and align it with the upper-left corner of our new client area".
        const auto clientRect = ((static_cast<BOOL>(wParam) == FALSE) ?
            reinterpret_cast<LPRECT>(lParam) : &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam))->rgrc[0]);
        if (frameBorderVisible) {
            // Store the original top margin before the default window procedure applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame because we don't want to remove the whole window frame,
            // we still need the standard window frame (the resizable frame border and the frame
            // shadow) for the left, bottom and right edges.
            // If we return 0 here directly, the whole window frame will be removed (which means
            // there will be no resizable frame border and the frame shadow will also disappear),
            // and that's also how most applications customize their title bars on Windows. It's
            // totally OK but since we want to preserve as much original frame as possible, we
            // can't use that solution.
            const LRESULT ret = DefWindowProcW(hWnd, WM_NCCALCSIZE, wParam, lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied,
            // and the whole top frame (the title bar and the top border) is gone now.
            // For the top frame, we only has 2 choices: (1) remove the top frame entirely, or
            // (2) don't touch it at all. We can't preserve the top border by adjusting the top
            // margin here. If we try to modify the top margin, the original title bar will
            // always be painted by DWM regardless what margin we set, so here we can only remove
            // the top frame entirely and use some special technique to bring the top border back.
            clientRect->top = originalTop;
        }
        const bool max = IsMaximized(hWnd);
        const bool full = Utils::isFullScreen(windowId);
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
            clientRect->top += frameSizeY;
            if (!frameBorderVisible) {
                clientRect->bottom -= frameSizeY;
                const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
                clientRect->left += frameSizeX;
                clientRect->right -= frameSizeX;
            }
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (max || full) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = SHAppBarMessage(ABM_GETSTATE, &abd);
            // First, check if we have an auto-hide taskbar at all:
            if (taskbarState & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to ABM_GETAUTOHIDEBAREX was introduced in Windows 8.1,
                // we have to use another way to judge this if we are running
                // on Windows 7 or Windows 8.
                if (WindowsVersionHelper::isWin8Point1OrGreater()) {
                    MONITORINFOEXW monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    const HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                    if (!monitor) {
                        WARNING << Utils::getSystemErrorMessage(kMonitorFromWindow);
                        break;
                    }
                    if (GetMonitorInfoW(monitor, &monitorInfo) == FALSE) {
                        WARNING << Utils::getSystemErrorMessage(kGetMonitorInfoW);
                        break;
                    }
                    // This helper can be used to determine if there's a
                    // auto-hide taskbar on the given edge of the monitor
                    // we're currently on.
                    const auto hasAutohideTaskbar = [&monitorInfo](const UINT edge) -> bool {
                        APPBARDATA _abd;
                        SecureZeroMemory(&_abd, sizeof(_abd));
                        _abd.cbSize = sizeof(_abd);
                        _abd.uEdge = edge;
                        _abd.rc = monitorInfo.rcMonitor;
                        const auto hTaskbar = reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &_abd));
                        return (hTaskbar != nullptr);
                    };
                    top = hasAutohideTaskbar(ABE_TOP);
                    bottom = hasAutohideTaskbar(ABE_BOTTOM);
                    left = hasAutohideTaskbar(ABE_LEFT);
                    right = hasAutohideTaskbar(ABE_RIGHT);
                } else {
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                        if (!windowMonitor) {
                            WARNING << Utils::getSystemErrorMessage(kMonitorFromWindow);
                            break;
                        }
                        const HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (!taskbarMonitor) {
                            WARNING << Utils::getSystemErrorMessage(kMonitorFromWindow);
                            break;
                        }
                        if (taskbarMonitor == windowMonitor) {
                            SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    } else {
                        WARNING << Utils::getSystemErrorMessage(kFindWindowW);
                        break;
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }
                // If there's a taskbar on any side of the monitor, reduce
                // our size a little bit on that edge.
                // Note to future code archeologists:
                // This doesn't seem to work for fullscreen on the primary
                // display. However, testing a bunch of other apps with
                // fullscreen modes and an auto-hiding taskbar has
                // shown that _none_ of them reveal the taskbar from
                // fullscreen mode. This includes Edge, Firefox, Chrome,
                // Sublime Text, PowerPoint - none seemed to support this.
                // This does however work fine for maximized.
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += kAutoHideTaskBarThickness;
                } else if (bottom) {
                    clientRect->bottom -= kAutoHideTaskBarThickness;
                } else if (left) {
                    clientRect->left += kAutoHideTaskBarThickness;
                } else if (right) {
                    clientRect->right -= kAutoHideTaskBarThickness;
                }
            }
        }
        Utils::syncWmPaintWithDwm(); // This should be executed at the very last.
        // By returning WVR_REDRAW we can make the window resizing look less broken.
        // But we must return 0 if wParam is FALSE, according to Microsoft Docs.
        // **IMPORTANT NOTE**:
        // If you are drawing something manually through D3D in your window, don't
        // try to return WVR_REDRAW here, otherwise Windows exhibits bugs where
        // client pixels and child windows are mispositioned by the width/height
        // of the upper-left non-client area. It's confirmed that this issue exists
        // from Windows 7 to Windows 10. Not tested on Windows 11 yet. Don't know
        // whether it exists on Windows XP to Windows Vista or not.
        const bool needD3DWorkaround = (qEnvironmentVariableIntValue(kD3DWorkaroundEnvVar) != 0);
        *result = (((static_cast<BOOL>(wParam) == FALSE) || needD3DWorkaround) ? 0 : WVR_REDRAW);
        return true;
    }
    case WM_NCHITTEST: {
        // 原生Win32窗口只有顶边是在窗口内部resize的，其余三边都是在窗口
        // 外部进行resize的，其原理是，WS_THICKFRAME这个窗口样式会在窗
        // 口的左、右和底边添加三个透明的resize区域，这三个区域在正常状态
        // 下是完全不可见的，它们由DWM负责绘制和控制。这些区域的宽度等于
        // (SM_CXSIZEFRAME + SM_CXPADDEDBORDER)，高度等于
        // (SM_CYSIZEFRAME + SM_CXPADDEDBORDER)，在100%缩放时，均等
        // 于8像素。它们属于窗口区域的一部分，但不属于客户区，而是属于非客
        // 户区，因此GetWindowRect获取的区域中是包含这三个resize区域的，
        // 而GetClientRect获取的区域是不包含它们的。当把
        // DWMWA_EXTENDED_FRAME_BOUNDS作为参数调用
        // DwmGetWindowAttribute时，也能获取到一个窗口大小，这个大小介
        // 于前面两者之间，暂时不知道这个数据的意义及其作用。我们在
        // WM_NCCALCSIZE消息的处理中，已经把整个窗口都设置为客户区了，也
        // 就是说，我们的窗口已经没有非客户区了，因此那三个透明的resize区
        // 域，此刻也已经成为窗口客户区的一部分了，从而变得不透明了。所以
        // 现在的resize，看起来像是在窗口内部resize，是因为原本透明的地方
        // 现在变得不透明了，实际上，单纯从范围上来看，现在我们resize的地方，
        // 就是普通窗口的边框外部，那三个透明区域的范围。
        // 因此，如果我们把边框完全去掉（就是我们正在做的事情），resize就
        // 会看起来是在内部进行，这个问题通过常规方法非常难以解决。我测试过
        // QQ和钉钉的窗口，它们的窗口就是在外部resize，但实际上它们是通过
        // 把窗口实际的内容，嵌入到一个完全透明的但尺寸要大一圈的窗口中实现
        // 的，虽然看起来效果还不错，但对于此项目而言，代码和窗口结构过于复
        // 杂，因此我没有采用此方案。然而，对于具体的软件项目而言，其做法也
        // 不失为一个优秀的解决方案，毕竟其在大多数条件下的表现都还可以。
        //
        // 和1.x的做法不同，现在的2.x选择了保留窗口三边，去除整个窗口顶部，
        // 好处是保留了系统的原生边框，外观较好，且与系统结合紧密，而且resize
        // 的表现也有很大改善，缺点是需要自行绘制顶部边框线。原本以为只能像
        // Windows Terminal那样在WM_PAINT里搞黑魔法，但后来发现，其实只
        // 要颜色相近，我们自行绘制一根实线也几乎能以假乱真，而且这样也不会
        // 破坏Qt自己的绘制系统，能做到不依赖黑魔法就能实现像Windows Terminal
        // 那样外观和功能都比较完美的自定义边框。

        // A normal Win32 window can be resized outside of it. Here is the
        // reason: the WS_THICKFRAME window style will cause a window has three
        // transparent areas beside the window's left, right and bottom
        // edge. Their width or height is eight pixels if the window is not
        // scaled. In most cases, they are totally invisible. It's DWM's
        // responsibility to draw and control them. They exist to let the
        // user resize the window, visually outside of it. They are in the
        // window area, but not the client area, so they are in the
        // non-client area actually. But we have turned the whole window
        // area into client area in WM_NCCALCSIZE, so the three transparent
        // resize areas also become a part of the client area and thus they
        // become visible. When we resize the window, it looks like we are
        // resizing inside of it, however, that's because the transparent
        // resize areas are visible now, we ARE resizing outside of the
        // window actually. But I don't know how to make them become
        // transparent again without breaking the frame shadow drawn by DWM.
        // If you really want to solve it, you can try to embed your window
        // into a larger transparent window and draw the frame shadow
        // yourself. As what we have said in WM_NCCALCSIZE, you can only
        // remove the top area of the window, this will let us be able to
        // resize outside of the window and don't need much process in this
        // message, it looks like a perfect plan, however, the top border is
        // missing due to the whole top area is removed, and it's very hard
        // to bring it back because we have to use a trick in WM_PAINT
        // (learned from Windows Terminal), but no matter what we do in
        // WM_PAINT, it will always break the backing store mechanism of Qt,
        // so actually we can't do it. And it's very difficult to do such
        // things in NativeEventFilters as well. What's worse, if we really
        // do this, the four window borders will become white and they look
        // horrible in dark mode. This solution only supports Windows 10
        // because the border width on Win10 is only one pixel, however it's
        // eight pixels on Windows 7 so preserving the three window borders
        // looks terrible on old systems.
        //
        // Unlike the 1.x code, we choose to preserve the three edges of the
        // window in 2.x, and get rid of the whole top part of the window.
        // There are quite some advantages such as the appearance looks much
        // better and due to we have the original system window frame, our
        // window can behave just like a normal Win32 window even if we now
        // doesn't have a title bar at all. Most importantly, the flicker and
        // jitter during window resizing is totally gone now. The disadvantage
        // is we have to draw a top frame border ourself. Previously I thought
        // we have to do the black magic in WM_PAINT just like what Windows
        // Terminal does, however, later I found that if we choose a proper
        // color, our homemade top border can almost have exactly the same
        // appearance with the system's one.

        const POINT nativeGlobalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT nativeLocalPos = nativeGlobalPos;
        if (ScreenToClient(hWnd, &nativeLocalPos) == FALSE) {
            WARNING << Utils::getSystemErrorMessage(kScreenToClient);
            break;
        }
        const qreal dpr = data.params.getWindowDevicePixelRatio();
        const QPoint qtScenePos = QPointF(QPointF(qreal(nativeLocalPos.x), qreal(nativeLocalPos.y)) / dpr).toPoint();
        const bool max = IsMaximized(hWnd);
        const bool full = Utils::isFullScreen(windowId);
        const int frameSizeY = Utils::getResizeBorderThickness(windowId, false, true);
        const bool isTop = (nativeLocalPos.y < frameSizeY);
        const bool buttonSwapped = (GetSystemMetrics(SM_SWAPBUTTON) != FALSE);
        const bool leftButtonPressed = (buttonSwapped ?
                (GetAsyncKeyState(VK_RBUTTON) < 0) : (GetAsyncKeyState(VK_LBUTTON) < 0));
        const bool isTitleBar = (data.params.isInsideTitleBarDraggableArea(qtScenePos) && leftButtonPressed);
        const bool isFixedSize = data.params.isWindowFixedSize();
        const bool dontOverrideCursor = data.params.getProperty(kDontOverrideCursorVar, false).toBool();
        const bool dontToggleMaximize = data.params.getProperty(kDontToggleMaximizeVar, false).toBool();
        if (dontToggleMaximize) {
            static bool once = false;
            if (!once) {
                once = true;
                DEBUG << "To disable window maximization, you should remove the "
                         "WS_MAXIMIZEBOX style from the window instead. FramelessHelper "
                         "won't do that for you, so you'll have to do it manually yourself.";
            }
        }
        if (frameBorderVisible) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(hWnd, WM_NCHITTEST, 0, lParam);
            if (originalRet != HTCLIENT) {
                *result = (dontOverrideCursor ? HTBORDER : originalRet);
                return true;
            }
            if (full) {
                *result = HTCLIENT;
                return true;
            }
            if (max) {
                *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                return true;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (isTop && !isFixedSize) {
                // Return HTCLIENT instead of HTBORDER here, because the mouse is
                // inside our homemade title bar now, return HTCLIENT to let our
                // title bar can still capture mouse events.
                *result = (dontOverrideCursor ? HTCLIENT : HTTOP);
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        } else {
            if (full) {
                *result = HTCLIENT;
                return true;
            }
            if (max) {
                *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                return true;
            }
            if (!isFixedSize) {
                RECT clientRect = {0, 0, 0, 0};
                if (GetClientRect(hWnd, &clientRect) == FALSE) {
                    WARNING << Utils::getSystemErrorMessage(kGetClientRect);
                    break;
                }
                const LONG width = clientRect.right;
                const LONG height = clientRect.bottom;
                const bool isBottom = (nativeLocalPos.y >= (height - frameSizeY));
                // Make the border a little wider to let the user easy to resize on corners.
                const qreal scaleFactor = ((isTop || isBottom) ? 2.0 : 1.0);
                const int frameSizeX = Utils::getResizeBorderThickness(windowId, true, true);
                const int scaledFrameSizeX = qRound(qreal(frameSizeX) * scaleFactor);
                const bool isLeft = (nativeLocalPos.x < scaledFrameSizeX);
                const bool isRight = (nativeLocalPos.x >= (width - scaledFrameSizeX));
                if (dontOverrideCursor && (isTop || isBottom || isLeft || isRight)) {
                    // Return HTCLIENT instead of HTBORDER here, because the mouse is
                    // inside the window now, return HTCLIENT to let the controls
                    // inside our window can still capture mouse events.
                    *result = HTCLIENT;
                    return true;
                }
                if (isTop) {
                    if (isLeft) {
                        *result = HTTOPLEFT;
                        return true;
                    }
                    if (isRight) {
                        *result = HTTOPRIGHT;
                        return true;
                    }
                    *result = HTTOP;
                    return true;
                }
                if (isBottom) {
                    if (isLeft) {
                        *result = HTBOTTOMLEFT;
                        return true;
                    }
                    if (isRight) {
                        *result = HTBOTTOMRIGHT;
                        return true;
                    }
                    *result = HTBOTTOM;
                    return true;
                }
                if (isLeft) {
                    *result = HTLEFT;
                    return true;
                }
                if (isRight) {
                    *result = HTRIGHT;
                    return true;
                }
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        }
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 2)) // I contributed this to Qt since 6.2.2
    case WM_WINDOWPOSCHANGING: {
        // Tell Windows to discard the entire contents of the client area, as re-using
        // parts of the client area would lead to jitter during resize.
        const auto windowPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        windowPos->flags |= SWP_NOCOPYBITS;
    } break;
#endif
#if ((QT_VERSION <= QT_VERSION_CHECK(6, 2, 6)) || ((QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)) && (QT_VERSION <= QT_VERSION_CHECK(6, 4, 1))))
    case WM_GETDPISCALEDSIZE: {
        // QtBase commit 2cfca7fd1911cc82a22763152c04c65bc05bc19a introduced a bug
        // which caused the custom margins is ignored during the handling of the
        // WM_GETDPISCALEDSIZE message, it was shipped with Qt 6.2.1 ~ 6.2.6 & 6.3 ~ 6.4.1.
        // We workaround it by overriding the wrong handling directly.
        RECT clientRect = {};
        if (GetClientRect(hWnd, &clientRect) == FALSE) {
            WARNING << Utils::getSystemErrorMessage(kGetClientRect);
            *result = FALSE; // Use the default linear DPI scaling provided by Windows.
            return true; // Jump over Qt's wrong handling logic.
        }
        const QSizeF oldSize = {qreal(clientRect.right - clientRect.left), qreal(clientRect.bottom - clientRect.top)};
        const UINT oldDpi = data.dpi.x;
        static constexpr const auto defaultDpi = qreal(USER_DEFAULT_SCREEN_DPI);
        // We need to round the scale factor according to Qt's rounding policy.
        const qreal oldDpr = Utils::roundScaleFactor(qreal(oldDpi) / defaultDpi);
        const QSizeF unscaledSize = (oldSize / oldDpr);
        const auto newDpi = UINT(wParam);
        const qreal newDpr = Utils::roundScaleFactor(qreal(newDpi) / defaultDpi);
        const QSizeF newSize = (unscaledSize * newDpr);
        const auto suggestedSize = reinterpret_cast<LPSIZE>(lParam);
        suggestedSize->cx = qRound(newSize.width());
        suggestedSize->cy = qRound(newSize.height());
        // If the window frame is visible, we need to expand the suggested size, currently
        // it's pure client size, we need to add the frame size to it. Windows expects a
        // full window size, including the window frame.
        // If the window frame is not visible, the window size equals to the client size,
        // the suggested size doesn't need further adjustments.
        if (frameBorderVisible) {
            const int frameSizeX = Utils::getResizeBorderThicknessForDpi(true, newDpi);
            const int frameSizeY = Utils::getResizeBorderThicknessForDpi(false, newDpi);
            suggestedSize->cx += (frameSizeX * 2);
            suggestedSize->cy += frameSizeY;
        }
        *result = TRUE; // We have set our preferred window size, don't use the default linear DPI scaling.
        return true; // Jump over Qt's wrong handling logic.
    }
#endif
    case WM_DPICHANGED: {
        const Dpi dpi = {UINT(LOWORD(wParam)), UINT(HIWORD(wParam))};
        DEBUG.noquote() << "New DPI for window" << hwnd2str(hWnd) << "is" << dpi;
        g_win32Helper()->mutex.lock();
        g_win32Helper()->data[windowId].dpi = dpi;
        g_win32Helper()->mutex.unlock();
#if (QT_VERSION <= QT_VERSION_CHECK(6, 4, 1))
        // We need to wait until Qt has handled this message, otherwise everything
        // we have done here will always be overwritten.
        QTimer::singleShot(0, qApp, [data](){ // Copy the variables intentionally, otherwise they'll go out of scope when Qt finally use them.
            // Sync the internal window frame margins with the latest DPI, otherwise
            // we will get wrong window sizes after the DPI change.
            Utils::updateInternalWindowFrameMargins(data.params.getWindowHandle(), true);
        });
#endif // (QT_VERSION <= QT_VERSION_CHECK(6, 4, 1))
    } break;
    case WM_DWMCOMPOSITIONCHANGED: {
        // Re-apply the custom window frame if recovered from the basic theme.
        Utils::updateWindowFrameMargins(windowId, false);
    } break;
    default:
        break;
    }
    if (!frameBorderVisible) {
        switch (uMsg) {
        case WM_NCUAHDRAWCAPTION:
        case WM_NCUAHDRAWFRAME: {
            // These undocumented messages are sent to draw themed window
            // borders. Block them to prevent drawing borders over the client
            // area.
            *result = 0;
            return true;
        }
        case WM_NCPAINT: {
            // 边框阴影处于非客户区的范围，因此如果直接阻止非客户区的绘制，会导致边框阴影丢失

            if (!Utils::isDwmCompositionEnabled()) {
                // Only block WM_NCPAINT when DWM composition is disabled. If
                // it's blocked when DWM composition is enabled, the frame
                // shadow won't be drawn.
                *result = 0;
                return true;
            } else {
                break;
            }
        }
        case WM_NCACTIVATE: {
            if (Utils::isDwmCompositionEnabled()) {
                // DefWindowProc won't repaint the window border if lParam (normally a HRGN)
                // is -1. See the following link's "lParam" section:
                // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
                // Don't use "*result = 0" here, otherwise the window won't respond to the
                // window activation state change.
                *result = DefWindowProcW(hWnd, WM_NCACTIVATE, wParam, -1);
            } else {
                if (static_cast<BOOL>(wParam) == FALSE) {
                    *result = TRUE;
                } else {
                    *result = FALSE;
                }
            }
            return true;
        }
        case WM_SETICON:
        case WM_SETTEXT: {
            // Disable painting while these messages are handled to prevent them
            // from drawing a window caption over the client area.
            SetLastError(ERROR_SUCCESS);
            const auto oldStyle = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
            if (oldStyle == 0) {
                WARNING << Utils::getSystemErrorMessage(kGetWindowLongPtrW);
                break;
            }
            // Prevent Windows from drawing the default title bar by temporarily
            // toggling the WS_VISIBLE style.
            const DWORD newStyle = (oldStyle & ~WS_VISIBLE);
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLongPtrW(hWnd, GWL_STYLE, static_cast<LONG_PTR>(newStyle)) == 0) {
                WARNING << Utils::getSystemErrorMessage(kSetWindowLongPtrW);
                break;
            }
            Utils::triggerFrameChange(windowId);
            const LRESULT ret = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLongPtrW(hWnd, GWL_STYLE, static_cast<LONG_PTR>(oldStyle)) == 0) {
                WARNING << Utils::getSystemErrorMessage(kSetWindowLongPtrW);
                break;
            }
            Utils::triggerFrameChange(windowId);
            *result = ret;
            return true;
        }
        default:
            break;
        }
    }
    if (WindowsVersionHelper::isWin11OrGreater() && data.fallbackTitleBarWindowId) {
        switch (uMsg) {
        case WM_SIZE: // Sent to a window after its size has changed.
        case WM_DISPLAYCHANGE: // Sent to a window when the display resolution has changed.
        {
            const bool isFixedSize = data.params.isWindowFixedSize();
            if (!resizeFallbackTitleBarWindow(windowId, data.fallbackTitleBarWindowId, isFixedSize)) {
                WARNING << "Failed to re-position the fallback title bar window.";
            }
        } break;
        default:
            break;
        }
    }
    const bool wallpaperChanged = ((uMsg == WM_SETTINGCHANGE) && (wParam == SPI_SETDESKWALLPAPER));
    bool systemThemeChanged = ((uMsg == WM_THEMECHANGED) || (uMsg == WM_SYSCOLORCHANGE)
                               || (uMsg == WM_DWMCOLORIZATIONCOLORCHANGED));
    if (WindowsVersionHelper::isWin10RS1OrGreater()) {
        if (uMsg == WM_SETTINGCHANGE) {
            if ((wParam == 0) && (lParam != 0) // lParam sometimes may be NULL.
                && (std::wcscmp(reinterpret_cast<LPCWSTR>(lParam), kThemeSettingChangeEventName) == 0)) {
                systemThemeChanged = true;
                if (WindowsVersionHelper::isWin10RS5OrGreater()) {
                    const bool dark = Utils::shouldAppsUseDarkMode();
                    const auto isWidget = [&data]() -> bool {
                        const auto widget = data.params.getWidgetHandle();
                        return (widget && widget->isWidgetType());
                    }();
                    if (!isWidget) {
                        // Causes some QtWidgets paint incorrectly, so only apply to Qt Quick applications.
                        Utils::updateGlobalWin32ControlsTheme(windowId, dark);
                    }
                    Utils::refreshWin32ThemeResources(windowId, dark);
                }
            }
        }
    }
    if (systemThemeChanged || wallpaperChanged) {
        // Sometimes the FramelessManager instance may be destroyed already.
        if (FramelessManager * const manager = FramelessManager::instance()) {
            if (FramelessManagerPrivate * const managerPriv = FramelessManagerPrivate::get(manager)) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
                if (systemThemeChanged) {
                    managerPriv->notifySystemThemeHasChangedOrNot();
                }
#endif // (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
                if (wallpaperChanged) {
                    managerPriv->notifyWallpaperHasChangedOrNot();
                }
            }
        }
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
