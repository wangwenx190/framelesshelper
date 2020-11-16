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

#ifndef Q_OS_WINDOWS
#include <QGuiApplication>
#include <QWindow>
#endif

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include "framelesshelper.h"
#endif

#ifndef Q_OS_WINDOWS
namespace {

QWindow *toWindow(QObject *object)
{
    Q_ASSERT(object);
    return object->isWindowType() ? qobject_cast<QWindow *>(object) : nullptr;
}

using FLWM_CORE_DATA = struct _FLWM_CORE_DATA
{
    FramelessHelper framelessHelper;
};

} // namespace
#endif

#ifndef Q_OS_WINDOWS
Q_GLOBAL_STATIC(FLWM_CORE_DATA, coreData)
#endif

FramelessWindowsManager::FramelessWindowsManager() = default;

void FramelessWindowsManager::addWindow(WindowId window, const bool center)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::addFramelessWindow(window);
#else
    coreData()->framelessHelper.removeWindowFrame(window);
#endif
    if (center) {
        moveWindowToDesktopCenter(window);
    }
}

void FramelessWindowsManager::moveWindowToDesktopCenter(WindowId window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::moveWindowToDesktopCenter(window);
#else
    FramelessHelper::moveWindowToDesktopCenter(window);
#endif
}

void FramelessWindowsManager::addIgnoreArea(WindowId window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->ignoreAreas.append(area);
    }
#else
    coreData()->framelessHelper.addIgnoreArea(window, area);
#endif
}

void FramelessWindowsManager::addDraggableArea(WindowId window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->draggableAreas.append(area);
    }
#else
    coreData()->framelessHelper.addDraggableArea(window, area);
#endif
}

void FramelessWindowsManager::addIgnoreObject(WindowId window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->ignoreObjects.append(object);
    }
#else
    coreData()->framelessHelper.addIgnoreObject(window, object);
#endif
}

void FramelessWindowsManager::addDraggableObject(WindowId window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->draggableObjects.append(object);
    }
#else
    coreData()->framelessHelper.addDraggableObject(window, object);
#endif
}

int FramelessWindowsManager::getBorderWidth(WindowId window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::BorderWidth);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getBorderWidth();
#endif
}

void FramelessWindowsManager::setBorderWidth(WindowId window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->borderWidth = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setBorderWidth(value);
#endif
}

int FramelessWindowsManager::getBorderHeight(WindowId window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::BorderHeight);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getBorderHeight();
#endif
}

void FramelessWindowsManager::setBorderHeight(WindowId window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->borderHeight = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setBorderHeight(value);
#endif
}

int FramelessWindowsManager::getTitleBarHeight(WindowId window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(window,
                                                 WinNativeEventFilter::SystemMetric::TitleBarHeight);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getTitleBarHeight();
#endif
}

void FramelessWindowsManager::setTitleBarHeight(WindowId window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->titleBarHeight = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setTitleBarHeight(value);
#endif
}

bool FramelessWindowsManager::getResizable(WindowId window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? !data->fixedSize : true;
#else
    return coreData()->framelessHelper.getResizable(window);
#endif
}

void FramelessWindowsManager::setResizable(WindowId window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::setWindowResizable(window, value);
#else
    coreData()->framelessHelper.setResizable(window, value);
#endif
}

QSize FramelessWindowsManager::getMinimumSize(WindowId window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? data->minimumSize : QSize();
#else
    const auto win = toWindow(window);
    return win ? win->minimumSize() : QSize();
#endif
}

void FramelessWindowsManager::setMinimumSize(WindowId window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->minimumSize = value;
    }
#else
    const auto win = toWindow(window);
    if (win) {
        win->setMinimumSize(value);
    }
#endif
}

QSize FramelessWindowsManager::getMaximumSize(WindowId window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? data->maximumSize : QSize();
#else
    const auto win = toWindow(window);
    return win ? win->maximumSize() : QSize();
#endif
}

void FramelessWindowsManager::setMaximumSize(WindowId window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->maximumSize = value;
    }
#else
    const auto win = toWindow(window);
    if (win) {
        win->setMaximumSize(value);
    }
#endif
}

bool FramelessWindowsManager::getTitleBarEnabled(WindowId window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    return data ? !data->disableTitleBar : true;
#else
    return coreData()->framelessHelper.getTitleBarEnabled(window);
#endif
}

void FramelessWindowsManager::setTitleBarEnabled(WindowId window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::getWindowData(window);
    if (data) {
        data->disableTitleBar = !value;
    }
#else
    coreData()->framelessHelper.setTitleBarEnabled(window, value);
#endif
}
