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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QMargins>
#include <QMouseEvent>
#include <QScreen>
#include <QTouchEvent>
#include <QWindow>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <qpa/qplatformnativeinterface.h>
#else
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformwindow_p.h>
#endif

Q_DECLARE_METATYPE(QMargins)

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {}

void FramelessHelper::updateQtFrame(QWindow *window, const int titleBarHeight)
{
    Q_ASSERT(window);
    if (titleBarHeight >= 0) {
        // Reduce top frame to zero since we paint it ourselves. Use
        // device pixel to avoid rounding errors.
        const QMargins margins = {0, -titleBarHeight, 0, 0};
        const QVariant marginsVar = QVariant::fromValue(margins);
        // The dynamic property takes effect when creating the platform
        // window.
        window->setProperty("_q_windowsCustomMargins", marginsVar);
        // If a platform window exists, change via native interface.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QPlatformWindow *platformWindow = window->handle();
        if (platformWindow) {
            QGuiApplication::platformNativeInterface()
                ->setWindowProperty(platformWindow,
                                    QString::fromUtf8("WindowsCustomMargins"),
                                    marginsVar);
        }
#else
        auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
            window->handle());
        if (platformWindow) {
            platformWindow->setCustomMargins(margins);
        }
#endif
    }
}

void FramelessHelper::moveWindowToDesktopCenter(QWindow *window)
{
    Q_ASSERT(window);
    const QSize ss = window->screen()->size();
    const int sw = ss.width();
    const int sh = ss.height();
    const int ww = window->width();
    const int wh = window->height();
    window->setX(qRound(static_cast<qreal>(sw - ww) / 2.0));
    window->setY(qRound(static_cast<qreal>(sh - wh) / 2.0));
}

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

QList<QRectF> FramelessHelper::getIgnoreAreas(const QWindow *window) const
{
    Q_ASSERT(window);
    return m_ignoreAreas.value(window);
}

void FramelessHelper::addIgnoreArea(const QWindow *window, const QRectF &val)
{
    Q_ASSERT(window);
    QList<QRectF> areas = m_ignoreAreas[window];
    areas.append(val);
    m_ignoreAreas[window] = areas;
}

QList<QRectF> FramelessHelper::getDraggableAreas(const QWindow *window) const
{
    Q_ASSERT(window);
    return m_draggableAreas.value(window);
}

void FramelessHelper::addDraggableArea(const QWindow *window, const QRectF &val)
{
    Q_ASSERT(window);
    QList<QRectF> areas = m_draggableAreas[window];
    areas.append(val);
    m_draggableAreas[window] = areas;
}

QObjectList FramelessHelper::getIgnoreObjects(const QWindow *window) const
{
    Q_ASSERT(window);
    QObjectList ret{};
    const QObjectList objs = m_ignoreObjects.value(window);
    if (!objs.isEmpty()) {
        for (auto &&_obj : qAsConst(objs)) {
            if (_obj) {
                ret.append(_obj);
            }
        }
    }
    return ret;
}

void FramelessHelper::addIgnoreObject(const QWindow *window, QObject *val)
{
    Q_ASSERT(window);
    QObjectList objs = m_ignoreObjects[window];
    objs.append(val);
    m_ignoreObjects[window] = objs;
}

QObjectList FramelessHelper::getDraggableObjects(const QWindow *window) const
{
    Q_ASSERT(window);
    QObjectList ret{};
    const QObjectList objs = m_draggableObjects.value(window);
    if (!objs.isEmpty()) {
        for (auto &&_obj : qAsConst(objs)) {
            if (_obj) {
                ret.append(_obj);
            }
        }
    }
    return ret;
}

void FramelessHelper::addDraggableObject(const QWindow *window, QObject *val)
{
    Q_ASSERT(window);
    QObjectList objs = m_draggableObjects[window];
    objs.append(val);
    m_draggableObjects[window] = objs;
}

bool FramelessHelper::getResizable(const QWindow *window) const
{
    Q_ASSERT(window);
    return !m_fixedSize.value(window);
}

void FramelessHelper::setResizable(const QWindow *window, const bool val)
{
    Q_ASSERT(window);
    m_fixedSize[window] = !val;
}

bool FramelessHelper::getTitleBarEnabled(const QWindow *window) const
{
    Q_ASSERT(window);
    return !m_disableTitleBar.value(window);
}

void FramelessHelper::setTitleBarEnabled(const QWindow *window, const bool val)
{
    Q_ASSERT(window);
    m_disableTitleBar[window] = !val;
}

