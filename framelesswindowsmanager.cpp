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
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindow.h>
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
#include "framelesshelper.h"
#else
#include <QtGui/qscreen.h>
#include "framelesshelper_win32.h"
#endif
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
Q_GLOBAL_STATIC(FramelessHelper, framelessHelperUnix)
#endif

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (!QCoreApplication::testAttribute(Qt::AA_DontCreateNativeWidgetSiblings)) {
        QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelperUnix()->removeWindowFrame(window);
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
    auto objList = qvariant_cast<QObjectList>(window->property(Constants::kHitTestVisibleInChromeFlag));
    if (value) {
        if (objList.isEmpty() || !objList.contains(object)) {
            objList.append(object);
        }
    } else {
        if (!objList.isEmpty() && objList.contains(object)) {
            objList.removeAll(object);
        }
    }
    window->setProperty(Constants::kHitTestVisibleInChromeFlag, QVariant::fromValue(objList));
}

int FramelessWindowsManager::getResizeBorderThickness(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return 8;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    const int value = window->property(Constants::kResizeBorderThicknessFlag).toInt();
    return value <= 0 ? 8 : value;
#else
    return Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, false);
#endif
}

void FramelessWindowsManager::setResizeBorderThickness(QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window || (value <= 0)) {
        return;
    }
    window->setProperty(Constants::kResizeBorderThicknessFlag, value);
}

int FramelessWindowsManager::getTitleBarHeight(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return 31;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    const int value = window->property(Constants::kTitleBarHeightFlag).toInt();
    return value <= 0 ? 31 : value;
#else
    return Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, false);
#endif
}

void FramelessWindowsManager::setTitleBarHeight(QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window || (value <= 0)) {
        return;
    }
    window->setProperty(Constants::kTitleBarHeightFlag, value);
}

bool FramelessWindowsManager::getResizable(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    return !window->property(Constants::kWindowFixedSizeFlag).toBool();
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
    window->setProperty(Constants::kWindowFixedSizeFlag, !value);
#else
    window->setFlag(Qt::MSWindowsFixedSizeDialogHint, !value);
#endif
}

void FramelessWindowsManager::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef FRAMELESSHELPER_USE_UNIX_VERSION
    framelessHelperUnix()->bringBackWindowFrame(window);
#else
    FramelessHelperWin::removeFramelessWindow(window);
#endif
}

bool FramelessWindowsManager::isWindowFrameless(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    return window->property(Constants::kFramelessModeFlag).toBool();
}

FRAMELESSHELPER_END_NAMESPACE
