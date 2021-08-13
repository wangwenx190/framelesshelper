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
#include "framelesswindowsmanager.h"
#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent) {}

void FramelessHelper::removeWindowFrame(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    window->setFlags(window->flags() | Qt::FramelessWindowHint);
    window->installEventFilter(this);
    window->setProperty(Constants::framelessMode_flag, true);
}

void FramelessHelper::bringBackWindowFrame(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    window->removeEventFilter(this);
    window->setFlags(window->flags() & ~Qt::FramelessWindowHint);
    window->setProperty(Constants::framelessMode_flag, false);
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
    if ((type != QEvent::MouseButtonDblClick) && (type != QEvent::MouseButtonPress) && (type != QEvent::MouseMove)) {
        return false;
    }
    const auto window = qobject_cast<QWindow *>(object);
    const int resizeBorderWidth = FramelessWindowsManager::getResizeBorderWidth(window);
    const int resizeBorderHeight = FramelessWindowsManager::getResizeBorderHeight(window);
    const int titleBarHeight = FramelessWindowsManager::getTitleBarHeight(window);
    const bool resizable = FramelessWindowsManager::getResizable(window);
    const int windowWidth = window->width();
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
    const QPoint localMousePosition = mouseEvent->localPos().toPoint();
    const Qt::Edges edges = [window, resizeBorderWidth, resizeBorderHeight, windowWidth, &localMousePosition] {
        const int windowHeight = window->height();
        if (localMousePosition.y() <= resizeBorderHeight) {
            if (localMousePosition.x() <= resizeBorderWidth) {
                return Qt::TopEdge | Qt::LeftEdge;
            }
            if (localMousePosition.x() >= (windowWidth - resizeBorderWidth)) {
                return Qt::TopEdge | Qt::RightEdge;
            }
            return Qt::Edges{Qt::TopEdge};
        }
        if (localMousePosition.y() >= (windowHeight - resizeBorderHeight)) {
            if (localMousePosition.x() <= resizeBorderWidth) {
                return Qt::BottomEdge | Qt::LeftEdge;
            }
            if (localMousePosition.x() >= (windowWidth - resizeBorderWidth)) {
                return Qt::BottomEdge | Qt::RightEdge;
            }
            return Qt::Edges{Qt::BottomEdge};
        }
        if (localMousePosition.x() <= resizeBorderWidth) {
            return Qt::Edges{Qt::LeftEdge};
        }
        if (localMousePosition.x() >= (windowWidth - resizeBorderWidth)) {
            return Qt::Edges{Qt::RightEdge};
        }
        return Qt::Edges{};
    } ();
    const bool hitTestVisible = Utilities::isHitTestVisibleInChrome(window);
    bool isInTitlebarArea = false;
    if ((window->windowState() == Qt::WindowMaximized)
            || (window->windowState() == Qt::WindowFullScreen)) {
        isInTitlebarArea = (localMousePosition.y() >= 0)
                && (localMousePosition.y() <= titleBarHeight)
                && (localMousePosition.x() >= 0)
                && (localMousePosition.x() <= windowWidth)
                && !hitTestVisible;
    }
    if (window->windowState() == Qt::WindowNoState) {
        isInTitlebarArea = (localMousePosition.y() > resizeBorderHeight)
                && (localMousePosition.y() <= (titleBarHeight + resizeBorderHeight))
                && (localMousePosition.x() > resizeBorderWidth)
                && (localMousePosition.x() < (windowWidth - resizeBorderWidth))
                && !hitTestVisible;
    }
    if (type == QEvent::MouseButtonDblClick) {
        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            return false;
        }
        if (isInTitlebarArea) {
            if (window->windowState() == Qt::WindowState::WindowFullScreen) {
                return false;
            }
            if (window->windowState() == Qt::WindowState::WindowMaximized) {
                window->showNormal();
            } else {
                window->showMaximized();
            }
            window->setCursor(Qt::ArrowCursor);
        }
    } else if (type == QEvent::MouseMove) {
        // Display resize indicators
        if ((window->windowState() == Qt::WindowState::WindowNoState) && resizable) {
            if (((edges & Qt::TopEdge) && (edges & Qt::LeftEdge))
                    || ((edges & Qt::BottomEdge) && (edges & Qt::RightEdge))) {
                window->setCursor(Qt::SizeFDiagCursor);
            } else if (((edges & Qt::TopEdge) && (edges & Qt::RightEdge))
                       || ((edges & Qt::BottomEdge) && (edges & Qt::LeftEdge))) {
                window->setCursor(Qt::SizeBDiagCursor);
            } else if ((edges & Qt::TopEdge) || (edges & Qt::BottomEdge)) {
                window->setCursor(Qt::SizeVerCursor);
            } else if ((edges & Qt::LeftEdge) || (edges & Qt::RightEdge)) {
                window->setCursor(Qt::SizeHorCursor);
            } else {
                window->setCursor(Qt::ArrowCursor);
            }
        }

        if (mouseEvent->buttons() & Qt::LeftButton) {
            if (edges == Qt::Edges{}) {
                if (isInTitlebarArea) {
                    if (!window->startSystemMove()) {
                        // ### FIXME: TO BE IMPLEMENTED!
                        qWarning() << "Current OS doesn't support QWindow::startSystemMove().";
                    }
                }
            }
        }

    } else if (type == QEvent::MouseButtonPress) {
        if (edges != Qt::Edges{}) {
            if ((window->windowState() == Qt::WindowState::WindowNoState) && !hitTestVisible && resizable) {
                if (!window->startSystemResize(edges)) {
                    // ### FIXME: TO BE IMPLEMENTED!
                    qWarning() << "Current OS doesn't support QWindow::startSystemResize().";
                }
            }
        }
    }

    return false;
}

FRAMELESSHELPER_END_NAMESPACE

#endif
