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

bool Utilities::isMouseInSpecificObjects(const QPointF &mousePos, const QObjectList &objects, const qreal dpr)
{
    if (mousePos.isNull()) {
        qWarning() << "Mouse position point is null.";
        return false;
    }
    if (objects.isEmpty()) {
        qWarning() << "Object list is empty.";
        return false;
    }
    for (auto &&object : qAsConst(objects)) {
        if (!object) {
            qWarning() << "Object pointer is null.";
            continue;
        }
        if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
            qWarning() << object << "is not a QWidget or QQuickItem!";
            continue;
        }
        if (!object->property("visible").toBool()) {
            qDebug() << "Skipping invisible object" << object;
            continue;
        }
        const auto mapOriginPointToWindow = [](const QObject *obj) -> QPointF {
            Q_ASSERT(obj);
            if (!obj) {
                return {};
            }
            QPointF point = {obj->property("x").toReal(), obj->property("y").toReal()};
            for (QObject *parent = obj->parent(); parent; parent = parent->parent()) {
                point += {parent->property("x").toReal(), parent->property("y").toReal()};
                if (parent->isWindowType()) {
                    break;
                }
            }
            return point;
        };
        const QPointF originPoint = mapOriginPointToWindow(object);
        const qreal width = object->property("width").toReal();
        const qreal height = object->property("height").toReal();
        const QRectF rect = {originPoint.x() * dpr, originPoint.y() * dpr, width * dpr, height * dpr};
        if (rect.contains(mousePos)) {
            return true;
        }
    }
    return false;
}
