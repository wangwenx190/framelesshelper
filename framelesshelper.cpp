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

#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include "framelesswindowsmanager.h"
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessHelper::FramelessHelper(QWindow *window)
    : QObject(window)
    , m_window(window)
    , m_hoveredFrameSection(Qt::NoSection)
    , m_clickedFrameSection(Qt::NoSection)
{
    Q_ASSERT(window != nullptr && window->isTopLevel());
}

/*!
    Setup the window, make it frameless.
 */
void FramelessHelper::install()
{
    QRect origRect = m_window->geometry();
    m_origWindowFlags = m_window->flags();

#ifdef Q_OS_MAC
    m_window->setFlags(Qt::Window);
#else
    m_window->setFlags(m_origWindowFlags | Qt::FramelessWindowHint);
#endif

    m_window->setGeometry(origRect);
    resizeWindow(origRect.size());

    m_window->installEventFilter(this);
}

/*!
    Restore the window to its original state
 */
void FramelessHelper::uninstall()
{
    m_window->setFlags(m_origWindowFlags);
    m_origWindowFlags = Qt::WindowFlags();
    resizeWindow(QSize());

    m_window->removeEventFilter(this);
}

/*!
    Resize non-client area
 */
void FramelessHelper::resizeWindow(const QSize& windowSize)
{
    if (windowSize == this->windowSize())
        return;

    setWindowSize(windowSize);
}

QRect FramelessHelper::titleBarRect()
{
    return QRect(0, 0, windowSize().width(), titleBarHeight());
}


QRegion FramelessHelper::titleBarRegion()
{
    QRegion region(titleBarRect());

    for (const auto obj : m_HTVObjects) {
        if (!obj || !(obj->isWidgetType() || obj->inherits("QQuickItem"))) {
            continue;
        }

        if (!obj->property("visible").toBool()) {
            continue;
        }

        region -= getHTVObjectRect(obj);
    }

    return region;
}

QRect FramelessHelper::clientRect()
{
    QRect rect(0, 0, windowSize().width(), windowSize().height());
    rect = rect.adjusted(
        resizeBorderThickness(), titleBarHeight(),
        -resizeBorderThickness(), -resizeBorderThickness()
    );
    return rect;
}

QRegion FramelessHelper::nonClientRegion()
{
    QRegion region(QRect(QPoint(0, 0), windowSize()));
    region -= clientRect();

    for (const auto obj : m_HTVObjects) {
        if (!obj || !(obj->isWidgetType() || obj->inherits("QQuickItem"))) {
            continue;
        }

        if (!obj->property("visible").toBool()) {
            continue;
        }

        region -= getHTVObjectRect(obj);
    }

    return region;
}

bool FramelessHelper::isInTitlebarArea(const QPoint& pos)
{
    return titleBarRegion().contains(pos);
}

const int kCornerFactor = 2;

/*!
    \brief Determine window frame section by coordinates.

    Returns the window frame section at position \a pos, or \c Qt::NoSection
    if there is no window frame section at this position.

 */
Qt::WindowFrameSection FramelessHelper::mapPosToFrameSection(const QPoint& pos)
{
    int border = 0;

    // TODO: get system default resize border
    const int sysBorder = Utilities::getSystemMetric(m_window, SystemMetric::ResizeBorderThickness, false);

    Qt::WindowStates states = m_window->windowState();
    // Resizing is disabled when WindowMaximized or WindowFullScreen
    if (!(states & Qt::WindowMaximized) && !(states & Qt::WindowFullScreen))
    {
        border = resizeBorderThickness();
        border = qMin(border, sysBorder);
    }

    QRect windowRect(0, 0, windowSize().width(), windowSize().height());

    if (windowRect.contains(pos))
    {
        QPoint mappedPos = pos - windowRect.topLeft();

        // The corner is kCornerFactor times the size of the border
        if (QRect(0, 0, border * kCornerFactor, border * kCornerFactor).contains(mappedPos))
            return Qt::TopLeftSection;

        if (QRect(border * kCornerFactor, 0, windowRect.width() - border * 2 * kCornerFactor, border).contains(mappedPos))
            return Qt::TopSection;

        if (QRect(windowRect.width() - border * kCornerFactor, 0, border * kCornerFactor, border * kCornerFactor).contains(mappedPos))
            return Qt::TopRightSection;

        if (QRect(windowRect.width() - border, border * kCornerFactor, border, windowRect.height() - border * 2 * kCornerFactor).contains(mappedPos))
            return Qt::RightSection;

        if (QRect(windowRect.width() - border * kCornerFactor, windowRect.height() - border * kCornerFactor, border * kCornerFactor, border * kCornerFactor).contains(mappedPos))
            return Qt::BottomRightSection;

        if (QRect(border * kCornerFactor, windowRect.height() - border, windowRect.width() - border * 2 * kCornerFactor, border).contains(mappedPos))
            return Qt::BottomSection;

        if (QRect(0, windowRect.height() - border * kCornerFactor, border * kCornerFactor, border * kCornerFactor).contains(mappedPos))
            return Qt::BottomLeftSection;

        if (QRect(0, border * kCornerFactor, border, windowRect.height() - border * 2 * kCornerFactor).contains(mappedPos))
            return Qt::LeftSection;

        // Determining window frame secion is the highest priority,
        // so the determination of the title bar area can be simpler.
        if (isInTitlebarArea(pos))
            return Qt::TitleBarArea;
    }

    return Qt::NoSection;
}

