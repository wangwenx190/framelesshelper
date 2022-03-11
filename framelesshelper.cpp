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
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include "framelesswindowsmanager.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

static constexpr const int g_resizeBorderThickness = kDefaultResizeBorderThicknessAero;

[[nodiscard]] static inline Qt::CursorShape calculateCursorShape
    (const QWindow * const window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    if (((pos.x() < g_resizeBorderThickness) && (pos.y() < g_resizeBorderThickness))
        || ((pos.x() >= (window->width() - g_resizeBorderThickness)) && (pos.y() >= (window->height() - g_resizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((pos.x() >= (window->width() - g_resizeBorderThickness)) && (pos.y() < g_resizeBorderThickness))
        || ((pos.x() < g_resizeBorderThickness) && (pos.y() >= (window->height() - g_resizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((pos.x() < g_resizeBorderThickness) || (pos.x() >= (window->width() - g_resizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((pos.y() < g_resizeBorderThickness) || (pos.y() >= (window->height() - g_resizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

[[nodiscard]] static inline Qt::Edges calculateWindowEdges
    (const QWindow * const window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    if (pos.x() < g_resizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= (window->width() - g_resizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() < g_resizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= (window->height() - g_resizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {}

FramelessHelper::~FramelessHelper() = default;

void FramelessHelper::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    window->setFlags(window->flags() | Qt::FramelessWindowHint);
    window->installEventFilter(this);
}

void FramelessHelper::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
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
    if ((type != QEvent::MouseButtonDblClick) && (type != QEvent::MouseButtonPress)
            && (type != QEvent::MouseMove)) {
        return false;
    }
    const auto window = qobject_cast<QWindow *>(object);
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPointF localPos = mouseEvent->position();
#else
    const QPointF localPos = mouseEvent->windowPos();
#endif
    if (type == QEvent::MouseMove) {
        const Qt::CursorShape cs = calculateCursorShape(window, localPos);
        if (cs == Qt::ArrowCursor) {
            window->unsetCursor();
        } else {
            window->setCursor(cs);
        }
    } else if (type == QEvent::MouseButtonPress) {
        const Qt::Edges edges = calculateWindowEdges(window, localPos);
        if (edges != Qt::Edges{}) {
            if (!window->startSystemResize(edges)) {
                qWarning() << "Current platform doesn't support \"QWindow::startSystemResize()\".";
            }
        }
    }
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
