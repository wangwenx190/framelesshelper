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
#include <QtCore/qsize.h>
#include <QtGui/qcolor.h>

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
#  define Q_NAMESPACE_EXPORT(ns) Q_NAMESPACE
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#  define Q_NODISCARD [[nodiscard]]
#else
#  define Q_NODISCARD
#endif

#ifndef FRAMELESSHELPER_NAMESPACE
#  define FRAMELESSHELPER_NAMESPACE __flh_ns
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

Q_NAMESPACE_EXPORT(FRAMELESSHELPER_CORE_API)

[[maybe_unused]] static constexpr const int kDefaultResizeBorderThickness = 8;
[[maybe_unused]] static constexpr const int kDefaultCaptionHeight = 23;
[[maybe_unused]] static constexpr const int kDefaultTitleBarHeight = 30;
[[maybe_unused]] static constexpr const int kDefaultWindowFrameBorderThickness = 1;

[[maybe_unused]] static const QColor kDefaultSystemLightColor = QStringLiteral("#f0f0f0");
[[maybe_unused]] static const QColor kDefaultSystemDarkColor = QStringLiteral("#202020");

[[maybe_unused]] static constexpr const QSize kDefaultSystemButtonSize = {int(qRound(qreal(kDefaultTitleBarHeight) * 1.5)), kDefaultTitleBarHeight};
[[maybe_unused]] static constexpr const QSize kDefaultSystemButtonIconSize = {16, 16};

[[maybe_unused]] static constexpr const char kInternalOptionsFlag[] = "FRAMELESSHELPER_INTERNAL_OPTIONS";
[[maybe_unused]] static constexpr const char kUsePureQtImplFlag[] = "FRAMELESSHELPER_PURE_QT_IMPLEMENTATION";
[[maybe_unused]] static constexpr const char kForceHideFrameBorderFlag[] = "FRAMELESSHELPER_FORCE_HIDE_FRAME_BORDER";
[[maybe_unused]] static constexpr const char kForceShowFrameBorderFlag[] = "FRAMELESSHELPER_FORCE_SHOW_FRAME_BORDER";

[[maybe_unused]] static const QString kConfigFileName = QStringLiteral(".framelesshelper.ini");
[[maybe_unused]] static const QString kUsePureQtImplKeyPath = QStringLiteral("Options/UsePureQtImplementation");
[[maybe_unused]] static const QString kForceHideFrameBorderKeyPath = QStringLiteral("Options/ForceHideFrameBorder");
[[maybe_unused]] static const QString kForceShowFrameBorderKeyPath = QStringLiteral("Options/ForceShowFrameBorder");

enum class Option : int
{
    Default                               = 0x00000000, // Default placeholder, have no effect.
    ForceHideWindowFrameBorder            = 0x00000001, // Windows only, force hide the window frame border even on Windows 10 and onwards.
    ForceShowWindowFrameBorder            = 0x00000002, // Windows only, force show the window frame border even on Windows 7 (~ 8.1).
    DontDrawTopWindowFrameBorder          = 0x00000004, // Windows only, don't draw the top window frame border even if the window frame border is visible.
    EnableRoundedWindowCorners            = 0x00000008, // Not implemented yet.
    TransparentWindowBackground           = 0x00000010, // Not implemented yet.
    MaximizeButtonDocking                 = 0x00000020, // Windows only, enable the window docking feature introduced in Windows 11.
    UseStandardWindowLayout               = 0x00000040, // The standard window layout is a titlebar always on top and fill the window width.
    BeCompatibleWithQtFramelessWindowHint = 0x00000080, // Windows only, make the code compatible with Qt::FramelessWindowHint. Don't use this option unless you really need that flag.
    DontTouchQtInternals                  = 0x00000100, // Windows only, don't modify Qt's internal data.
    DontTouchWindowFrameBorderColor       = 0x00000200, // Windows only, don't change the window frame border color.
    DontInstallSystemMenuHook             = 0x00000400, // Windows only, don't install the system menu hook.
    DisableSystemMenu                     = 0x00000800, // Windows only, don't open the system menu when right clicks the titlebar.
    NoDoubleClickMaximizeToggle           = 0x00001000, // Don't toggle the maximize state when double clicks the titlebar.
    DisableResizing                       = 0x00002000, // Disable resizing of the window.
    DisableDragging                       = 0x00004000, // Disable dragging through the titlebar of the window.
    DontTouchCursorShape                  = 0x00008000  // Don't change the cursor shape while the mouse is hovering above the window.
};
Q_DECLARE_FLAGS(Options, Option)
Q_FLAG_NS(Options)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

FRAMELESSHELPER_END_NAMESPACE
