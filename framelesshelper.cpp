/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include <QDebug>
#include <QGuiApplication>
#include <QMargins>
#ifdef QT_WIDGETS_LIB
#include <QWidget>
#endif
#ifdef QT_QUICK_LIB
#include <QQuickItem>
#endif
#include <QEvent>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>

Q_DECLARE_METATYPE(QMargins)

namespace {

QWindow *getWindowHandle(QObject *const val) {
    if (val) {
        const auto validWindow = [](QWindow *const window) -> QWindow * {
            return (window && window->handle()) ? window : nullptr;
        };
        if (val->isWindowType()) {
            return validWindow(qobject_cast<QWindow *>(val));
        }
#ifdef QT_WIDGETS_LIB
        else if (val->isWidgetType()) {
            const auto widget = qobject_cast<QWidget *>(val);
            if (widget) {
                return validWindow(widget->windowHandle());
            }
        }
#endif
        else {
            qWarning().noquote() << "Can't acquire the window handle: only "
                                    "QWidget and QWindow are accepted.";
        }
    }
    return nullptr;
}

} // namespace

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {
    // ### TODO: The default border width and height on Windows is 8 pixels if
    // the scale factor is 1.0. Don't know how to acquire these values on UNIX
    // platforms through native API.
    m_borderWidth = 8;
    m_borderHeight = 8;
    m_titleBarHeight = 30;
}

void FramelessHelper::updateQtFrame(QWindow *const window,
                                    const int titleBarHeight) {
    if (window && (titleBarHeight > 0)) {
        // Reduce top frame to zero since we paint it ourselves. Use
        // device pixel to avoid rounding errors.
        const QMargins margins = {0, -titleBarHeight, 0, 0};
        const QVariant marginsVar = QVariant::fromValue(margins);
        // The dynamic property takes effect when creating the platform
        // window.
        window->setProperty("_q_windowsCustomMargins", marginsVar);
        // If a platform window exists, change via native interface.
        QPlatformWindow *platformWindow = window->handle();
        if (platformWindow) {
            QGuiApplication::platformNativeInterface()->setWindowProperty(
                platformWindow, QString::fromUtf8("WindowsCustomMargins"),
                marginsVar);
        }
    }
}

int FramelessHelper::getBorderWidth() const { return m_borderWidth; }

void FramelessHelper::setBorderWidth(const int val) { m_borderWidth = val; }

int FramelessHelper::getBorderHeight() const { return m_borderHeight; }

void FramelessHelper::setBorderHeight(const int val) { m_borderHeight = val; }

int FramelessHelper::getTitleBarHeight() const { return m_titleBarHeight; }

void FramelessHelper::setTitleBarHeight(const int val) {
    m_titleBarHeight = val;
}

QVector<QRect> FramelessHelper::getIgnoreAreas(QObject *const obj) const {
    if (!obj) {
        return {};
    }
    return m_ignoreAreas.value(obj);
}

void FramelessHelper::setIgnoreAreas(QObject *const obj,
                                     const QVector<QRect> &val) {
    if (obj) {
        m_ignoreAreas[obj] = val;
    }
}

QVector<QRect> FramelessHelper::getDraggableAreas(QObject *const obj) const {
    if (!obj) {
        return {};
    }
    return m_draggableAreas.value(obj);
}

void FramelessHelper::setDraggableAreas(QObject *const obj,
                                        const QVector<QRect> &val) {
    if (obj) {
        m_draggableAreas[obj] = val;
    }
}

QVector<QPointer<QObject>>
FramelessHelper::getIgnoreObjects(QObject *const obj) const {
    if (!obj) {
        return {};
    }
    return m_ignoreObjects.value(obj);
}

void FramelessHelper::setIgnoreObjects(QObject *const obj,
                                       const QVector<QPointer<QObject>> &val) {
    if (obj) {
        m_ignoreObjects[obj] = val;
    }
}

QVector<QPointer<QObject>>
FramelessHelper::getDraggableObjects(QObject *const obj) const {
    if (!obj) {
        return {};
    }
    return m_draggableObjects.value(obj);
}

void FramelessHelper::setDraggableObjects(
    QObject *const obj, const QVector<QPointer<QObject>> &val) {
    if (obj) {
        m_draggableObjects[obj] = val;
    }
}

