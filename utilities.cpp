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
#include <QtCore/qvariant.h>
#include <QtGui/qguiapplication.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

QWindow *Utilities::findWindow(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
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
    return qEnvironmentVariableIsSet(Constants::kUseNativeTitleBarFlag);
}

bool Utilities::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef Q_OS_WINDOWS
    if (window->flags() & Qt::MSWindowsFixedSizeDialogHint) {
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

bool Utilities::isHitTestVisibleInChrome(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const auto objs = qvariant_cast<QObjectList>(window->property(Constants::kHitTestVisibleInChromeFlag));
    if (objs.isEmpty()) {
        return false;
    }
    for (auto &&obj : qAsConst(objs)) {
        if (!obj || !(obj->isWidgetType() || obj->inherits("QQuickItem"))) {
            continue;
        }
        if (!obj->property("visible").toBool()) {
            continue;
        }
        const QPointF originPoint = mapOriginPointToWindow(obj);
        const qreal width = obj->property("width").toReal();
        const qreal height = obj->property("height").toReal();
        const QRectF rect = {originPoint.x(), originPoint.y(), width, height};
        if (rect.contains(QCursor::pos(window->screen()))) {
            return true;
        }
    }
    return false;
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

FRAMELESSHELPER_END_NAMESPACE
