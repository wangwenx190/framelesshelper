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
#include "utils.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

struct QtHelper
{
    QMutex mutex = {};
    QHash<QWindow *, FramelessHelperQt *> qtFramelessHelpers = {};
    QHash<QWindow *, Options> options = {};

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
    if (g_qtHelper()->qtFramelessHelpers.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return;
    }
    const auto options = qvariant_cast<Options>(window->property(kInternalOptionsFlag));
    g_qtHelper()->options.insert(window, options);
    // Give it a parent so that it can be deleted even if we forget to do so.
    const auto qtFramelessHelper = new FramelessHelperQt(window);
    g_qtHelper()->qtFramelessHelpers.insert(window, qtFramelessHelper);
    g_qtHelper()->mutex.unlock();
    window->setFlags(window->flags() | Qt::FramelessWindowHint);
    window->installEventFilter(qtFramelessHelper);
}

void FramelessHelperQt::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_qtHelper()->mutex.lock();
    if (!g_qtHelper()->qtFramelessHelpers.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return;
    }
    g_qtHelper()->options.remove(window);
    FramelessHelperQt *qtFramelessHelper = g_qtHelper()->qtFramelessHelpers.value(window);
    g_qtHelper()->qtFramelessHelpers.remove(window);
    g_qtHelper()->mutex.unlock();
    window->removeEventFilter(qtFramelessHelper);
    delete qtFramelessHelper;
    qtFramelessHelper = nullptr;
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
    if (!g_qtHelper()->qtFramelessHelpers.contains(window)) {
        g_qtHelper()->mutex.unlock();
        return false;
    }
    const Options options = g_qtHelper()->options.value(window);
    g_qtHelper()->mutex.unlock();
    if (Utils::isWindowFixedSize(window)) {
        return false;
    }
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = mouseEvent->scenePosition().toPoint();
#else
    const QPoint scenePos = mouseEvent->windowPos().toPoint();
#endif
    switch (type) {
    case QEvent::MouseMove: {
        if (options & Option::DontTouchCursorShape) {
            return false;
        }
        const Qt::CursorShape cs = Utils::calculateCursorShape(window, scenePos);
        if (cs == Qt::ArrowCursor) {
            window->unsetCursor();
        } else {
            window->setCursor(cs);
        }
    } break;
    case QEvent::MouseButtonPress: {
        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }
        const Qt::Edges edges = Utils::calculateWindowEdges(window, scenePos);
        if (edges == Qt::Edges{}) {
            return false;
        }
        Utils::startSystemResize(window, edges);
        return true;
    }
    default:
        break;
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
