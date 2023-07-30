/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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
#include <QtCore/qmath.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtGui/qcolor.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_NAMESPACE
class QEvent;
class QEnterEvent;
QT_END_NAMESPACE

#ifndef FRAMELESSHELPER_CORE_API
#  ifdef FRAMELESSHELPER_CORE_STATIC
#    define FRAMELESSHELPER_CORE_API
#  else // !FRAMELESSHELPER_CORE_STATIC
#    ifdef FRAMELESSHELPER_CORE_LIBRARY
#      define FRAMELESSHELPER_CORE_API Q_DECL_EXPORT
#    else // !FRAMELESSHELPER_CORE_LIBRARY
#      define FRAMELESSHELPER_CORE_API Q_DECL_IMPORT
#    endif // FRAMELESSHELPER_CORE_LIBRARY
#  endif // FRAMELESSHELPER_CORE_STATIC
#endif // FRAMELESSHELPER_CORE_API

#if (defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS))
#  define Q_OS_WINDOWS // Since 5.14
#endif

#ifndef Q_DISABLE_COPY_MOVE // Since 5.13
#  define Q_DISABLE_COPY_MOVE(Class) \
      Q_DISABLE_COPY(Class) \
      Class(Class &&) = delete; \
      Class &operator=(Class &&) = delete;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
   using QStringView = const QString &;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
#  define qExchange(a, b) std::exchange(a, b)
#endif

#ifndef Q_NAMESPACE_EXPORT // Since 5.14
#  define Q_NAMESPACE_EXPORT(...) Q_NAMESPACE
#endif

// QColor can't be constexpr before 5.14
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#  define Q_COLOR_CONSTEXPR constexpr
#else
#  define Q_COLOR_CONSTEXPR
#endif

// MOC can't handle C++ attributes before 5.15.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#  define Q_NODISCARD [[nodiscard]]
#  define Q_MAYBE_UNUSED [[maybe_unused]]
#else
#  define Q_NODISCARD
#  define Q_MAYBE_UNUSED
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
   using QT_NATIVE_EVENT_RESULT_TYPE = qintptr;
   using QT_ENTER_EVENT_TYPE = QEnterEvent;
#else
   using QT_NATIVE_EVENT_RESULT_TYPE = long;
   using QT_ENTER_EVENT_TYPE = QEvent;
#endif

// QLatin1StringView can't be constexpr until Qt6?
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  define Q_STRING_CONSTEXPR constexpr
#else
#  define Q_STRING_CONSTEXPR
#endif

#ifndef QUtf8String
#  define QUtf8String(str) QString::fromUtf8(str)
#endif

#ifndef Q_GADGET_EXPORT // Since 6.3
#  define Q_GADGET_EXPORT(...) Q_GADGET
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
  using namespace Qt::Literals::StringLiterals;
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY_LITERAL
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
#    define FRAMELESSHELPER_BYTEARRAY_LITERAL(ba) ba##_ba
#  elif (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#    define FRAMELESSHELPER_BYTEARRAY_LITERAL(ba) ba##_qba
#  else
#    define FRAMELESSHELPER_BYTEARRAY_LITERAL(ba) QByteArrayLiteral(ba)
#  endif
#endif

#ifndef FRAMELESSHELPER_STRING_LITERAL
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
#    define FRAMELESSHELPER_STRING_LITERAL(str) u##str##_s
#  elif (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#    define FRAMELESSHELPER_STRING_LITERAL(str) u##str##_qs
#  else
#    define FRAMELESSHELPER_STRING_LITERAL(str) QStringLiteral(str)
#  endif
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY
#  define FRAMELESSHELPER_BYTEARRAY(ba) ba
#endif

#ifndef FRAMELESSHELPER_STRING
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
#    define FRAMELESSHELPER_STRING(str) str##_L1
#  else
#    define FRAMELESSHELPER_STRING(str) QLatin1String(str)
#  endif
#endif