bool FramelessHelper::isHoverResizeHandler()
{
    return m_hoveredFrameSection == Qt::LeftSection ||
        m_hoveredFrameSection == Qt::RightSection ||
        m_hoveredFrameSection == Qt::TopSection ||
        m_hoveredFrameSection == Qt::BottomSection ||
        m_hoveredFrameSection == Qt::TopLeftSection ||
        m_hoveredFrameSection == Qt::TopRightSection ||
        m_hoveredFrameSection == Qt::BottomLeftSection ||
        m_hoveredFrameSection == Qt::BottomRightSection;
}

bool FramelessHelper::isClickResizeHandler()
{
    return m_clickedFrameSection == Qt::LeftSection ||
        m_clickedFrameSection == Qt::RightSection ||
        m_clickedFrameSection == Qt::TopSection ||
        m_clickedFrameSection == Qt::BottomSection ||
        m_clickedFrameSection == Qt::TopLeftSection ||
        m_clickedFrameSection == Qt::TopRightSection ||
        m_clickedFrameSection == Qt::BottomLeftSection ||
        m_clickedFrameSection == Qt::BottomRightSection;
}

QCursor FramelessHelper::cursorForFrameSection(Qt::WindowFrameSection frameSection)
{
    Qt::CursorShape cursor = Qt::ArrowCursor;

    switch (frameSection)
    {
    case Qt::LeftSection:
    case Qt::RightSection:
        cursor = Qt::SizeHorCursor;
        break;
    case Qt::BottomSection:
    case Qt::TopSection:
        cursor = Qt::SizeVerCursor;
        break;
    case Qt::TopLeftSection:
    case Qt::BottomRightSection:
        cursor = Qt::SizeFDiagCursor;
        break;
    case Qt::TopRightSection:
    case Qt::BottomLeftSection:
        cursor = Qt::SizeBDiagCursor;
        break;
    case Qt::TitleBarArea:
        cursor = Qt::ArrowCursor;
        break;
    default:
        break;
    }

    return QCursor(cursor);
}

void FramelessHelper::setCursor(const QCursor& cursor)
{
    m_window->setCursor(cursor);
    m_cursorChanged = true;
}

void FramelessHelper::unsetCursor()
{
    if (!m_cursorChanged)
        return;

    m_window->unsetCursor();
    m_cursorChanged = false;
}

void FramelessHelper::updateCursor()
{
#ifdef Q_OS_LINUX
    if (isHoverResizeHandler()) {
        Utilities::setX11CursorShape(m_window,
            Utilities::getX11CursorForFrameSection(m_hoveredFrameSection));
        m_cursorChanged = true;
    } else {
        if (!m_cursorChanged)
            return;
        Utilities::resetX1CursorShape(m_window);
        m_cursorChanged = false;
    }
#else
    if (isHoverResizeHandler()) {
        setCursor(cursorForFrameSection(m_hoveredFrameSection));
    } else {
        unsetCursor();
    }
#endif
}

void FramelessHelper::updateMouse(const QPoint& pos)
{
    updateHoverStates(pos);
    updateCursor();
}

void FramelessHelper::updateHoverStates(const QPoint& pos)
{
    m_hoveredFrameSection = mapPosToFrameSection(pos);
}

void FramelessHelper::startMove(const QPoint &globalPos)
{
#ifdef Q_OS_LINUX
    // On HiDPI screen, X11 ButtonRelease is likely to trigger
    // a QEvent::MouseMove, so we reset m_clickedFrameSection in advance.
    m_clickedFrameSection = Qt::NoSection;
    Utilities::sendX11ButtonReleaseEvent(m_window, globalPos);
    Utilities::startX11Moving(m_window, globalPos);
#endif
}

