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

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#ifdef QT_WIDGETS_LIB
#include <QWidget>
#endif

#if (defined(Q_OS_WIN) || defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_WINRT)) \
    && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include "framelesshelper.h"
#endif

namespace {

void reportError()
{
    qFatal("Only top level QWidgets and QWindows are accepted.");
}

QScreen *getCurrentScreenFromWindow(QObject *window)
{
    Q_ASSERT(window);
    if (window->isWindowType()) {
        return qobject_cast<QWindow *>(window)->screen();
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return qobject_cast<QWidget *>(window)->screen();
#else
        QWindow *win = qobject_cast<QWidget *>(window)->windowHandle();
        return win ? win->screen() : nullptr;
#endif
    }
#endif
    else {
        reportError();
    }
    return nullptr;
}

#ifdef Q_OS_WINDOWS
void *getRawHandleFromWindow(QObject *window)
{
    Q_ASSERT(window);
    if (window->isWindowType()) {
        return reinterpret_cast<void *>(qobject_cast<QWindow *>(window)->winId());
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
        return reinterpret_cast<void *>(qobject_cast<QWidget *>(window)->winId());
    }
#endif
    else {
        reportError();
    }
    return nullptr;
}
#endif

#ifndef Q_OS_WINDOWS
using FLWM_CORE_DATA = struct _FLWM_CORE_DATA
{
    FramelessHelper framelessHelper;
};
#endif

} // namespace

#ifndef Q_OS_WINDOWS
Q_GLOBAL_STATIC(FLWM_CORE_DATA, coreData)
#endif

FramelessWindowsManager::FramelessWindowsManager() {}

void FramelessWindowsManager::addWindow(QObject *window, const bool center)
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

void FramelessWindowsManager::moveWindowToDesktopCenter(QObject *window, const bool realCenter)
{
    Q_ASSERT(window);
    if (realCenter) {
#ifdef Q_OS_WINDOWS
        WinNativeEventFilter::moveWindowToDesktopCenter(getRawHandleFromWindow(window));
#else
        FramelessHelper::moveWindowToDesktopCenter(window);
#endif
    } else {
        QSize windowSize = {}, screenSize = {};
        QRect screenGeometry = {};
        if (window->isWindowType()) {
            const auto win = qobject_cast<QWindow *>(window);
            if (win) {
                windowSize = win->size();
                screenSize = getDesktopAvailableSize(win);
                screenGeometry = getDesktopAvailableGeometry(win);
            }
        }
#ifdef QT_WIDGETS_LIB
        else if (window->isWidgetType()) {
            const auto widget = qobject_cast<QWidget *>(window);
            if (widget && widget->isTopLevel()) {
                windowSize = widget->size();
                screenSize = getDesktopAvailableSize(widget);
                screenGeometry = getDesktopAvailableGeometry(widget);
            }
        }
#endif
        else {
            reportError();
        }
        const int newX = qRound(static_cast<qreal>(screenSize.width() - windowSize.width()) / 2.0);
        const int newY = qRound(static_cast<qreal>(screenSize.height() - windowSize.height()) / 2.0);
        const int x = newX + screenGeometry.x();
        const int y = newY + screenGeometry.y();
        if (window->isWindowType()) {
            const auto win = qobject_cast<QWindow *>(window);
            if (win) {
                win->setX(x);
                win->setY(y);
            }
        }
#ifdef QT_WIDGETS_LIB
        else if (window->isWidgetType()) {
            const auto widget = qobject_cast<QWidget *>(window);
            if (widget && widget->isTopLevel()) {
                widget->move(x, y);
            }
        }
#endif
        else {
            reportError();
        }
    }
}

QSize FramelessWindowsManager::getDesktopSize(QObject *window)
{
    const QScreen *screen = window ? getCurrentScreenFromWindow(window)
                                   : QGuiApplication::primaryScreen();
    return screen ? screen->size() : QSize();
}

QRect FramelessWindowsManager::getDesktopAvailableGeometry(QObject *window)
{
    const QScreen *screen = window ? getCurrentScreenFromWindow(window)
                                   : QGuiApplication::primaryScreen();
    return screen ? screen->availableGeometry() : QRect();
}

QSize FramelessWindowsManager::getDesktopAvailableSize(QObject *window)
{
    const QScreen *screen = window ? getCurrentScreenFromWindow(window)
                                   : QGuiApplication::primaryScreen();
    return screen ? screen->availableSize() : QSize();
}

void FramelessWindowsManager::addIgnoreArea(QObject *window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->ignoreAreas.append(area);
    }
