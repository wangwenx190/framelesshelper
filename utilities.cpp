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

#include "utilities.h"
#include <QtCore/qdebug.h>
#include <QtGui/qguiapplication.h>

QWindow *Utilities::findWindow(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == winId) {
                return window;
            }
        }
    }
    return nullptr;
}

bool Utilities::shouldUseNativeTitleBar()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_useNativeTitleBar_flag);
}

bool Utilities::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef Q_OS_WINDOWS
    if (window->flags().testFlag(Qt::MSWindowsFixedSizeDialogHint)) {
        return true;
    }
#endif
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    return false;
}

QPointF Utilities::getGlobalMousePosition(const QWindow *window)
{
    if (window) {
        return (QCursor::pos(window->screen()) * window->devicePixelRatio());
    } else {
        const qreal dpr = 1.0; // TODO
        return (QCursor::pos() * dpr);
    }
}

bool Utilities::isHitTestVisibleInChrome(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    QObjectList objs = {};
    const auto target = qvariant_cast<const QObject *>(window->property(_flh_global::_flh_nativeParent_flag));
    objs = target ? target->findChildren<QObject *>() : window->findChildren<QObject *>();
    if (objs.isEmpty()) {
        return false;
    }
    for (auto &&obj : qAsConst(objs)) {
        if (!obj || !(obj->isWidgetType() || obj->inherits("QQuickItem"))) {
            continue;
        }
        if (!obj->property(_flh_global::_flh_hitTestVisibleInChrome_flag).toBool() || !obj->property("visible").toBool()) {
            continue;
        }
        const QPointF originPoint = mapOriginPointToWindow(obj);
        const qreal width = obj->property("width").toReal();
        const qreal height = obj->property("height").toReal();
        const qreal dpr = window->devicePixelRatio();
        const QRectF rect = {originPoint.x() * dpr, originPoint.y() * dpr, width * dpr, height * dpr};
        if (rect.contains(getGlobalMousePosition(window))) {
            return true;
        }
    }
    return false;
}

QObject *Utilities::getNativeParent(const QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return nullptr;
    }
    if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
        qWarning() << object << "is not a QWidget or a QQuickItem.";
        return nullptr;
    }
    QObject *parent = object->parent();
    while (parent) {
        QObject *p = parent->parent();
        if (!p || p->isWindowType()) {
            return parent;
        }
        parent = p;
    }
    return parent;
}

QPointF Utilities::mapOriginPointToWindow(const QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return {};
    }
    if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
        qWarning() << object << "is not a QWidget or a QQuickItem.";
        return {};
    }
    QPointF point = {object->property("x").toReal(), object->property("y").toReal()};
    for (QObject *parent = object->parent(); parent; parent = parent->parent()) {
        point += {parent->property("x").toReal(), parent->property("y").toReal()};
        if (parent->isWindowType()) {
            break;
        }
    }
    return point;
}
