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

#pragma once

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtCore/qpointer.h>
#include <QtGui/qcolor.h>
#include <QtGui/qwindowdefs.h>
#include <functional>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

#ifndef FRAMELESSHELPER_CORE_API
#  ifdef FRAMELESSHELPER_CORE_STATIC
#    define FRAMELESSHELPER_CORE_API
#  else
#    ifdef FRAMELESSHELPER_CORE_LIBRARY
#      define FRAMELESSHELPER_CORE_API Q_DECL_EXPORT
#    else
#      define FRAMELESSHELPER_CORE_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS)
#  define Q_OS_WINDOWS
#endif

#ifndef Q_DISABLE_COPY_MOVE
#  define Q_DISABLE_COPY_MOVE(Class) \
      Q_DISABLE_COPY(Class) \
      Class(Class &&) = delete; \
      Class &operator=(Class &&) = delete;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#  define qAsConst(i) std::as_const(i)
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
#  define QStringView const QString &
#else
#  include <QtCore/qstringview.h>
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
#  define qExchange(a, b) std::exchange(a, b)
#  define Q_NAMESPACE_EXPORT(...) Q_NAMESPACE
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#  define Q_NODISCARD [[nodiscard]]
#  define Q_MAYBE_UNUSED [[maybe_unused]]
#  define Q_CONSTEXPR2 constexpr
#else
#  define Q_NODISCARD
#  define Q_MAYBE_UNUSED
#  define Q_CONSTEXPR2
#endif

#ifndef QT_NATIVE_EVENT_RESULT_TYPE
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#    define QT_NATIVE_EVENT_RESULT_TYPE qintptr
#    define QT_ENTER_EVENT_TYPE QEnterEvent
#  else
#    define QT_NATIVE_EVENT_RESULT_TYPE long
#    define QT_ENTER_EVENT_TYPE QEvent
#  endif
#endif

#ifndef QUtf8String
#  define QUtf8String(str) QString::fromUtf8(str)
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY_LITERAL
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#    define FRAMELESSHELPER_BYTEARRAY_LITERAL(ba) ba##_qba
#  else
#    define FRAMELESSHELPER_BYTEARRAY_LITERAL(ba) QByteArrayLiteral(ba)
#  endif
#endif

#ifndef FRAMELESSHELPER_STRING_LITERAL
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#    define FRAMELESSHELPER_STRING_LITERAL(str) u##str##_qs
#  else
#    define FRAMELESSHELPER_STRING_LITERAL(str) QStringLiteral(str)
#  endif
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY_CONSTANT2
#  define FRAMELESSHELPER_BYTEARRAY_CONSTANT2(name, ba) \
     [[maybe_unused]] static const auto k##name = FRAMELESSHELPER_BYTEARRAY_LITERAL(ba);
#endif

#ifndef FRAMELESSHELPER_STRING_CONSTANT2
#  define FRAMELESSHELPER_STRING_CONSTANT2(name, str) \
     [[maybe_unused]] static const auto k##name = FRAMELESSHELPER_STRING_LITERAL(str);
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY_CONSTANT
#  define FRAMELESSHELPER_BYTEARRAY_CONSTANT(ba) FRAMELESSHELPER_BYTEARRAY_CONSTANT2(ba, #ba)
#endif

#ifndef FRAMELESSHELPER_STRING_CONSTANT
#  define FRAMELESSHELPER_STRING_CONSTANT(str) FRAMELESSHELPER_STRING_CONSTANT2(str, #str)
#endif

#ifndef FRAMELESSHELPER_NAMESPACE
#  define FRAMELESSHELPER_NAMESPACE wangwenx190::FramelessHelper
#endif

#ifndef FRAMELESSHELPER_BEGIN_NAMESPACE
#  define FRAMELESSHELPER_BEGIN_NAMESPACE namespace FRAMELESSHELPER_NAMESPACE {
#endif

#ifndef FRAMELESSHELPER_END_NAMESPACE
#  define FRAMELESSHELPER_END_NAMESPACE }
#endif

#ifndef FRAMELESSHELPER_USE_NAMESPACE
#  define FRAMELESSHELPER_USE_NAMESPACE using namespace FRAMELESSHELPER_NAMESPACE;
#endif

#ifndef FRAMELESSHELPER_PREPEND_NAMESPACE
#  define FRAMELESSHELPER_PREPEND_NAMESPACE(X) ::FRAMELESSHELPER_NAMESPACE::X
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

