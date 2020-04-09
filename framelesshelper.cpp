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
#include <QVariant>
#include <QWidget>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>
#include <QTouchEvent>
#endif

Q_DECLARE_METATYPE(QMargins)

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {
    connect(this, &FramelessHelper::titlebarHeightChanged, this,
            &FramelessHelper::updateQtFrame_internal);
    connect(this, &FramelessHelper::framelessWindowsChanged,
            [this]() { updateQtFrame_internal(m_titlebarHeight); });
#ifdef Q_OS_WINDOWS
    m_borderWidth = WinNativeEventFilter::borderWidth(nullptr);
    m_borderHeight = WinNativeEventFilter::borderHeight(nullptr);
    m_titlebarHeight = WinNativeEventFilter::titlebarHeight(nullptr);
#else
    // TODO: The default border width and height on Windows is 8 pixels if DPI
    // is 96. Don't know how to acquire these values on UNIX platforms.
    m_borderWidth = 8;
    m_borderHeight = 8;
    QWidget widget;
    QStyleOption styleOption;
    styleOption.initFrom(&widget);
    m_titlebarHeight =
        widget.style()->pixelMetric(QStyle::PixelMetric::PM_TitleBarHeight,
                                    &styleOption) +
        m_borderHeight;
    qDebug().noquote() << "Window device pixel ratio:"
                       << widget.devicePixelRatioF();
    qDebug().noquote() << "Window border width:" << m_borderWidth
                       << "Window border height:" << m_borderHeight
                       << "Window titlebar height:" << m_titlebarHeight;
#endif
    updateQtFrame_internal(m_titlebarHeight);
}

void FramelessHelper::updateQtFrame(QWindow *window, int titlebarHeight) {
    if (window && (titlebarHeight > 0)) {
        // Reduce top frame to zero since we paint it ourselves. Use
        // device pixel to avoid rounding errors.
        const QMargins margins = {0, -titlebarHeight, 0, 0};
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

int FramelessHelper::borderWidth() const { return m_borderWidth; }

void FramelessHelper::setBorderWidth(int val) {
    if (m_borderWidth != val) {
        m_borderWidth = val;
#ifdef Q_OS_WINDOWS
        WinNativeEventFilter::setBorderWidth(val);
#endif
        Q_EMIT borderWidthChanged(val);
    }
}

int FramelessHelper::borderHeight() const { return m_borderHeight; }

void FramelessHelper::setBorderHeight(int val) {
    if (m_borderHeight != val) {
        m_borderHeight = val;
#ifdef Q_OS_WINDOWS
        WinNativeEventFilter::setBorderHeight(val);
#endif
        Q_EMIT borderHeightChanged(val);
    }
}

int FramelessHelper::titlebarHeight() const { return m_titlebarHeight; }

void FramelessHelper::setTitlebarHeight(int val) {
    if (m_titlebarHeight != val) {
        m_titlebarHeight = val;
#ifdef Q_OS_WINDOWS
        WinNativeEventFilter::setTitlebarHeight(val);
#endif
        Q_EMIT titlebarHeightChanged(val);
    }
}

FramelessHelper::Areas FramelessHelper::ignoreAreas() const {
    return m_ignoreAreas;
}

void FramelessHelper::setIgnoreAreas(const Areas &val) {
    if (m_ignoreAreas != val) {
        m_ignoreAreas = val;
#ifdef Q_OS_WINDOWS
        auto iter = val.cbegin();
        while (iter != val.cend()) {
            if (iter.key()) {
                const auto hwnd =
                    static_cast<HWND>(getWindowRawHandle(iter.key()));
                if (hwnd) {
                    const auto data = WinNativeEventFilter::windowData(hwnd);
                    data->ignoreAreas = iter.value();
                }
            }
            ++iter;
        }
#endif
        Q_EMIT ignoreAreasChanged(val);
    }
}

FramelessHelper::Areas FramelessHelper::draggableAreas() const {
    return m_draggableAreas;
}

void FramelessHelper::setDraggableAreas(const Areas &val) {
    if (m_draggableAreas != val) {
        m_draggableAreas = val;
#ifdef Q_OS_WINDOWS
        auto iter = val.cbegin();
        while (iter != val.cend()) {
            if (iter.key()) {
                const auto hwnd =
                    static_cast<HWND>(getWindowRawHandle(iter.key()));
                if (hwnd) {
                    const auto data = WinNativeEventFilter::windowData(hwnd);
                    data->draggableAreas = iter.value();
                }
            }
            ++iter;
        }
#endif
        Q_EMIT draggableAreasChanged(val);
    }
}

QVector<QObject *> FramelessHelper::framelessWindows() const {
    return m_framelessWindows;
}

void FramelessHelper::setFramelessWindows(const QVector<QObject *> &val) {
    if (m_framelessWindows != val) {
        m_framelessWindows = val;
        if (!val.isEmpty()) {
            for (auto &&object : qAsConst(val)) {
                if (object) {
#ifdef Q_OS_WINDOWS
                    const auto hwnd =
                        static_cast<HWND>(getWindowRawHandle(object));
                    if (hwnd) {
                        WinNativeEventFilter::addFramelessWindow(hwnd);
                    } else {
                        qWarning().noquote()
                            << "Can't make the window frameless: failed to "
                               "acquire the window handle.";
                    }
#else
                    // Don't miss the Qt::Window flag.
                    const Qt::WindowFlags flags =
                        Qt::Window | Qt::FramelessWindowHint;
                    QWindow *window = getWindowHandle(object);
                    if (window) {
                        window->setFlags(flags);
                        // MouseTracking is always enabled for QWindow.
                        window->installEventFilter(this);
                    } else {
                        const auto widget = qobject_cast<QWidget *>(object);
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
        }
        Q_EMIT framelessWindowsChanged(val);
    }
}

#ifndef Q_OS_WINDOWS
bool FramelessHelper::eventFilter(QObject *object, QEvent *event) {
    const auto isWindowTopLevel = [](QObject *window) -> bool {
        if (window) {
            if (window->isWidgetType()) {
                return qobject_cast<QWidget *>(window)->isTopLevel();
            } else if (window->isWindowType()) {
                return qobject_cast<QWindow *>(window)->isTopLevel();
            }
        }
        return false;
    };
    if (!object || !isWindowTopLevel(object)) {
        event->ignore();
        return false;
    }
    const auto getWindowEdges = [this](const QPointF &point, int ww,
                                       int wh) -> Qt::Edges {
        if (point.y() < m_borderHeight) {
            if (point.x() < m_borderWidth) {
                return Qt::Edge::TopEdge | Qt::Edge::LeftEdge;
            }
            if (point.x() > (ww - m_borderWidth)) {
                return Qt::Edge::TopEdge | Qt::Edge::RightEdge;
            }
            return Qt::Edge::TopEdge;
        }
        if (point.y() > (wh - m_borderHeight)) {
            if (point.x() < m_borderWidth) {
                return Qt::Edge::BottomEdge | Qt::Edge::LeftEdge;
            }
            if (point.x() > (ww - m_borderWidth)) {
                return Qt::Edge::BottomEdge | Qt::Edge::RightEdge;
            }
            return Qt::Edge::BottomEdge;
        }
        if (point.x() < m_borderWidth) {
            return Qt::Edge::LeftEdge;
        }
        if (point.x() > (ww - m_borderWidth)) {
            return Qt::Edge::RightEdge;
        }
        return {};
    };
    const auto getCursorShape = [](Qt::Edges edges) -> Qt::CursorShape {
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
    const auto isInSpecificAreas = [](int x, int y,
                                      const QVector<QRect> &areas) -> bool {
        for (auto &&area : qAsConst(areas)) {
            if (area.contains(x, y, true)) {
                return true;
            }
        }
        return false;
    };
    const auto isInTitlebarArea =
        [this, &isInSpecificAreas](const QPointF &point,
                                   QObject *window) -> bool {
        if (window) {
            return (point.y() < m_titlebarHeight) &&
                !isInSpecificAreas(point.x(), point.y(),
                                   m_ignoreAreas.value(window)) &&
                (m_draggableAreas.isEmpty()
                     ? true
                     : isInSpecificAreas(point.x(), point.y(),
                                         m_draggableAreas.value(window)));
        }
        return false;
    };
    const auto moveOrResize = [this, &getWindowEdges, &isInTitlebarArea](
                                  const QPointF &point, QObject *object) {
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
                        Qt::WindowState::WindowNoState)) {
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
                // FIXME: If the current object is a QWidget, we can use
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
                } else if (object->isWidgetType()) {
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
            }
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
                break;
            }
            moveOrResize(mouseEvent->localPos(), object);
        }
        break;
    }
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
            } else {
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
        }
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate: {
        moveOrResize(
            static_cast<QTouchEvent *>(event)->touchPoints().first().pos(),
            object);
        break;
    }
    default: {
        break;
    }
    }
    event->ignore();
    return false;
}
#endif

