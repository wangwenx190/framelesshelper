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

#include <framelesshelpercore_global.h>
#if __has_include(<QtQml/qqmlregistration.h>)
#  include <QtQml/qqmlregistration.h>
#else
#  include <QtQml/qqml.h>
#endif

#ifndef FRAMELESSHELPER_QUICK_API
#  ifdef FRAMELESSHELPER_QUICK_STATIC
#    define FRAMELESSHELPER_QUICK_API
#  else
#    ifdef FRAMELESSHELPER_QUICK_LIBRARY
#      define FRAMELESSHELPER_QUICK_API Q_DECL_EXPORT
#    else
#      define FRAMELESSHELPER_QUICK_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#ifndef FRAMELESSHELPER_QUICK_ENUM_VALUE
#  define FRAMELESSHELPER_QUICK_ENUM_VALUE(Enum, Value) \
     Value = static_cast<int>(FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Enum::Value),
#endif

#ifndef FRAMELESSHELPER_ENUM_CORE_TO_QUICK
#  define FRAMELESSHELPER_ENUM_CORE_TO_QUICK(Enum, Value) \
     static_cast<FRAMELESSHELPER_PREPEND_NAMESPACE(QuickGlobal)::Enum>(static_cast<int>(Value))
#endif

#ifndef FRAMELESSHELPER_ENUM_QUICK_TO_CORE
#  define FRAMELESSHELPER_ENUM_QUICK_TO_CORE(Enum, Value) \
     static_cast<FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Enum>(static_cast<int>(Value))
#endif

#ifndef FRAMELESSHELPER_FLAGS_CORE_TO_QUICK
#  define FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Enum, Value, In, Out) \
     if (In & FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Enum::Value) { \
         Out |= FRAMELESSHELPER_PREPEND_NAMESPACE(QuickGlobal)::Enum::Value; \
     }
#endif

#ifndef FRAMELESSHELPER_FLAGS_QUICK_TO_CORE
#  define FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Enum, Value, In, Out) \
     if (In & FRAMELESSHELPER_PREPEND_NAMESPACE(QuickGlobal)::Enum::Value) { \
         Out |= FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Enum::Value; \
     }
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static constexpr const char FRAMELESSHELPER_QUICK_URI[] = "org.wangwenx190.FramelessHelper";

struct FRAMELESSHELPER_QUICK_API QuickGlobal
{
    enum class SystemTheme
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemTheme, Unknown)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemTheme, Light)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemTheme, Dark)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemTheme, HighContrast)
    };
    Q_ENUM(SystemTheme)

    enum class SystemButtonType
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Unknown)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, WindowIcon)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Help)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Minimize)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Maximize)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Restore)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(SystemButtonType, Close)
    };
    Q_ENUM(SystemButtonType)

    enum class DwmColorizationArea
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(DwmColorizationArea, None_)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(DwmColorizationArea, StartMenu_TaskBar_ActionCenter)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(DwmColorizationArea, TitleBar_WindowBorder)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(DwmColorizationArea, All)
    };
    Q_ENUM(DwmColorizationArea)

    enum class Anchor
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, Top)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, Bottom)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, Left)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, Right)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, HorizontalCenter)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, VerticalCenter)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(Anchor, Center)
    };
    Q_ENUM(Anchor)

    enum class ButtonState
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ButtonState, Unspecified)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ButtonState, Hovered)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ButtonState, Pressed)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ButtonState, Clicked)
    };
    Q_ENUM(ButtonState)

    enum class WindowsVersion
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _2000)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _XP)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _XP_64)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _Vista)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _Vista_SP1)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _Vista_SP2)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _7)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _7_SP1)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _8)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _8_1)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _8_1_Update1)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1507)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1511)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1607)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1703)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1709)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1803)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1809)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1903)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_1909)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_2004)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_20H2)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_21H1)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _10_21H2)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _11_21H2)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, _11_22H2)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(WindowsVersion, Latest)
    };
    Q_ENUM(WindowsVersion)
    static_assert(static_cast<int>(WindowsVersion::Latest) == static_cast<int>(Global::WindowsVersion::Latest));

    enum class ApplicationType
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ApplicationType, Widgets)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ApplicationType, Quick)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(ApplicationType, Hybrid)
    };
    Q_ENUM(ApplicationType)

    enum class BlurMode
    {
        FRAMELESSHELPER_QUICK_ENUM_VALUE(BlurMode, Disable)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(BlurMode, Default)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(BlurMode, Windows_Aero)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(BlurMode, Windows_Acrylic)
        FRAMELESSHELPER_QUICK_ENUM_VALUE(BlurMode, Windows_Mica)
    };
    Q_ENUM(BlurMode)

private:
    Q_GADGET
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessHelperConstants)
#endif
#ifdef QML_UNCREATABLE
    QML_UNCREATABLE("The FramelessHelperConstants namespace is not creatable, you can only use it to access it's enums.")
#endif
};

namespace FramelessHelper::Quick
{
FRAMELESSHELPER_QUICK_API void initialize();
FRAMELESSHELPER_QUICK_API void uninitialize();
} // namespace FramelessHelper::Quick

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(QuickGlobal))
