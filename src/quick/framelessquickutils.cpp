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

#include "framelessquickutils.h"
#include <framelesswindowsmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

Q_GLOBAL_STATIC(FramelessQuickUtils, g_quickUtils)

FramelessQuickUtils::FramelessQuickUtils(QObject *parent) : QObject(parent)
{
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, [this](){
        Q_EMIT darkModeEnabledChanged();
        Q_EMIT systemAccentColorChanged();
        Q_EMIT titleBarColorizedChanged();
    });
}

FramelessQuickUtils::~FramelessQuickUtils() = default;

FramelessQuickUtils *FramelessQuickUtils::instance()
{
    return g_quickUtils();
}

qreal FramelessQuickUtils::titleBarHeight()
{
    return kDefaultTitleBarHeight;
}

bool FramelessQuickUtils::frameBorderVisible()
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater());
#else
    return false;
#endif
}

qreal FramelessQuickUtils::frameBorderThickness()
{
#ifdef Q_OS_WINDOWS
    return kDefaultWindowFrameBorderThickness;
#else
    return 0;
#endif
}

bool FramelessQuickUtils::darkModeEnabled()
{
    return Utils::shouldAppsUseDarkMode();
}

QColor FramelessQuickUtils::systemAccentColor()
{
#ifdef Q_OS_WINDOWS
    return Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
    return Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
    return Utils::getControlsAccentColor();
#endif
}

bool FramelessQuickUtils::titleBarColorized()
{
    return Utils::isTitleBarColorized();
}

QColor FramelessQuickUtils::defaultSystemLightColor()
{
    return kDefaultSystemLightColor;
}

QColor FramelessQuickUtils::defaultSystemDarkColor()
{
    return kDefaultSystemDarkColor;
}

QSizeF FramelessQuickUtils::defaultSystemButtonSize()
{
    return kDefaultSystemButtonSize;
}

QSizeF FramelessQuickUtils::defaultSystemButtonIconSize()
{
    return kDefaultSystemButtonIconSize;
}

QColor FramelessQuickUtils::defaultSystemButtonBackgroundColor()
{
    return kDefaultSystemButtonBackgroundColor;
}

QColor FramelessQuickUtils::defaultSystemCloseButtonBackgroundColor()
{
    return kDefaultSystemCloseButtonBackgroundColor;
}

QColor FramelessQuickUtils::getSystemButtonBackgroundColor(const SystemButtonType button, const ButtonState state)
{
    return Utils::calculateSystemButtonBackgroundColor(button, state);
}

FRAMELESSHELPER_END_NAMESPACE
