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

#include "framelessquickhelper.h"
#include "framelesswindowsmanager.h"
#include <QtQuick/qquickwindow.h>
#include "utilities.h"

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent) : QQuickItem(parent)
{
    startTimer(500);
}

int FramelessQuickHelper::borderWidth() const
{
    return FramelessWindowsManager::getBorderWidth(window());
}

void FramelessQuickHelper::setBorderWidth(const int val)
{
    FramelessWindowsManager::setBorderWidth(window(), val);
    Q_EMIT borderWidthChanged(val);
}

int FramelessQuickHelper::borderHeight() const
{
    return FramelessWindowsManager::getBorderHeight(window());
}

void FramelessQuickHelper::setBorderHeight(const int val)
{
    FramelessWindowsManager::setBorderHeight(window(), val);
    Q_EMIT borderHeightChanged(val);
}

int FramelessQuickHelper::titleBarHeight() const
{
    return FramelessWindowsManager::getTitleBarHeight(window());
}

void FramelessQuickHelper::setTitleBarHeight(const int val)
{
    FramelessWindowsManager::setTitleBarHeight(window(), val);
    Q_EMIT titleBarHeightChanged(val);
}

bool FramelessQuickHelper::resizable() const
{
    return FramelessWindowsManager::getResizable(window());
}

void FramelessQuickHelper::setResizable(const bool val)
{
    FramelessWindowsManager::setResizable(window(), val);
    Q_EMIT resizableChanged(val);
}

bool FramelessQuickHelper::lightThemeEnabled() const
{
    return Utilities::isLightThemeEnabled();
}

bool FramelessQuickHelper::darkThemeEnabled() const
{
    return Utilities::isDarkThemeEnabled();
}

#ifdef Q_OS_WINDOWS
bool FramelessQuickHelper::colorizationEnabled() const
{
    return Utilities::isColorizationEnabled();
}

QColor FramelessQuickHelper::colorizationColor() const
{
    return Utilities::getColorizationColor();
}

bool FramelessQuickHelper::highContrastModeEnabled() const
{
    return Utilities::isHighContrastModeEnabled();
}

bool FramelessQuickHelper::darkFrameEnabled() const
{
    return Utilities::isDarkFrameEnabled(window());
}

bool FramelessQuickHelper::transparencyEffectEnabled() const
{
    return Utilities::isTransparencyEffectEnabled();
}
#endif

void FramelessQuickHelper::removeWindowFrame()
{
    FramelessWindowsManager::addWindow(window());
}

void FramelessQuickHelper::addIgnoreObject(QQuickItem *val)
{
    Q_ASSERT(val);
    if (!val) {
        return;
    }
    FramelessWindowsManager::addIgnoreObject(window(), val);
}

void FramelessQuickHelper::timerEvent(QTimerEvent *event)
{
    QQuickItem::timerEvent(event);
    Q_EMIT lightThemeEnabledChanged(lightThemeEnabled());
    Q_EMIT darkThemeEnabledChanged(darkThemeEnabled());
#ifdef Q_OS_WINDOWS
    Q_EMIT colorizationEnabledChanged(colorizationEnabled());
    Q_EMIT colorizationColorChanged(colorizationColor());
    Q_EMIT highContrastModeEnabledChanged(highContrastModeEnabled());
    Q_EMIT darkFrameEnabledChanged(darkFrameEnabled());
    Q_EMIT transparencyEffectEnabledChanged(transparencyEffectEnabled());
#endif
}

void FramelessQuickHelper::setBlurEffectEnabled(const bool enabled, const QColor &gradientColor)
{
    Utilities::setBlurEffectEnabled(window(), enabled, gradientColor);
}
