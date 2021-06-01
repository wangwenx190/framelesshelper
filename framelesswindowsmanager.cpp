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

#include "framelesswindowsmanager.h"
#include <QtCore/qvariant.h>
#include <QtGui/qwindow.h>
#include "utilities.h"
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
#include "framelesshelper.h"
#else
#include <QtGui/qscreen.h>
#include "framelesshelper_win32.h"
#endif

#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
Q_GLOBAL_STATIC(FramelessHelper, framelessHelper)
#endif

FramelessWindowsManager::FramelessWindowsManager() = default;

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelper()->removeWindowFrame(window);
#else
    FramelessHelperWin::addFramelessWindow(window);
    // Work-around a Win32 multi-monitor bug.
    QObject::connect(window, &QWindow::screenChanged, [window](QScreen *screen){
        Q_UNUSED(screen);
        window->resize(window->size());
    });
#endif
}

void FramelessWindowsManager::setHitTestVisibleInChrome(QWindow *window, QObject *object, const bool value)
{
    Q_ASSERT(window);
    Q_ASSERT(object);
    if (!window || !object) {
        return;
    }
    if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
        qWarning() << object << "is not a QWidget or QQuickItem.";
        return;
    }
    const QObject *nativeParent = Utilities::getNativeParent(object);
    Q_ASSERT(nativeParent);
    if (!nativeParent) {
        return;
    }
    window->setProperty(_flh_global::_flh_nativeParent_flag, QVariant::fromValue(nativeParent));
    object->setProperty(_flh_global::_flh_hitTestVisibleInChrome_flag, value);
}

int FramelessWindowsManager::getBorderWidth(const QWindow *window)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    return framelessHelper()->getBorderWidth();
#else
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    return Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderWidth, false);
#endif
}

void FramelessWindowsManager::setBorderWidth(QWindow *window, const int value)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    framelessHelper()->setBorderWidth(value);
#else
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    FramelessHelperWin::setBorderWidth(window, value);
#endif
}

int FramelessWindowsManager::getBorderHeight(const QWindow *window)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    return framelessHelper()->getBorderHeight();
#else
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    return Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderHeight, false);
#endif
}

void FramelessWindowsManager::setBorderHeight(QWindow *window, const int value)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    framelessHelper()->setBorderHeight(value);
#else
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    FramelessHelperWin::setBorderHeight(window, value);
#endif
}

int FramelessWindowsManager::getTitleBarHeight(const QWindow *window)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    return framelessHelper()->getTitleBarHeight();
#else
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    return Utilities::getSystemMetric(window, Utilities::SystemMetric::TitleBarHeight, false);
#endif
}

void FramelessWindowsManager::setTitleBarHeight(QWindow *window, const int value)
{
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    Q_UNUSED(window);
    framelessHelper()->setTitleBarHeight(value);
#else
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    FramelessHelperWin::setTitleBarHeight(window, value);
#endif
}

bool FramelessWindowsManager::getResizable(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    return framelessHelper()->getResizable(window);
#else
    return !Utilities::isWindowFixedSize(window);
#endif
}

void FramelessWindowsManager::setResizable(QWindow *window, const bool value)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelper()->setResizable(window, value);
#else
    window->setFlag(Qt::MSWindowsFixedSizeDialogHint, !value);
#endif
}
