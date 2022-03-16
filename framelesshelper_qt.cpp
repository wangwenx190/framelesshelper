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

#include "framelesshelper_qt.h"
#include <QtCore/qmutex.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

struct QtHelper
{
    QMutex mutex = {};
    QWindowList acceptableWindows = {};

    explicit QtHelper() = default;
    ~QtHelper() = default;

private:
    Q_DISABLE_COPY_MOVE(QtHelper)
};

Q_GLOBAL_STATIC(QtHelper, g_qtHelper)

FramelessHelperQt::FramelessHelperQt(QObject *parent) : QObject(parent) {}

FramelessHelperQt::~FramelessHelperQt() = default;

void FramelessHelperQt::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_qtHelper()->mutex.lock();
    if (g_qtHelper()->acceptableWindows.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return;
    }
    g_qtHelper()->acceptableWindows.append(window);
    g_qtHelper()->mutex.unlock();
    window->setFlags(window->flags() | Qt::FramelessWindowHint);
    window->installEventFilter(this);
}

void FramelessHelperQt::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_qtHelper()->mutex.lock();
    if (!g_qtHelper()->acceptableWindows.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return;
    }
    g_qtHelper()->acceptableWindows.removeAll(window);
    g_qtHelper()->mutex.unlock();
    window->removeEventFilter(this);
    window->setFlags(window->flags() & ~Qt::FramelessWindowHint);
}

bool FramelessHelperQt::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    // Only monitor window events.
    if (!object->isWindowType()) {
        return false;
    }
    const QEvent::Type type = event->type();
    // We are only interested in mouse events.
    if ((type != QEvent::MouseButtonPress) && (type != QEvent::MouseMove)) {
        return false;
    }
    const auto window = qobject_cast<QWindow *>(object);
    g_qtHelper()->mutex.lock();
    if (!g_qtHelper()->acceptableWindows.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return false;
    }
    g_qtHelper()->mutex.unlock();
    if (Utilities::isWindowFixedSize(window)) {
        return false;
    }
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPointF localPos = mouseEvent->position();
#else
    const QPointF localPos = mouseEvent->windowPos();
#endif
    if (type == QEvent::MouseMove) {
        const Qt::CursorShape cs = Utilities::calculateCursorShape(window, localPos);
        if (cs == Qt::ArrowCursor) {
            window->unsetCursor();
        } else {
            window->setCursor(cs);
        }
    } else if (type == QEvent::MouseButtonPress) {
        const Qt::Edges edges = Utilities::calculateWindowEdges(window, localPos);
        if (edges != Qt::Edges{}) {
            Utilities::startSystemResize(window, edges);
        }
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
