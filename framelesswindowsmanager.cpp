/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include "framelesswindowsmanager.h"

#include <QWindow>

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include "framelesshelper.h"
#endif

#ifndef Q_OS_WINDOWS
Q_GLOBAL_STATIC(FramelessHelper, framelessHelper)
#endif

FramelessWindowsManager::FramelessWindowsManager() = default;

void FramelessWindowsManager::addWindow(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::addFramelessWindow(const_cast<QWindow *>(window));
#else
    framelessHelper()->removeWindowFrame(const_cast<QWindow *>(window));
#endif
}

void FramelessWindowsManager::addIgnoreObject(const QWindow *window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    QObjectList objects = WinNativeEventFilter::getIgnoredObjects(window);
    objects.append(object);
    WinNativeEventFilter::setIgnoredObjects(const_cast<QWindow *>(window), objects);
#else
    framelessHelper()->addIgnoreObject(window, object);
#endif
}

int FramelessWindowsManager::getBorderWidth(const QWindow *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::BorderWidth,
                                                 false);
#else
    Q_UNUSED(window)
    return framelessHelper()->getBorderWidth();
#endif
}

void FramelessWindowsManager::setBorderWidth(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    WinNativeEventFilter::setBorderWidth(const_cast<QWindow *>(window), value);
#else
    Q_UNUSED(window)
    framelessHelper()->setBorderWidth(value);
#endif
}

int FramelessWindowsManager::getBorderHeight(const QWindow *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::BorderHeight,
                                                 false);
#else
    Q_UNUSED(window)
    return framelessHelper()->getBorderHeight();
#endif
}

void FramelessWindowsManager::setBorderHeight(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    WinNativeEventFilter::setBorderHeight(const_cast<QWindow *>(window), value);
#else
    Q_UNUSED(window)
    framelessHelper()->setBorderHeight(value);
#endif
}

int FramelessWindowsManager::getTitleBarHeight(const QWindow *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::TitleBarHeight,
                                                 false);
#else
    Q_UNUSED(window)
    return framelessHelper()->getTitleBarHeight();
#endif
}

void FramelessWindowsManager::setTitleBarHeight(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    WinNativeEventFilter::setTitleBarHeight(const_cast<QWindow *>(window), value);
#else
    Q_UNUSED(window)
    framelessHelper()->setTitleBarHeight(value);
#endif
}

bool FramelessWindowsManager::getResizable(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    if (window->flags().testFlag(Qt::MSWindowsFixedSizeDialogHint)) {
        return false;
    }
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && minSize == maxSize) {
        return false;
    }
    return true;
#else
    return framelessHelper()->getResizable(window);
#endif
}

void FramelessWindowsManager::setResizable(const QWindow *window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const_cast<QWindow *>(window)->setFlag(Qt::MSWindowsFixedSizeDialogHint, !value);
#else
    framelessHelper()->setResizable(window, value);
#endif
}
