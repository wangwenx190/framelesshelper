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

#include "framelesshelper.h"
#include <QtCore/qmutex.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

struct UnixHelper
{
    QMutex mutex = {};
    QWindowList acceptableWindows = {};

    explicit UnixHelper() = default;
    ~UnixHelper() = default;

private:
    Q_DISABLE_COPY_MOVE(UnixHelper)
};

Q_GLOBAL_STATIC(UnixHelper, g_unixHelper)

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {}

FramelessHelper::~FramelessHelper() = default;

void FramelessHelper::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_unixHelper()->mutex.lock();
    if (g_unixHelper()->acceptableWindows.contains(window)) {
        g_unixHelper()->mutex.unlock();
        return;
    }
    g_unixHelper()->acceptableWindows.append(window);
    g_unixHelper()->mutex.unlock();
    window->setFlags(window->flags() | Qt::FramelessWindowHint);
    window->installEventFilter(this);
}

void FramelessHelper::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_unixHelper()->mutex.lock();
    if (!g_unixHelper()->acceptableWindows.contains(window)) {
        g_unixHelper()->mutex.unlock();
        return;
    }
    g_unixHelper()->acceptableWindows.removeAll(window);
    g_unixHelper()->mutex.unlock();
    window->removeEventFilter(this);
    window->setFlags(window->flags() & ~Qt::FramelessWindowHint);
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
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
    g_unixHelper()->mutex.lock();
    if (!g_unixHelper()->acceptableWindows.contains(window)) {
        g_unixHelper()->mutex.unlock();
        return false;
    }
    g_unixHelper()->mutex.unlock();
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