void FramelessHelper::removeWindowFrame(QWindow *window, const bool center)
{
    Q_ASSERT(window);
    window->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint
                     | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint);
    // MouseTracking is always enabled for QWindow.
    window->installEventFilter(this);
    if (center) {
        moveWindowToDesktopCenter(window);
    }
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object->isWindowType()) {
        return false;
    }
    // QWindow will always be a top level window. It can't
    // be anyone's child window.
    const auto currentWindow = qobject_cast<QWindow *>(object);
    static bool m_bIsMRBPressed = false;
    static QPointF m_pOldMousePos = {};
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
    const auto isInSpecificAreas = [](const QPointF &mousePos, const QList<QRectF> &areas) -> bool {
        if (areas.isEmpty()) {
            return false;
        }
        for (auto &&area : qAsConst(areas)) {
            if (area.contains(mousePos)) {
                return true;
            }
        }
        return false;
    };
    const auto isInSpecificObjects = [](const QPointF &mousePos,
                                        const QObjectList &objects) -> bool {
        if (objects.isEmpty()) {
            return false;
        }
        for (auto &&obj : qAsConst(objects)) {
            if (!obj) {
                continue;
            }
            const bool isWidget = obj->inherits("QWidget");
            const bool isQuickItem = obj->inherits("QQuickItem");
            if (!isWidget && !isQuickItem) {
                qWarning() << obj << "is not a QWidget or QQuickItem!";
                continue;
            }
            const auto mapOriginPointToWindow = [](const QObject *obj) -> QPointF {
                Q_ASSERT(obj);
                QPointF point = {obj->property("x").toReal(), obj->property("y").toReal()};
                for (QObject *parent = obj->parent(); parent; parent = parent->parent()) {
                    point += {parent->property("x").toReal(), parent->property("y").toReal()};
                }
                return point;
            };
            const QPointF originPoint = mapOriginPointToWindow(obj);
            const qreal width = obj->property("width").toReal();
            const qreal height = obj->property("height").toReal();
            if (QRectF(originPoint.x(), originPoint.y(), width, height).contains(mousePos)) {
                return true;
            }
        }
        return false;
    };
    const auto isInIgnoreAreas = [this, &isInSpecificAreas](const QPointF &point,
                                                            const QWindow *window) -> bool {
        Q_ASSERT(window);
        return isInSpecificAreas(point, getIgnoreAreas(window));
    };
    const auto isInIgnoreObjects = [this, &isInSpecificObjects](const QPointF &point,
                                                                const QWindow *window) -> bool {
        Q_ASSERT(window);
        return isInSpecificObjects(point, getIgnoreObjects(window));
    };
    const auto isInDraggableAreas = [this, &isInSpecificAreas](const QPointF &point,
                                                               const QWindow *window) -> bool {
        Q_ASSERT(window);
        const auto areas = getDraggableAreas(window);
        return (areas.isEmpty() ? true : isInSpecificAreas(point, areas));
    };
    const auto isInDraggableObjects = [this, &isInSpecificObjects](const QPointF &point,
                                                                   const QWindow *window) -> bool {
        Q_ASSERT(window);
        const auto objs = getDraggableObjects(window);
        return (objs.isEmpty() ? true : isInSpecificObjects(point, objs));
    };
    const auto isResizePermitted = [&isInIgnoreAreas,
                                    &isInIgnoreObjects](const QPointF &globalPoint,
                                                        const QPointF &point,
                                                        const QWindow *window) -> bool {
        Q_ASSERT(window);
        return (!isInIgnoreAreas(point, window) && !isInIgnoreObjects(globalPoint, window));
    };
    const auto isInTitlebarArea = [this,
                                   &isInDraggableAreas,
                                   &isInDraggableObjects,
                                   &isResizePermitted](const QPointF &globalPoint,
                                                       const QPointF &point,
                                                       const QWindow *window) -> bool {
        Q_ASSERT(window);
        const bool customDragAreas = !getDraggableAreas(window).isEmpty();
        const bool customDragObjects = !getDraggableObjects(window).isEmpty();
        const bool customDrag = customDragAreas || customDragObjects;
        return ((customDrag ? (isInDraggableAreas(point, window)
                               && isInDraggableObjects(globalPoint, window))
                            : (point.y() <= m_titleBarHeight))
                && isResizePermitted(globalPoint, point, window) && getTitleBarEnabled(window));
        return false;
    };
    const auto moveOrResize =
        [this, &getWindowEdges, &isResizePermitted, &isInTitlebarArea](const QPointF &globalPoint,
                                                                       const QPointF &point,
                                                                       QWindow *window) {
            Q_ASSERT(window);
            //const QPointF deltaPoint = globalPoint - m_pOldMousePos;
            const Qt::Edges edges = getWindowEdges(point, window->width(), window->height());
            if (edges == Qt::Edges{}) {
                if (isInTitlebarArea(globalPoint, point, window)) {
                    if (!window->startSystemMove()) {
                        // ### FIXME: TO BE IMPLEMENTED!
                    }
                }
            } else {
                if (window->windowStates().testFlag(Qt::WindowState::WindowNoState)
                    && isResizePermitted(globalPoint, point, window) && getResizable(window)) {
                    if (!window->startSystemResize(edges)) {
                        // ### FIXME: TO BE IMPLEMENTED!
                    }
                }
            }
        };
    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            if (isInTitlebarArea(mouseEvent->screenPos(), mouseEvent->windowPos(), currentWindow)) {
                if (currentWindow->windowStates().testFlag(Qt::WindowState::WindowFullScreen)) {
                    break;
                }
                if (currentWindow->windowStates().testFlag(Qt::WindowState::WindowMaximized)) {
                    currentWindow->showNormal();
                } else {
                    currentWindow->showMaximized();
                }
                currentWindow->setCursor(Qt::CursorShape::ArrowCursor);
            }
        }
    } break;
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            m_bIsMRBPressed = true;
            m_pOldMousePos = mouseEvent->screenPos();
            moveOrResize(mouseEvent->screenPos(), mouseEvent->windowPos(), currentWindow);
        }
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (currentWindow->windowStates().testFlag(Qt::WindowState::WindowNoState)
                && getResizable(currentWindow)) {
                currentWindow->setCursor(getCursorShape(getWindowEdges(mouseEvent->windowPos(),
                                                                       currentWindow->width(),
                                                                       currentWindow->height())));
            }
        }
    } break;
    case QEvent::MouseButtonRelease: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            m_bIsMRBPressed = false;
            m_pOldMousePos = {};
        }
    } break;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate: {
        const auto point = static_cast<QTouchEvent *>(event)->touchPoints().first();
        moveOrResize(point.screenPos(), point.pos(), currentWindow);
    } break;
    default:
        break;
    }
    return false;
}
#endif
