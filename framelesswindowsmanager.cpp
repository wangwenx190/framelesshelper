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

void FramelessWindowsManager::addWindow(const QWindow *window, const bool center)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::addFramelessWindow(window);
#else
    framelessHelper()->removeWindowFrame(window);
#endif
    if (center) {
        moveWindowToDesktopCenter(window);
    }
}

void FramelessWindowsManager::moveWindowToDesktopCenter(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::moveWindowToDesktopCenter(window);
#else
    FramelessHelper::moveWindowToDesktopCenter(window);
#endif
}

void FramelessWindowsManager::addIgnoreArea(const QWindow *window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->ignoreAreas.append(area);
    }
#else
    framelessHelper()->addIgnoreArea(window, area);
#endif
}

void FramelessWindowsManager::addDraggableArea(const QWindow *window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->draggableAreas.append(area);
    }
#else
    framelessHelper()->addDraggableArea(window, area);
#endif
}

void FramelessWindowsManager::addIgnoreObject(const QWindow *window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->ignoreObjects.append(object);
    }
#else
    framelessHelper()->addIgnoreObject(window, object);
#endif
}

void FramelessWindowsManager::addDraggableObject(const QWindow *window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->draggableObjects.append(object);
    }
#else
    framelessHelper()->addDraggableObject(window, object);
#endif
}

int FramelessWindowsManager::getBorderWidth(const QWindow *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::BorderWidth);
#else
    Q_UNUSED(window)
    return framelessHelper()->getBorderWidth();
#endif
}

void FramelessWindowsManager::setBorderWidth(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->borderWidth = value;
    }
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
                                                 WinNativeEventFilter::SystemMetric::BorderHeight);
#else
    Q_UNUSED(window)
    return framelessHelper()->getBorderHeight();
#endif
}

void FramelessWindowsManager::setBorderHeight(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->borderHeight = value;
    }
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
                                                 WinNativeEventFilter::SystemMetric::TitleBarHeight);
#else
    Q_UNUSED(window)
    return framelessHelper()->getTitleBarHeight();
#endif
}

void FramelessWindowsManager::setTitleBarHeight(const QWindow *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->titleBarHeight = value;
    }
#else
    Q_UNUSED(window)
    framelessHelper()->setTitleBarHeight(value);
#endif
}

bool FramelessWindowsManager::getResizable(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? !data->fixedSize : true;
#else
    return framelessHelper()->getResizable(window);
#endif
}

void FramelessWindowsManager::setResizable(const QWindow *window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::setWindowResizable(window, value);
#else
    framelessHelper()->setResizable(window, value);
#endif
}

QSize FramelessWindowsManager::getMinimumSize(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? data->minimumSize : QSize();
#else
    return window->minimumSize();
#endif
}

void FramelessWindowsManager::setMinimumSize(const QWindow *window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->minimumSize = value;
    }
#else
    window->setMinimumSize(value);
#endif
}

QSize FramelessWindowsManager::getMaximumSize(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? data->maximumSize : QSize();
#else
    return window->maximumSize();
#endif
}

void FramelessWindowsManager::setMaximumSize(const QWindow *window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->maximumSize = value;
    }
#else
    window->setMaximumSize(value);
#endif
}

bool FramelessWindowsManager::getTitleBarEnabled(const QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? !data->disableTitleBar : true;
#else
    return framelessHelper()->getTitleBarEnabled(window);
#endif
}

void FramelessWindowsManager::setTitleBarEnabled(const QWindow *window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->disableTitleBar = !value;
    }
#else
    framelessHelper()->setTitleBarEnabled(window, value);
#endif
}