#else
    coreData()->framelessHelper.addIgnoreArea(window, area);
#endif
}

void FramelessWindowsManager::addDraggableArea(QObject *window, const QRect &area)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->draggableAreas.append(area);
    }
#else
    coreData()->framelessHelper.addDraggableArea(window, area);
#endif
}

void FramelessWindowsManager::addIgnoreObject(QObject *window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->ignoreObjects.append(object);
    }
#else
    coreData()->framelessHelper.addIgnoreObject(window, object);
#endif
}

void FramelessWindowsManager::addDraggableObject(QObject *window, QObject *object)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->draggableObjects.append(object);
    }
#else
    coreData()->framelessHelper.addDraggableObject(window, object);
#endif
}

int FramelessWindowsManager::getBorderWidth(QObject *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(getRawHandleFromWindow(window),
                                                 WinNativeEventFilter::SystemMetric::BorderWidth);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getBorderWidth();
#endif
}

void FramelessWindowsManager::setBorderWidth(QObject *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->borderWidth = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setBorderWidth(value);
#endif
}

int FramelessWindowsManager::getBorderHeight(QObject *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(getRawHandleFromWindow(window),
                                                 WinNativeEventFilter::SystemMetric::BorderHeight);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getBorderHeight();
#endif
}

void FramelessWindowsManager::setBorderHeight(QObject *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->borderHeight = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setBorderHeight(value);
#endif
}

int FramelessWindowsManager::getTitleBarHeight(QObject *window)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    return WinNativeEventFilter::getSystemMetric(getRawHandleFromWindow(window),
                                                 WinNativeEventFilter::SystemMetric::TitleBarHeight);
#else
    Q_UNUSED(window)
    return coreData()->framelessHelper.getTitleBarHeight();
#endif
}

void FramelessWindowsManager::setTitleBarHeight(QObject *window, const int value)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(window);
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->titleBarHeight = value;
    }
#else
    Q_UNUSED(window)
    coreData()->framelessHelper.setTitleBarHeight(value);
#endif
}

bool FramelessWindowsManager::getResizable(QObject *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    return data ? !data->fixedSize : true;
#else
    return coreData()->framelessHelper.getResizable(window);
#endif
}

void FramelessWindowsManager::setResizable(QObject *window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::setWindowResizable(getRawHandleFromWindow(window), value);
#else
    coreData()->framelessHelper.setResizable(window, value);
#endif
}

QSize FramelessWindowsManager::getMinimumSize(QObject *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    return data ? data->minimumSize : QSize();
#else
    if (window->isWindowType()) {
        return qobject_cast<QWindow *>(window)->minimumSize();
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
        return qobject_cast<QWidget *>(window)->minimumSize();
    }
#endif
    else {
        reportError();
    }
    return {};
#endif
}

void FramelessWindowsManager::setMinimumSize(QObject *window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->minimumSize = value;
    }
#else
    if (window->isWindowType()) {
        qobject_cast<QWindow *>(window)->setMinimumSize(value);
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
        qobject_cast<QWidget *>(window)->setMinimumSize(value);
    }
#endif
    else {
        reportError();
    }
#endif
}

QSize FramelessWindowsManager::getMaximumSize(QObject *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    return data ? data->maximumSize : QSize();
#else
    if (window->isWindowType()) {
        return qobject_cast<QWindow *>(window)->maximumSize();
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
        return qobject_cast<QWidget *>(window)->maximumSize();
    }
#endif
    else {
        reportError();
    }
    return {};
#endif
}

void FramelessWindowsManager::setMaximumSize(QObject *window, const QSize &value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->maximumSize = value;
    }
#else
    if (window->isWindowType()) {
        qobject_cast<QWindow *>(window)->setMaximumSize(value);
    }
#ifdef QT_WIDGETS_LIB
    else if (window->isWidgetType()) {
        qobject_cast<QWidget *>(window)->setMaximumSize(value);
    }
#endif
    else {
        reportError();
    }
#endif
}

bool FramelessWindowsManager::getTitleBarEnabled(QObject *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    return data ? !data->disableTitleBar : true;
#else
    return coreData()->framelessHelper.getTitleBarEnabled(window);
#endif
}

void FramelessWindowsManager::setTitleBarEnabled(QObject *window, const bool value)
{
    Q_ASSERT(window);
#ifdef Q_OS_WINDOWS
    const auto data = WinNativeEventFilter::windowData(window);
    if (data) {
        data->disableTitleBar = !value;
    }
#else
    coreData()->framelessHelper.setTitleBarEnabled(window, value);
#endif
}