#ifndef FRAMELESSHELPER_STRING_TYPE
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
#    define FRAMELESSHELPER_STRING_TYPE QLatin1StringView
#  else
#    define FRAMELESSHELPER_STRING_TYPE QLatin1String
#  endif
#endif

#ifndef Q_UNREACHABLE_RETURN // Since 6.5
#  define Q_UNREACHABLE_RETURN(...) \
     do { \
         Q_UNREACHABLE(); \
         return __VA_ARGS__; \
     } while (false)
#endif

#ifndef FRAMELESSHELPER_BYTEARRAY_CONSTANT2
#  define FRAMELESSHELPER_BYTEARRAY_CONSTANT2(name, ba) \
     [[maybe_unused]] static constexpr const auto k##name = FRAMELESSHELPER_BYTEARRAY(ba);
#endif

#ifndef FRAMELESSHELPER_STRING_CONSTANT2
#  define FRAMELESSHELPER_STRING_CONSTANT2(name, str) \
     [[maybe_unused]] static Q_STRING_CONSTEXPR const auto k##name = FRAMELESSHELPER_STRING(str);
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

#ifndef FRAMELESSHELPER_MAKE_VERSION
#  define FRAMELESSHELPER_MAKE_VERSION(Major, Minor, Patch) \
     ((((Major) & 0xff) << 24) | (((Minor) & 0xff) << 16) | (((Patch) & 0xff) << 8))
#endif

#ifndef FRAMELESSHELPER_EXTRACT_VERSION
#  define FRAMELESSHELPER_EXTRACT_VERSION(Version, Major, Minor, Patch) \
     { \
         (Major) = (((Version) & 0xff) >> 24); \
         (Minor) = (((Version) & 0xff) >> 16); \
         (Patch) = (((Version) & 0xff) >> 8); \
     }
#endif

#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
// Call this function in your main() function if you are using FramelessHelper as a static library,
// it can make sure the resources bundled in the static library are correctly initialized.
// NOTE: This function is intentionally not inside any namespaces.
FRAMELESSHELPER_CORE_API void framelesshelpercore_initResource();
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE

FRAMELESSHELPER_BEGIN_NAMESPACE

#include "framelesshelper.version"

[[maybe_unused]] inline constexpr const int FRAMELESSHELPER_VERSION =
      FRAMELESSHELPER_MAKE_VERSION(
          FRAMELESSHELPER_VERSION_MAJOR,
          FRAMELESSHELPER_VERSION_MINOR,
          FRAMELESSHELPER_VERSION_PATCH);

