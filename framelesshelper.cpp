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

#include "framelesshelper.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include "utilities.h"
#include <QtCore/qdebug.h>
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {}

int FramelessHelper::getBorderWidth() const
{
    return m_borderWidth;
}

void FramelessHelper::setBorderWidth(const int val)
{
    m_borderWidth = val;
}

int FramelessHelper::getBorderHeight() const
{
    return m_borderHeight;
}

void FramelessHelper::setBorderHeight(const int val)
{
    m_borderHeight = val;
}

int FramelessHelper::getTitleBarHeight() const
{
    return m_titleBarHeight;
}

void FramelessHelper::setTitleBarHeight(const int val)
{
    m_titleBarHeight = val;
}

bool FramelessHelper::getResizable(const QWindow *window) const
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    return !m_fixedSize.value(window);
}

void FramelessHelper::setResizable(const QWindow *window, const bool val)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    m_fixedSize.insert(window, !val);
}

void FramelessHelper::removeWindowFrame(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    // TODO: check whether these flags are correct for Linux and macOS.
    window->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint
                     | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint);
    // MouseTracking is always enabled for QWindow.
    window->installEventFilter(this);
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!object->isWindowType()) {
        return false;
    }
    // QWindow will always be a top level window. It can't
    // be anyone's child window.
    const auto currentWindow = qobject_cast<QWindow *>(object);
    const auto getWindowEdges =
        [this](const QPointF &point, const int ww, const int wh) -> Qt::Edges {
        if (point.y() <= m_borderHeight) {
            if (point.x() <= m_borderWidth) {
                return Qt::Edge::TopEdge | Qt::Edge::LeftEdge;
            }
            if (point.x() >= (ww - m_borderWidth)) {
                return Qt::Edge::TopEdge | Qt::Edge::RightEdge;
            }
            return Qt::Edge::TopEdge;
        }
        if (point.y() >= (wh - m_borderHeight)) {
            if (point.x() <= m_borderWidth) {
                return Qt::Edge::BottomEdge | Qt::Edge::LeftEdge;
            }
            if (point.x() >= (ww - m_borderWidth)) {
                return Qt::Edge::BottomEdge | Qt::Edge::RightEdge;
            }
            return Qt::Edge::BottomEdge;
        }
        if (point.x() <= m_borderWidth) {
            return Qt::Edge::LeftEdge;
        }
        if (point.x() >= (ww - m_borderWidth)) {
            return Qt::Edge::RightEdge;
        }
        return {};
    };
    const auto getCursorShape = [](const Qt::Edges edges) -> Qt::CursorShape {
        if ((edges.testFlag(Qt::Edge::TopEdge) && edges.testFlag(Qt::Edge::LeftEdge))
            || (edges.testFlag(Qt::Edge::BottomEdge) && edges.testFlag(Qt::Edge::RightEdge))) {
            return Qt::CursorShape::SizeFDiagCursor;
        }
        if ((edges.testFlag(Qt::Edge::TopEdge) && edges.testFlag(Qt::Edge::RightEdge))
            || (edges.testFlag(Qt::Edge::BottomEdge) && edges.testFlag(Qt::Edge::LeftEdge))) {
            return Qt::CursorShape::SizeBDiagCursor;
        }
        if (edges.testFlag(Qt::Edge::TopEdge) || edges.testFlag(Qt::Edge::BottomEdge)) {
            return Qt::CursorShape::SizeVerCursor;
        }
        if (edges.testFlag(Qt::Edge::LeftEdge) || edges.testFlag(Qt::Edge::RightEdge)) {
            return Qt::CursorShape::SizeHorCursor;
        }
        return Qt::CursorShape::ArrowCursor;
    };
    const auto isInTitlebarArea = [this](const QPointF &point, const QWindow *window) -> bool {
        Q_ASSERT(window);
        if (!window) {
            return false;
        }
        return (point.y() <= m_titleBarHeight) && !Utilities::isHitTestVisibleInChrome(window);
    };
    const auto moveOrResize =
        [this, &getWindowEdges, &isInTitlebarArea](const QPointF &point, QWindow *window) {
            Q_ASSERT(window);
            if (!window) {
                return;
            }
            const Qt::Edges edges = getWindowEdges(point, window->width(), window->height());
            if (edges == Qt::Edges{}) {
                if (isInTitlebarArea(point, window)) {
                    if (!window->startSystemMove()) {
                        // ### FIXME: TO BE IMPLEMENTED!
                        qWarning() << "Current OS doesn't support QWindow::startSystemMove().";
                    }
                }
            } else {
                if ((window->windowState() == Qt::WindowState::WindowNoState)
                    && !Utilities::isHitTestVisibleInChrome(window) && getResizable(window)) {
                    if (!window->startSystemResize(edges)) {
                        // ### FIXME: TO BE IMPLEMENTED!
                        qWarning() << "Current OS doesn't support QWindow::startSystemResize().";
                    }
                }
            }
        };
    const auto getMousePos = [](const QMouseEvent *e, const bool global) -> QPointF {
        Q_ASSERT(e);
        if (!e) {
            return {};
        }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return global ? e->globalPosition() : e->scenePosition();
#else
        return global ? e->screenPos() : e->windowPos();
#endif
    };
    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            break;
        }
        if (isInTitlebarArea(getMousePos(mouseEvent, false), currentWindow)) {
            if (currentWindow->windowState() == Qt::WindowState::WindowFullScreen) {
                break;
            }
            if (currentWindow->windowState() == Qt::WindowState::WindowMaximized) {
                currentWindow->showNormal();
            } else {
                currentWindow->showMaximized();
            }
            currentWindow->setCursor(Qt::CursorShape::ArrowCursor);
        }
    } break;
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            break;
        }
        moveOrResize(getMousePos(mouseEvent, false), currentWindow);
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if ((currentWindow->windowState() == Qt::WindowState::WindowNoState)
                && getResizable(currentWindow)) {
            currentWindow->setCursor(
                        getCursorShape(getWindowEdges(getMousePos(mouseEvent, false),
                                                      currentWindow->width(),
                                                      currentWindow->height())));
        }
    } break;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate: {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const auto point = static_cast<QTouchEvent *>(event)->points().first().position();
#else
        const auto point = static_cast<QTouchEvent *>(event)->touchPoints().first().pos();
#endif
        moveOrResize(point, currentWindow);
    } break;
    default:
        break;
    }
    return false;
}
#endif