namespace Global
{

Q_NAMESPACE_EXPORT(FRAMELESSHELPER_CORE_API)

[[maybe_unused]] static constexpr const int kDefaultResizeBorderThickness = 8;
[[maybe_unused]] static constexpr const int kDefaultCaptionHeight = 23;
[[maybe_unused]] static constexpr const int kDefaultTitleBarHeight = 30;
[[maybe_unused]] static constexpr const int kDefaultWindowFrameBorderThickness = 1;
[[maybe_unused]] static constexpr const int kDefaultTitleBarFontPointSize = 11;
[[maybe_unused]] static constexpr const int kDefaultTitleBarTitleLabelMargin = 10;

[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultBlackColor = {0, 0, 0}; // #000000
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultWhiteColor = {255, 255, 255}; // #FFFFFF
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultTransparentColor = {0, 0, 0, 0};
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultDarkGrayColor = {169, 169, 169}; // #A9A9A9
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultSystemLightColor = {240, 240, 240}; // #F0F0F0
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultSystemDarkColor = {32, 32, 32}; // #202020
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultFrameBorderActiveColor = {77, 77, 77}; // #4D4D4D
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultFrameBorderInactiveColorDark = {87, 89, 89}; // #575959
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultFrameBorderInactiveColorLight = {166, 166, 166}; // #A6A6A6
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultSystemButtonBackgroundColor = {204, 204, 204}; // #CCCCCC
[[maybe_unused]] static Q_CONSTEXPR2 const QColor kDefaultSystemCloseButtonBackgroundColor = {232, 17, 35}; // #E81123

[[maybe_unused]] static constexpr const QSize kDefaultSystemButtonSize = {int(qRound(qreal(kDefaultTitleBarHeight) * 1.5)), kDefaultTitleBarHeight};
[[maybe_unused]] static constexpr const QSize kDefaultSystemButtonIconSize = {16, 16};
[[maybe_unused]] static constexpr const QSize kDefaultWindowSize = {160, 160};

[[maybe_unused]] static constexpr const char kUsePureQtImplFlag[] = "FRAMELESSHELPER_PURE_QT_IMPLEMENTATION";
[[maybe_unused]] static constexpr const char kForceHideFrameBorderFlag[] = "FRAMELESSHELPER_FORCE_HIDE_FRAME_BORDER";
[[maybe_unused]] static constexpr const char kForceShowFrameBorderFlag[] = "FRAMELESSHELPER_FORCE_SHOW_FRAME_BORDER";

FRAMELESSHELPER_STRING_CONSTANT2(ConfigFileName, ".framelesshelper.ini")
FRAMELESSHELPER_STRING_CONSTANT2(UsePureQtImplKeyPath, "Options/UsePureQtImplementation")
FRAMELESSHELPER_STRING_CONSTANT2(ForceHideFrameBorderKeyPath, "Options/ForceHideFrameBorder")
FRAMELESSHELPER_STRING_CONSTANT2(ForceShowFrameBorderKeyPath, "Options/ForceShowFrameBorder")

enum class Option
{
    ForceHideWindowFrameBorder            = 0x00000001, // Windows only, force hide the window frame border even on Windows 10 and onwards.
    ForceShowWindowFrameBorder            = 0x00000002, // Windows only, force show the window frame border even on Windows 7 (~ 8.1).
    DontDrawTopWindowFrameBorder          = 0x00000004, // Windows only, don't draw the top window frame border even if the window frame border is visible.
    DontForceSquareWindowCorners          = 0x00000008, // Windows only, don't force the window corners to be square if the default corner style is not square.
    TransparentWindowBackground           = 0x00000010, // Make the window's background become transparent.
    DisableWindowsSnapLayout              = 0x00000020, // Windows only, don't enable the snap layout feature (available since Windows 11) unconditionally.
    CreateStandardWindowLayout            = 0x00000040, // Using this option will cause FramelessHelper create a homemade titlebar and a window layout to contain it. If your window has a layout already, the newly created layout will mess up your own layout.
    BeCompatibleWithQtFramelessWindowHint = 0x00000080, // Windows only, make the code compatible with Qt::FramelessWindowHint. Don't use this option unless you really need that flag.
    DontTouchQtInternals                  = 0x00000100, // Windows only, don't modify Qt's internal data.
    DontTouchWindowFrameBorderColor       = 0x00000200, // Windows only, don't change the window frame border color.
    DontInstallSystemMenuHook             = 0x00000400, // Windows only, don't install the system menu hook.
    DisableSystemMenu                     = 0x00000800, // Windows only, don't open the system menu when right clicks the titlebar.
    NoDoubleClickMaximizeToggle           = 0x00001000, // Don't toggle the maximized state when user double clicks the titlebar.
    DisableResizing                       = 0x00002000, // Disable resizing of the window.
    DisableDragging                       = 0x00004000, // Disable dragging through the titlebar of the window.
    DontTouchCursorShape                  = 0x00008000, // Don't change the cursor shape while the mouse is hovering above the window.
    DontMoveWindowToDesktopCenter         = 0x00010000, // Don't move the window to the desktop center before it's first shown.
    DontTreatFullScreenAsZoomed           = 0x00020000, // Don't treat fullscreen as zoomed (maximized).
    DontTouchHighDpiScalingPolicy         = 0x00040000, // Don't change Qt's default high DPI scaling policy. Qt5 default: disabled, Qt6 default: enabled.
    DontTouchScaleFactorRoundingPolicy    = 0x00080000, // Don't change Qt's default scale factor rounding policy. Qt5 default: round, Qt6 default: pass through.
    DontTouchProcessDpiAwarenessLevel     = 0x00100000, // Windows only, don't change the current process's DPI awareness level.
    DontEnsureNonNativeWidgetSiblings     = 0x00200000, // Don't ensure that siblings of native widgets stay non-native.
    SyncNativeControlsThemeWithSystem     = 0x00400000  // Windows only, sync the native Win32 controls' theme with system theme.
};
Q_ENUM_NS(Option)
Q_DECLARE_FLAGS(Options, Option)
Q_FLAG_NS(Options)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

enum class SystemTheme
{
    Unknown = -1,
    Light = 0,
    Dark = 1,
    HighContrast = 2
};
Q_ENUM_NS(SystemTheme)

enum class SystemButtonType
{
    Unknown = -1,
    WindowIcon = 0,
    Help = 1,
    Minimize = 2,
    Maximize = 3,
    Restore = 4,
    Close = 5
};
Q_ENUM_NS(SystemButtonType)

enum class ResourceType
{
    Image = 0,
    Pixmap = 1,
    Icon = 2
};
Q_ENUM_NS(ResourceType)

enum class DwmColorizationArea
{
    None_ = 0, // Avoid name conflicts with X11 headers.
    StartMenu_TaskBar_ActionCenter = 1,
    TitleBar_WindowBorder = 2,
    All = 3
};
Q_ENUM_NS(DwmColorizationArea)

enum class Anchor
{
    Top = 0,
    Bottom = 1,
    Left = 2,
    Right = 3,
    HorizontalCenter = 4,
    VerticalCenter = 5,
    Center = 6
};
Q_ENUM_NS(Anchor)

enum class ButtonState
{
    Unspecified = -1,
    Hovered = 0,
    Pressed = 1,
    Clicked = 2
};
Q_ENUM_NS(ButtonState)

enum class WindowsVersion
{
    _2000 = 0,
    _XP = 1,
    _XP_64 = 2,
    _Vista = 3,
    _Vista_SP1 = 4,
    _Vista_SP2 = 5,
    _7 = 6,
    _7_SP1 = 7,
    _8 = 8,
    _8_1 = 9,
    _8_1_Update1 = 10,
    _10_1507 = 11,
    _10_1511 = 12,
    _10_1607 = 13,
    _10_1703 = 14,
    _10_1709 = 15,
    _10_1803 = 16,
    _10_1809 = 17,
    _10_1903 = 18,
    _10_1909 = 19,
    _10_2004 = 20,
    _10_20H2 = 21,
    _10_21H1 = 22,
    _10_21H2 = 23,
    _11_21H2 = 24
};
Q_ENUM_NS(WindowsVersion)

struct VersionNumber
{
    int major = 0;
    int minor = 0;
    int patch = 0;
    int tweak = 0;

    [[nodiscard]] friend bool operator==(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        return ((lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.patch == rhs.patch) && (lhs.tweak == rhs.tweak));
    }

    [[nodiscard]] friend bool operator!=(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool operator>(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        if (lhs.major > rhs.major) {
            return true;
        }
        if (lhs.major < rhs.major) {
            return false;
        }
        if (lhs.minor > rhs.minor) {
            return true;
        }
        if (lhs.minor < rhs.minor) {
            return false;
        }
        if (lhs.patch > rhs.patch) {
            return true;
        }
        if (lhs.patch < rhs.patch) {
            return false;
        }
        if (lhs.tweak > rhs.tweak) {
            return true;
        }
        if (lhs.tweak < rhs.tweak) {
            return false;
        }
        return false;
    }

    [[nodiscard]] friend bool operator<(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        return ((lhs != rhs) && !(lhs > rhs));
    }

    [[nodiscard]] friend bool operator>=(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        return ((lhs > rhs) || (lhs == rhs));
    }

    [[nodiscard]] friend bool operator<=(const VersionNumber &lhs, const VersionNumber &rhs) noexcept
    {
        return ((lhs < rhs) || (lhs == rhs));
    }
};

using GetWindowFlagsCallback = std::function<Qt::WindowFlags()>;
using SetWindowFlagsCallback = std::function<void(const Qt::WindowFlags)>;

using GetWindowSizeCallback = std::function<QSize()>;
using SetWindowSizeCallback = std::function<void(const QSize &)>;

using GetWindowPositionCallback = std::function<QPoint()>;
using SetWindowPositionCallback = std::function<void(const QPoint &)>;

using GetWindowScreenCallback = std::function<QScreen *()>;

using IsWindowFixedSizeCallback = std::function<bool()>;
using SetWindowFixedSizeCallback = std::function<void(const bool)>;

using GetWindowStateCallback = std::function<Qt::WindowState()>;
using SetWindowStateCallback = std::function<void(const Qt::WindowState)>;

using GetWindowHandleCallback = std::function<QWindow *()>;

using WindowToScreenCallback = std::function<QPoint(const QPoint &)>;
using ScreenToWindowCallback = std::function<QPoint(const QPoint &)>;

using IsInsideSystemButtonsCallback = std::function<bool(const QPoint &, SystemButtonType *)>;
using IsInsideTitleBarDraggableAreaCallback = std::function<bool(const QPoint &)>;

using GetWindowDevicePixelRatioCallback = std::function<qreal()>;

using SetSystemButtonStateCallback = std::function<void(const SystemButtonType, const ButtonState)>;

using GetWindowIdCallback = std::function<WId()>;

using ShouldIgnoreMouseEventsCallback = std::function<bool(const QPoint &)>;

using ShowSystemMenuCallback = std::function<void(const QPoint &)>;

struct UserSettings
{
    QPoint startupPosition = {};
    QSize startupSize = {};
    Qt::WindowState startupState = Qt::WindowNoState;
    Options options = {};
    QPoint systemMenuOffset = {};
    QPointer<QObject> windowIconButton = nullptr;
    QPointer<QObject> contextHelpButton = nullptr;
    QPointer<QObject> minimizeButton = nullptr;
    QPointer<QObject> maximizeButton = nullptr;
    QPointer<QObject> closeButton = nullptr;
};

struct SystemParameters
{
    GetWindowFlagsCallback getWindowFlags = nullptr;
    SetWindowFlagsCallback setWindowFlags = nullptr;

    GetWindowSizeCallback getWindowSize = nullptr;
    SetWindowSizeCallback setWindowSize = nullptr;

    GetWindowPositionCallback getWindowPosition = nullptr;
    SetWindowPositionCallback setWindowPosition = nullptr;

    GetWindowScreenCallback getWindowScreen = nullptr;

    IsWindowFixedSizeCallback isWindowFixedSize = nullptr;
    SetWindowFixedSizeCallback setWindowFixedSize = nullptr;

    GetWindowStateCallback getWindowState = nullptr;
    SetWindowStateCallback setWindowState = nullptr;

    GetWindowHandleCallback getWindowHandle = nullptr;

    WindowToScreenCallback windowToScreen = nullptr;
    ScreenToWindowCallback screenToWindow = nullptr;

    IsInsideSystemButtonsCallback isInsideSystemButtons = nullptr;
    IsInsideTitleBarDraggableAreaCallback isInsideTitleBarDraggableArea = nullptr;

    GetWindowDevicePixelRatioCallback getWindowDevicePixelRatio = nullptr;

    SetSystemButtonStateCallback setSystemButtonState = nullptr;

    GetWindowIdCallback getWindowId = nullptr;

    ShouldIgnoreMouseEventsCallback shouldIgnoreMouseEvents = nullptr;

    ShowSystemMenuCallback showSystemMenu = nullptr;

    [[nodiscard]] inline bool isValid() const
    {
        Q_ASSERT(getWindowFlags);
        Q_ASSERT(setWindowFlags);
        Q_ASSERT(getWindowSize);
        Q_ASSERT(setWindowSize);
        Q_ASSERT(getWindowPosition);
        Q_ASSERT(setWindowPosition);
        Q_ASSERT(getWindowScreen);
        Q_ASSERT(isWindowFixedSize);
        Q_ASSERT(setWindowFixedSize);
        Q_ASSERT(getWindowState);
        Q_ASSERT(setWindowState);
        Q_ASSERT(getWindowHandle);
        Q_ASSERT(windowToScreen);
        Q_ASSERT(screenToWindow);
        Q_ASSERT(isInsideSystemButtons);
        Q_ASSERT(isInsideTitleBarDraggableArea);
        Q_ASSERT(getWindowDevicePixelRatio);
        Q_ASSERT(setSystemButtonState);
        Q_ASSERT(getWindowId);
        Q_ASSERT(shouldIgnoreMouseEvents);
        Q_ASSERT(showSystemMenu);
        return (getWindowFlags && setWindowFlags && getWindowSize
                && setWindowSize && getWindowPosition && setWindowPosition
                && getWindowScreen && isWindowFixedSize && setWindowFixedSize
                && getWindowState && setWindowState && getWindowHandle
                && windowToScreen && screenToWindow && isInsideSystemButtons
                && isInsideTitleBarDraggableArea && getWindowDevicePixelRatio
                && setSystemButtonState && getWindowId && shouldIgnoreMouseEvents
                && showSystemMenu);
    }
};

[[maybe_unused]] static constexpr const VersionNumber WindowsVersions[] =
{
    { 5, 0,  2195}, // Windows 2000
    { 5, 1,  2600}, // Windows XP
    { 5, 2,  3790}, // Windows XP x64 Edition or Windows Server 2003
    { 6, 0,  6000}, // Windows Vista
    { 6, 0,  6001}, // Windows Vista with Service Pack 1 or Windows Server 2008
    { 6, 0,  6002}, // Windows Vista with Service Pack 2
    { 6, 1,  7600}, // Windows 7 or Windows Server 2008 R2
    { 6, 1,  7601}, // Windows 7 with Service Pack 1 or Windows Server 2008 R2 with Service Pack 1
    { 6, 2,  9200}, // Windows 8 or Windows Server 2012
    { 6, 3,  9200}, // Windows 8.1 or Windows Server 2012 R2
    { 6, 3,  9600}, // Windows 8.1 with Update 1
    {10, 0, 10240}, // Windows 10 Version 1507 (TH1)
    {10, 0, 10586}, // Windows 10 Version 1511 (November Update) (TH2)
    {10, 0, 14393}, // Windows 10 Version 1607 (Anniversary Update) (RS1) or Windows Server 2016
    {10, 0, 15063}, // Windows 10 Version 1703 (Creators Update) (RS2)
    {10, 0, 16299}, // Windows 10 Version 1709 (Fall Creators Update) (RS3)
    {10, 0, 17134}, // Windows 10 Version 1803 (April 2018 Update) (RS4)
    {10, 0, 17763}, // Windows 10 Version 1809 (October 2018 Update) (RS5) or Windows Server 2019
    {10, 0, 18362}, // Windows 10 Version 1903 (May 2019 Update) (19H1)
    {10, 0, 18363}, // Windows 10 Version 1909 (November 2019 Update) (19H2)
    {10, 0, 19041}, // Windows 10 Version 2004 (May 2020 Update) (20H1)
    {10, 0, 19042}, // Windows 10 Version 20H2 (October 2020 Update) (20H2)
    {10, 0, 19043}, // Windows 10 Version 21H1 (May 2021 Update) (21H1)
    {10, 0, 19044}, // Windows 10 Version 21H2 (November 2021 Update) (21H2)
    {10, 0, 22000}, // Windows 11 Version 21H2 (21H2)
};
static_assert((sizeof(WindowsVersions) / sizeof(WindowsVersions[0])) == (static_cast<int>(WindowsVersion::_11_21H2) + 1));

} // namespace Global

namespace FramelessHelper::Core
{
FRAMELESSHELPER_CORE_API void initialize(const Global::Options options = {});
} // namespace FramelessHelper::Core

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::VersionNumber)
Q_DECLARE_METATYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::UserSettings)
Q_DECLARE_METATYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::SystemParameters)