void FramelessHelper::removeWindowFrame(QObject *const obj) {
    if (obj) {
        // Don't miss the Qt::Window flag.
        const Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;
        QWindow *window = getWindowHandle(obj);
        if (window) {
            window->setFlags(flags);
            // MouseTracking is always enabled for QWindow.
            window->installEventFilter(this);
            updateQtFrame(window, m_titleBarHeight);
        }
#ifdef QT_WIDGETS_LIB
        else {
            const auto widget = qobject_cast<QWidget *>(obj);
            if (widget) {
                widget->setWindowFlags(flags);
                // We can't get MouseMove events if MouseTracking is
                // disabled.
                widget->setMouseTracking(true);
                widget->installEventFilter(this);
            }
        }
#endif
    }
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event) {
    const auto isWindowTopLevel = [](QObject *const window) -> bool {
        if (window) {
            if (window->isWindowType()) {
                return qobject_cast<QWindow *>(window)->isTopLevel();
            }
#ifdef QT_WIDGETS_LIB
            else if (window->isWidgetType()) {
                return qobject_cast<QWidget *>(window)->isTopLevel();
            }
#endif
        }
        return false;
    };
    if (!object || !isWindowTopLevel(object)) {
        event->ignore();
        return false;
    }
    const auto getWindowEdges = [this](const QPointF &point, const int ww,
                                       const int wh) -> Qt::Edges {
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
        if ((edges.testFlag(Qt::Edge::TopEdge) &&
             edges.testFlag(Qt::Edge::LeftEdge)) ||
            (edges.testFlag(Qt::Edge::BottomEdge) &&
             edges.testFlag(Qt::Edge::RightEdge))) {
            return Qt::CursorShape::SizeFDiagCursor;
        }
        if ((edges.testFlag(Qt::Edge::TopEdge) &&
             edges.testFlag(Qt::Edge::RightEdge)) ||
            (edges.testFlag(Qt::Edge::BottomEdge) &&
             edges.testFlag(Qt::Edge::LeftEdge))) {
            return Qt::CursorShape::SizeBDiagCursor;
        }
        if (edges.testFlag(Qt::Edge::TopEdge) ||
            edges.testFlag(Qt::Edge::BottomEdge)) {
            return Qt::CursorShape::SizeVerCursor;
        }
        if (edges.testFlag(Qt::Edge::LeftEdge) ||
            edges.testFlag(Qt::Edge::RightEdge)) {
            return Qt::CursorShape::SizeHorCursor;
        }
        return Qt::CursorShape::ArrowCursor;
    };
    const auto isInSpecificAreas = [](const int x, const int y,
                                      const QVector<QRect> &areas) -> bool {
        if (!areas.isEmpty()) {
            for (auto &&area : qAsConst(areas)) {
                if (area.contains(x, y)) {
                    return true;
                }
            }
        }
        return false;
    };
    const auto isInSpecificObjects =
        [](const int x, const int y,
           const QVector<QPointer<QObject>> &objects) -> bool {
        if (!objects.isEmpty()) {
            for (auto &&obj : qAsConst(objects)) {
                if (!obj) {
                    continue;
                }
#ifdef QT_WIDGETS_LIB
                const auto widget = qobject_cast<QWidget *>(obj);
                if (widget) {
                    if (QRect(widget->x(), widget->y(), widget->width(),
                              widget->height())
                            .contains(x, y)) {
                        return true;
                    }
                }
#endif
#ifdef QT_QUICK_LIB
                const auto quickItem = qobject_cast<QQuickItem *>(obj);
                if (quickItem) {
                    if (QRect(quickItem->x(), quickItem->y(),
                              quickItem->width(), quickItem->height())
                            .contains(x, y)) {
                        return true;
                    }
                }
#endif
            }
        }
        return false;
    };
    const auto isInIgnoreAreas =
        [this, &isInSpecificAreas](const QPointF &point,
                                   QObject *const window) -> bool {
        if (!window) {
            return false;
        }
        return isInSpecificAreas(point.x(), point.y(), getIgnoreAreas(window));
    };
    const auto isInIgnoreObjects =
        [this, &isInSpecificObjects](const QPointF &point,
                                     QObject *const window) -> bool {
#if defined(QT_WIDGETS_LIB) || defined(QT_QUICK_LIB)
        if (!window) {
            return false;
        }
        return isInSpecificObjects(point.x(), point.y(),
                                   getIgnoreObjects(window));
#else
        Q_UNUSED(point)
        Q_UNUSED(window)
        return false;
#endif
    };
    const auto isInDraggableAreas =
        [this, &isInSpecificAreas](const QPointF &point,
                                   QObject *const window) -> bool {
        if (!window) {
            return false;
        }
        const auto areas = getDraggableAreas(window);
        return (areas.isEmpty()
                    ? true
                    : isInSpecificAreas(point.x(), point.y(), areas));
    };
    const auto isInDraggableObjects =
        [this, &isInSpecificObjects](const QPointF &point,
                                     QObject *const window) -> bool {
#if defined(QT_WIDGETS_LIB) || defined(QT_QUICK_LIB)
        if (!window) {
            return false;
        }
        const auto objs = getDraggableObjects(window);
        return (objs.isEmpty()
                    ? true
                    : isInSpecificObjects(point.x(), point.y(), objs));
#else
        Q_UNUSED(point)
        Q_UNUSED(window)
        return true;
#endif
    };
    const auto isResizePermitted =
        [&isInIgnoreAreas, &isInIgnoreObjects](const QPointF &point,
                                               QObject *const window) -> bool {
        if (!window) {
            return false;
        }
        return (!isInIgnoreAreas(point, window) &&
                !isInIgnoreObjects(point, window));
    };
    const auto isInTitlebarArea =
        [this, &isInDraggableAreas, &isInDraggableObjects, &isResizePermitted](
            const QPointF &point, QObject *const window) -> bool {
        if (window) {
            return ((point.y() <= m_titleBarHeight) &&
                    isInDraggableAreas(point, window) &&
                    isInDraggableObjects(point, window) &&
                    isResizePermitted(point, window));
        }
        return false;
    };
    const auto moveOrResize = [&getWindowEdges, &isResizePermitted,
                               &isInTitlebarArea](const QPointF &point,
                                                  QObject *const object) {
        QWindow *window = getWindowHandle(object);
        if (window) {
            const Qt::Edges edges =
                getWindowEdges(point, window->width(), window->height());
            if (edges == Qt::Edges{}) {
                if (isInTitlebarArea(point, object)) {
                    window->startSystemMove();
                }
            } else {
                if (window->windowStates().testFlag(
                        Qt::WindowState::WindowNoState) &&
                    isResizePermitted(point, object)) {
                    window->startSystemResize(edges);
                }
            }
        } else {
            qWarning().noquote() << "Can't move or resize the window: failed "
                                    "to acquire the window handle.";
        }
    };
    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            if (isInTitlebarArea(mouseEvent->localPos(), object)) {
                // ### FIXME: If the current object is a QWidget, we can use
                // getWindowHandle(object) to get the window handle, but if we
                // call showMaximized() of that window, it will not be
                // maximized, it will be moved to the top-left edge of the
                // screen without changing it's size instead. Why? Convert the
                // object to QWidget and call showMaximized() doesn't have this
                // issue.
                if (object->isWindowType()) {
                    const auto window = qobject_cast<QWindow *>(object);
                    if (window) {
                        if (window->windowStates().testFlag(
                                Qt::WindowState::WindowFullScreen)) {
                            break;
                        }
                        if (window->windowStates().testFlag(
                                Qt::WindowState::WindowMaximized)) {
                            window->showNormal();
                        } else {
                            window->showMaximized();
                        }
                        window->setCursor(Qt::CursorShape::ArrowCursor);
                    }
                }
#ifdef QT_WIDGETS_LIB
                else if (object->isWidgetType()) {
                    const auto widget = qobject_cast<QWidget *>(object);
                    if (widget) {
                        if (widget->isFullScreen()) {
                            break;
                        }
                        if (widget->isMaximized()) {
                            widget->showNormal();
                        } else {
                            widget->showMaximized();
                        }
                        widget->setCursor(Qt::CursorShape::ArrowCursor);
                    }
                }
#endif
            }
        }
    } break;
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            moveOrResize(mouseEvent->localPos(), object);
        }
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            QWindow *window = getWindowHandle(object);
            if (window) {
                if (window->windowStates().testFlag(
                        Qt::WindowState::WindowNoState)) {
                    window->setCursor(getCursorShape(
                        getWindowEdges(mouseEvent->localPos(), window->width(),
                                       window->height())));
                }
            }
#ifdef QT_WIDGETS_LIB
            else {
                const auto widget = qobject_cast<QWidget *>(object);
                if (widget) {
                    if (!widget->isMinimized() && !widget->isMaximized() &&
                        !widget->isFullScreen()) {
                        widget->setCursor(getCursorShape(
                            getWindowEdges(mouseEvent->localPos(),
                                           widget->width(), widget->height())));
                    }
                }
            }
#endif
        }
    } break;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
        moveOrResize(
            static_cast<QTouchEvent *>(event)->touchPoints().first().pos(),
            object);
        break;
    default:
        break;
    }
    event->ignore();
    return false;
}