QWindow *FramelessHelper::getWindowHandle(QObject *val) {
    if (val) {
        const auto validWindow = [](QWindow *window) -> QWindow * {
            return (window && window->handle()) ? window : nullptr;
        };
        if (val->isWindowType()) {
            return validWindow(qobject_cast<QWindow *>(val));
        } else if (val->isWidgetType()) {
            const auto widget = qobject_cast<QWidget *>(val);
            if (widget) {
                return validWindow(widget->windowHandle());
            }
        } else {
            qWarning().noquote() << "Can't acquire the window handle: only "
                                    "QWidget and QWindow are accepted.";
        }
    }
    return nullptr;
}

void FramelessHelper::updateQtFrame_internal(int val) {
    if (!m_framelessWindows.isEmpty()) {
        for (auto &&object : qAsConst(m_framelessWindows)) {
            QWindow *window = getWindowHandle(object);
            if (window) {
                updateQtFrame(window, val);
            } else {
                qWarning().noquote() << "Can't modify the window frame: failed "
                                        "to acquire the window handle.";
            }
        }
    }
}

#ifdef Q_OS_WINDOWS
void *FramelessHelper::getWindowRawHandle(QObject *object) {
    if (object) {
        QWindow *window = getWindowHandle(object);
        if (window) {
            const auto handle = QGuiApplication::platformNativeInterface()
                                    ->nativeResourceForWindow("handle", window);
            if (handle) {
                return handle;
            }
        } else {
            const auto widget = qobject_cast<QWidget *>(object);
            if (widget) {
                return reinterpret_cast<void *>(widget->winId());
            }
        }
    }
    return nullptr;
}
#endif
