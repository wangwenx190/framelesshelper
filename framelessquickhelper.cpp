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

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent) : QQuickItem(parent)
{
}

qreal FramelessQuickHelper::resizeBorderThickness() const
{
    return FramelessWindowsManager::getResizeBorderThickness(window());
}

void FramelessQuickHelper::setResizeBorderThickness(const qreal val)
{
    FramelessWindowsManager::setResizeBorderThickness(window(), qRound(val));
    Q_EMIT resizeBorderThicknessChanged(val);
}

qreal FramelessQuickHelper::titleBarHeight() const
{
    return FramelessWindowsManager::getTitleBarHeight(window());
}

void FramelessQuickHelper::setTitleBarHeight(const qreal val)
{
    FramelessWindowsManager::setTitleBarHeight(window(), qRound(val));
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

void FramelessQuickHelper::removeWindowFrame()
{
    FramelessWindowsManager::addWindow(window());
}

void FramelessQuickHelper::bringBackWindowFrame()
{
    FramelessWindowsManager::removeWindow(window());
}

bool FramelessQuickHelper::isWindowFrameless() const
{
    return FramelessWindowsManager::isWindowFrameless(window());
}

void FramelessQuickHelper::setHitTestVisibleInChrome(QQuickItem *item, const bool visible)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    FramelessWindowsManager::setHitTestVisibleInChrome(window(), item, visible);
}

FRAMELESSHELPER_END_NAMESPACE