namespace Global
{

Q_NAMESPACE_EXPORT(FRAMELESSHELPER_CORE_API)

[[maybe_unused]] inline constexpr const int kDefaultResizeBorderThickness = 8;
[[maybe_unused]] inline constexpr const int kDefaultCaptionHeight = 23;
[[maybe_unused]] inline constexpr const int kDefaultTitleBarHeight = 32;
[[maybe_unused]] inline constexpr const int kDefaultExtendedTitleBarHeight = 48;
[[maybe_unused]] inline constexpr const int kDefaultWindowFrameBorderThickness = 1;
[[maybe_unused]] inline constexpr const int kDefaultTitleBarFontPointSize = 11;
[[maybe_unused]] inline constexpr const int kDefaultTitleBarContentsMargin = 10;
[[maybe_unused]] inline constexpr const int kMacOSChromeButtonAreaWidth = 60;
[[maybe_unused]] inline constexpr const QSize kDefaultWindowIconSize = {16, 16};
// We have to use "qRound()" here because "std::round()" is not constexpr, yet.
[[maybe_unused]] inline constexpr const QSize kDefaultSystemButtonSize = {qRound(qreal(kDefaultTitleBarHeight) * 1.5), kDefaultTitleBarHeight};
[[maybe_unused]] inline constexpr const QSize kDefaultSystemButtonIconSize = kDefaultWindowIconSize;
[[maybe_unused]] inline constexpr const QSize kDefaultWindowSize = {160, 160}; // Value taken from Windows QPA.

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#  define kDefaultBlackColor QColorConstants::Black
#  define kDefaultWhiteColor QColorConstants::White
#  define kDefaultTransparentColor QColorConstants::Transparent
#  define kDefaultDarkGrayColor QColorConstants::DarkGray
#else // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
   [[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultBlackColor = {0, 0, 0}; // #000000
   [[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultWhiteColor = {255, 255, 255}; // #FFFFFF
   [[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultTransparentColor = {0, 0, 0, 0};
   [[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultDarkGrayColor = {169, 169, 169}; // #A9A9A9
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))

[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultSystemLightColor = {240, 240, 240}; // #F0F0F0
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultSystemDarkColor = {32, 32, 32}; // #202020
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultFrameBorderActiveColor = {77, 77, 77}; // #4D4D4D
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultFrameBorderInactiveColorDark = {87, 89, 89}; // #575959
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultFrameBorderInactiveColorLight = {166, 166, 166}; // #A6A6A6
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultSystemButtonBackgroundColor = {204, 204, 204}; // #CCCCCC
[[maybe_unused]] inline Q_COLOR_CONSTEXPR const QColor kDefaultSystemCloseButtonBackgroundColor = {232, 17, 35}; // #E81123

[[maybe_unused]] inline constexpr const char kDontOverrideCursorVar[] = "FRAMELESSHELPER_DONT_OVERRIDE_CURSOR";
[[maybe_unused]] inline constexpr const char kDontToggleMaximizeVar[] = "FRAMELESSHELPER_DONT_TOGGLE_MAXIMIZE";
[[maybe_unused]] inline constexpr const char kSysMenuDisableMinimizeVar[] = "FRAMELESSHELPER_SYSTEM_MENU_DISABLE_MINIMIZE";
[[maybe_unused]] inline constexpr const char kSysMenuDisableMaximizeVar[] = "FRAMELESSHELPER_SYSTEM_MENU_DISABLE_MAXIMIZE";
[[maybe_unused]] inline constexpr const char kSysMenuDisableRestoreVar[] = "FRAMELESSHELPER_SYSTEM_MENU_DISABLE_RESTORE";

enum class Option : quint8
{
    UseCrossPlatformQtImplementation,
    ForceHideWindowFrameBorder,
    ForceShowWindowFrameBorder,
    DisableWindowsSnapLayout,
    WindowUseRoundCorners,
    CenterWindowBeforeShow,
    EnableBlurBehindWindow,
    ForceNonNativeBackgroundBlur,
    DisableLazyInitializationForMicaMaterial,
    ForceNativeBackgroundBlur,
    Last = ForceNativeBackgroundBlur
};
Q_ENUM_NS(Option)

enum class SystemTheme : quint8
{
    Unknown,
    Light,
    Dark,
    HighContrast
};
Q_ENUM_NS(SystemTheme)

enum class SystemButtonType : quint8
{
    Unknown,
    WindowIcon,
    Help,
    Minimize,
    Maximize,
    Restore,
    Close,
    Last = Close
};
Q_ENUM_NS(SystemButtonType)

#ifdef Q_OS_WINDOWS
enum class DwmColorizationArea : quint8
{
    None,
    StartMenu_TaskBar_ActionCenter,
    TitleBar_WindowBorder,
    All
};
Q_ENUM_NS(DwmColorizationArea)
#endif // Q_OS_WINDOWS

enum class ButtonState : quint8
{
    Normal,
    Hovered,
    Pressed,
    Released
};
Q_ENUM_NS(ButtonState)

#ifdef Q_OS_WINDOWS
enum class WindowsVersion : quint8
{
    _2000,
    _XP,
    _XP_64,
    _Vista,
    _Vista_SP1,
    _Vista_SP2,
    _7,
    _7_SP1,
    _8,
    _8_1,
    _8_1_Update1,
    _10_1507,
    _10_1511,
    _10_1607,
    _10_1703,
    _10_1709,
    _10_1803,
    _10_1809,
    _10_1903,
    _10_1909,
    _10_2004,
    _10_20H2,
    _10_21H1,
    _10_21H2,
    _10_22H2,
    _11_21H2,
    _11_22H2,

    _WS_03 = _XP_64, // Windows Server 2003
    _10 = _10_1507,
    _11 = _11_21H2,

    Latest = _11_22H2
};
Q_ENUM_NS(WindowsVersion)
#endif // Q_OS_WINDOWS

enum class BlurMode : quint8
{
    Disable, // Do not enable blur behind window
    Default, // Use platform default blur mode
    Windows_Aero, // Windows only, use the traditional DWM blur
    Windows_Acrylic, // Windows only, use the Acrylic blur
    Windows_Mica, // Windows only, use the Mica material
    Windows_MicaAlt // Windows only, use the Mica Alt material
};
Q_ENUM_NS(BlurMode)

enum class WallpaperAspectStyle : quint8
{
    Fill, // Keep aspect ratio to fill, expand/crop if necessary.
    Fit, // Keep aspect ratio to fill, but don't expand/crop.
    Stretch, // Ignore aspect ratio to fill.
    Tile,
    Center,
    Span // ???
};
Q_ENUM_NS(WallpaperAspectStyle)

#ifdef Q_OS_WINDOWS
enum class RegistryRootKey : quint8
{
    ClassesRoot,
    CurrentUser,
    LocalMachine,
    Users,
    PerformanceData,
    CurrentConfig,
    DynData,
    CurrentUserLocalSettings,
    PerformanceText,
    PerformanceNlsText
};
Q_ENUM_NS(RegistryRootKey)
#endif // Q_OS_WINDOWS

enum class WindowEdge : quint8
{
    Left   = 1 << 0,
    Top    = 1 << 1,
    Right  = 1 << 2,
    Bottom = 1 << 3
};
Q_ENUM_NS(WindowEdge)
Q_DECLARE_FLAGS(WindowEdges, WindowEdge)
Q_FLAG_NS(WindowEdges)
Q_DECLARE_OPERATORS_FOR_FLAGS(WindowEdges)

#ifdef Q_OS_WINDOWS
enum class DpiAwareness : quint8
{
    Unknown,
    Unaware,
    System,
    PerMonitor,
    PerMonitorVersion2,
    Unaware_GdiScaled
};
Q_ENUM_NS(DpiAwareness)
#endif // Q_OS_WINDOWS

enum class WindowCornerStyle : quint8
{
    Default,
    Square,
    Round
};
Q_ENUM_NS(WindowCornerStyle)

struct VersionInfo
{
    int version = 0;
    const char *version_str = nullptr;
    const char *commit = nullptr;
    const char *compileDateTime = nullptr;
    const char *compiler = nullptr;
    bool isDebug = false;
    bool isStatic = false;
};

struct Dpi
{
    quint32 x = 0;
    quint32 y = 0;

    [[nodiscard]] friend constexpr bool operator==(const Dpi &lhs, const Dpi &rhs) noexcept
    {
        return ((lhs.x == rhs.x) && (lhs.y == rhs.y));
    }

    [[nodiscard]] friend constexpr bool operator!=(const Dpi &lhs, const Dpi &rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }
};

} // namespace Global

namespace FramelessHelper::Core
{
FRAMELESSHELPER_CORE_API void initialize();
FRAMELESSHELPER_CORE_API void uninitialize();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::VersionInfo version();
FRAMELESSHELPER_CORE_API void setApplicationOSThemeAware();
FRAMELESSHELPER_CORE_API void outputLogo();
} // namespace FramelessHelper::Core

FRAMELESSHELPER_END_NAMESPACE

#ifndef QT_NO_DEBUG_STREAM
QT_BEGIN_NAMESPACE
FRAMELESSHELPER_CORE_API QDebug operator<<(QDebug, const FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::VersionInfo &);
FRAMELESSHELPER_CORE_API QDebug operator<<(QDebug, const FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Dpi &);
QT_END_NAMESPACE
#endif // QT_NO_DEBUG_STREAM
