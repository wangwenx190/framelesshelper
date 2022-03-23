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

#include "framelessquickhelper.h"
#include <QtQuick/qquickwindow.h>
#include <framelesswindowsmanager.h>
#include "framelessquickeventfilter.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessQuickHelper::FramelessQuickHelper(QObject *parent) : QObject(parent) {}

FramelessQuickHelper::~FramelessQuickHelper() = default;

void FramelessQuickHelper::addWindow(QQuickWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    FramelessWindowsManager::instance()->addWindow(window);
    FramelessQuickEventFilter::addWindow(window);
}

void FramelessQuickHelper::removeWindow(QQuickWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    FramelessQuickEventFilter::removeWindow(window);
    FramelessWindowsManager::instance()->removeWindow(window);
}

void FramelessQuickHelper::setTitleBarItem(QQuickWindow *window, QQuickItem *item)
{
    Q_ASSERT(window);
    Q_ASSERT(item);
    if (!window || !item) {
        return;
    }
    FramelessQuickEventFilter::setTitleBarItem(window, item);
}

void FramelessQuickHelper::setHitTestVisible(QQuickWindow *window, QQuickItem *item)
{
    Q_ASSERT(window);
    Q_ASSERT(item);
    if (!window || !item) {
        return;
    }
    FramelessQuickEventFilter::setHitTestVisible(window, item);
}

FRAMELESSHELPER_END_NAMESPACE