void FramelessHelper::startResize(const QPoint &globalPos, Qt::WindowFrameSection frameSection)
{
#ifdef Q_OS_LINUX
    // On HiDPI screen, X11 ButtonRelease is likely to trigger
    // a QEvent::MouseMove, so we reset m_clickedFrameSection in advance.
    m_clickedFrameSection = Qt::NoSection;
    Utilities::sendX11ButtonReleaseEvent(m_window, globalPos);
    Utilities::startX11Resizing(m_window, globalPos, frameSection);
#endif
}

void FramelessHelper::setHitTestVisible(QObject *obj)
{
    m_HTVObjects.push_back(obj);
}

bool FramelessHelper::isHitTestVisible(QObject *obj)
{
    return m_HTVObjects.contains(obj);
}

QRect FramelessHelper::getHTVObjectRect(QObject *obj)
{
    const QPoint originPoint = m_window->mapFromGlobal(
        Utilities::mapOriginPointToWindow(obj).toPoint());
    const int width = obj->property("width").toInt();
    const int height = obj->property("height").toInt();

    return QRect(originPoint, QSize(width, height));
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
{
    bool filterOut = false;

    if (object == m_window) {
        switch (event->type())
        {
        case QEvent::Resize:
        {
            QResizeEvent* re = static_cast<QResizeEvent *>(event);
            resizeWindow(re->size());
            break;
        }

        case QEvent::NonClientAreaMouseMove:
        case QEvent::MouseMove:
        {
            auto ev = static_cast<QMouseEvent *>(event);
            updateMouse(ev->pos());

            if (m_clickedFrameSection == Qt::TitleBarArea
                    && isInTitlebarArea(ev->pos())) {
                // Start system move
                startMove(ev->globalPos());
                ev->accept();
                filterOut = true;
            } else if (isClickResizeHandler() && isHoverResizeHandler()) {
                // Start system resize
                startResize(ev->globalPos(), m_hoveredFrameSection);
                ev->accept();
                filterOut = true;
            }

            // This case takes into account that the mouse moves outside the window boundary
            QRect windowRect(0, 0, windowSize().width(), windowSize().height());
            if (isClickResizeHandler() && !windowRect.contains(ev->pos())) {
                startResize(ev->globalPos(), m_clickedFrameSection);
                ev->accept();
                filterOut = true;
            }

            break;
        }
        case QEvent::Leave:
        {
            updateMouse(m_window->mapFromGlobal(QCursor::pos()));
            break;
        }
        case QEvent::NonClientAreaMouseButtonPress:
        case QEvent::MouseButtonPress:
        {
            auto ev = static_cast<QMouseEvent *>(event);

            if (ev->button() == Qt::LeftButton) 
                m_clickedFrameSection = m_hoveredFrameSection;

            break;
        }

        case QEvent::NonClientAreaMouseButtonRelease:
        case QEvent::MouseButtonRelease:
        {
            m_clickedFrameSection = Qt::NoSection;
            break;
        }

        case QEvent::NonClientAreaMouseButtonDblClick:
        case QEvent::MouseButtonDblClick:
        {
            auto ev = static_cast<QMouseEvent *>(event);
            if (isHoverResizeHandler() && ev->button() == Qt::LeftButton) {
                // double click resize handler
                if (m_clickedFrameSection == Qt::TopSection
                    || m_clickedFrameSection == Qt::BottomSection) {
                    QRect screenRect = m_window->screen()->availableGeometry();
                    QRect winRect = m_window->geometry();
                    m_window->setGeometry(winRect.x(), 0, winRect.width(), screenRect.height());
                } else if (m_clickedFrameSection == Qt::LeftSection
                            || m_clickedFrameSection == Qt::RightSection) {
                    QRect screenRect = m_window->screen()->availableGeometry();
                    QRect winRect = m_window->geometry();
                    m_window->setGeometry(0, winRect.y(), screenRect.width(), winRect.height());
                }

            } else if (isInTitlebarArea(ev->pos()) && ev->button() == Qt::LeftButton) {
                Qt::WindowStates states = m_window->windowState();
                if (states & Qt::WindowMaximized)
                    m_window->showNormal();
                else
                    m_window->showMaximized();
            }

            break;
        }

        default:
            break;
        }
    }

    return filterOut;
}

FRAMELESSHELPER_END_NAMESPACE

#endif
