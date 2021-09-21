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

#include "utilities.h"

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtX11Extras/qx11info_x11.h>
#include <X11/Xlib.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

static constexpr int kDefaultResizeBorderThickness = 8;
static constexpr int kDefaultCaptionHeight = 23;

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    const qreal devicePixelRatio = window->devicePixelRatio();
    const qreal scaleFactor = (dpiScale ? devicePixelRatio : 1.0);
    switch (metric) {
    case SystemMetric::ResizeBorderThickness: {
        const int resizeBorderThickness = window->property(Constants::kResizeBorderThicknessFlag).toInt();
        if ((resizeBorderThickness > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(resizeBorderThickness) * scaleFactor);
        } else {
            // ### TO BE IMPLEMENTED: Retrieve system value through official API
            if (dpiScale) {
                return qRound(static_cast<qreal>(kDefaultResizeBorderThickness) * devicePixelRatio);
            } else {
                return kDefaultResizeBorderThickness;
            }
        }
    }
    case SystemMetric::CaptionHeight: {
        const int captionHeight = window->property(Constants::kCaptionHeightFlag).toInt();
        if ((captionHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(captionHeight) * scaleFactor);
        } else {
            // ### TO BE IMPLEMENTED: Retrieve system value through official API
            if (dpiScale) {
                return qRound(static_cast<qreal>(kDefaultCaptionHeight) * devicePixelRatio);
            } else {
                return kDefaultCaptionHeight;
            }
        }
    }
    case SystemMetric::TitleBarHeight: {
        const int titleBarHeight = window->property(Constants::kTitleBarHeightFlag).toInt();
        if ((titleBarHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(titleBarHeight) * scaleFactor);
        } else {
            const int captionHeight = getSystemMetric(window,SystemMetric::CaptionHeight,
                                                      dpiScale, forceSystemValue);
            const int resizeBorderThickness = getSystemMetric(window, SystemMetric::ResizeBorderThickness,
                                                              dpiScale, forceSystemValue);
            return (((window->windowState() == Qt::WindowMaximized)
                     || (window->windowState() == Qt::WindowFullScreen))
                    ? captionHeight : (captionHeight + resizeBorderThickness));
        }
    }
    }
    return 0;
}

QColor Utilities::getColorizationColor()
{
    // ### TO BE IMPLEMENTED
    return Qt::darkGray;
}

int Utilities::getWindowVisibleFrameBorderThickness(const WId winId)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    return 1;
}

bool Utilities::shouldAppsUseDarkMode()
{
    // ### TO BE IMPLEMENTED
    return false;
}

ColorizationArea Utilities::getColorizationArea()
{
    // ### TO BE IMPLEMENTED
    //return ColorizationArea::None; // ‘None’ has been defined as a macro in X11 headers.
    return ColorizationArea::All;
}

bool Utilities::isThemeChanged(const void *data)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    return false;
}

bool Utilities::isSystemMenuRequested(const void *data, QPointF *pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    Q_UNUSED(pos);
    return false;
}

bool Utilities::showSystemMenu(const WId winId, const QPointF &pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    Q_UNUSED(pos);
    return false;
}

void Utilities::sendX11ButtonReleaseEvent(QWindow *w, const QPoint &globalPos)
{
    const QPoint pos = w->mapFromGlobal(globalPos);
	const auto display = QX11Info::display();
	const auto screen = QX11Info::appScreen();

	XEvent xevent;
	memset(&xevent, 0, sizeof(XEvent));

	xevent.type = ButtonRelease;
	xevent.xbutton.button = Button1;
	xevent.xbutton.window = w->winId();
	xevent.xbutton.x = pos.x();
	xevent.xbutton.y = pos.y();
	xevent.xbutton.x_root = globalPos.x();
	xevent.xbutton.y_root = globalPos.y();
	xevent.xbutton.display = display;

	if (XSendEvent(display, w->winId(), False, ButtonReleaseMask, &xevent) == 0)
        qWarning() << "Failed to send ButtonRelease event.";
	XFlush(display);
}

void Utilities::sendX11MoveResizeEvent(QWindow *w, const QPoint &globalPos, int section)
{
	const auto display = QX11Info::display();
	const auto winId = w->winId();
	const auto screen = QX11Info::appScreen();

	XEvent xev;
	const Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", false);
	xev.xclient.type = ClientMessage;
	xev.xclient.message_type = netMoveResize;
	xev.xclient.display = display;
	xev.xclient.window = winId;
	xev.xclient.format = 32;

	xev.xclient.data.l[0] = globalPos.x();
	xev.xclient.data.l[1] = globalPos.y();
	xev.xclient.data.l[2] = section;
	xev.xclient.data.l[3] = Button1;
	xev.xclient.data.l[4] = 1;
	XUngrabPointer(display, QX11Info::appTime());

	if(XSendEvent(display, QX11Info::appRootWindow(screen),
        false, SubstructureRedirectMask | SubstructureNotifyMask, &xev) == 0)
        qWarning("Failed to send Move or Resize event.");
	XFlush(display);
}

void Utilities::startX11Moving(QWindow *w, const QPoint &pos)
{
    sendX11MoveResizeEvent(w, pos, _NET_WM_MOVERESIZE_MOVE);
}

void Utilities::startX11Resizing(QWindow *w, const QPoint &pos, Qt::WindowFrameSection frameSection)
{
    int section = -1;

    switch (frameSection)
    {
    case Qt::LeftSection:
        section = _NET_WM_MOVERESIZE_SIZE_LEFT;
        break;
    case Qt::TopLeftSection:
        section = _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
        break;
    case Qt::TopSection:
        section = _NET_WM_MOVERESIZE_SIZE_TOP;
        break;
    case Qt::TopRightSection:
        section = _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
        break;
    case Qt::RightSection:
        section = _NET_WM_MOVERESIZE_SIZE_RIGHT;
        break;
    case Qt::BottomRightSection:
        section = _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
        break;
    case Qt::BottomSection:
        section = _NET_WM_MOVERESIZE_SIZE_BOTTOM;
        break;
    case Qt::BottomLeftSection:
        section = _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
        break;
    default:
        break;
    }

    if (section != -1)
        sendX11MoveResizeEvent(w, pos, section);
}

enum class X11CursorType
{
	kArrow = 2,
	kTop = 138,
	kTopRight = 136,
	kRight = 96,
	kBottomRight = 14,
	kBottom = 16,
	kBottomLeft = 12,
	kLeft = 70,
	kTopLeft = 134,
};

void Utilities::setX11CursorShape(QWindow *w, int cursorId)
{
	const auto display = QX11Info::display();
	const WId window_id = w->winId();
	const Cursor cursor = XCreateFontCursor(display, cursorId);
	if (!cursor) {
		qWarning() << "Failed to set cursor.";
	}
	XDefineCursor(display, window_id, cursor);
	XFlush(display);
}

void Utilities::resetX1CursorShape(QWindow *w)
{
	const auto display = QX11Info::display();
	const WId window_id = w->winId();
	XUndefineCursor(display, window_id);
	XFlush(display);
}

unsigned int Utilities::getX11CursorForFrameSection(Qt::WindowFrameSection frameSection)
{
    X11CursorType cursor = X11CursorType::kArrow;

    switch (frameSection)
    {
    case Qt::LeftSection:
        cursor = X11CursorType::kLeft;
        break;
    case Qt::RightSection:
        cursor = X11CursorType::kRight;
        break;
    case Qt::BottomSection:
        cursor = X11CursorType::kBottom;
        break;
    case Qt::TopSection:
        cursor = X11CursorType::kTop;
        break;
    case Qt::TopLeftSection:
        cursor = X11CursorType::kTopLeft;
        break;
    case Qt::BottomRightSection:
        cursor = X11CursorType::kBottomRight;
        break;
    case Qt::TopRightSection:
        cursor = X11CursorType::kTopRight;
        break;
    case Qt::BottomLeftSection:
        cursor = X11CursorType::kBottomLeft;
        break;
    case Qt::TitleBarArea:
        cursor = X11CursorType::kArrow;
        break;
    default:
        break;
    }

    return (unsigned int)cursor;
}

FRAMELESSHELPER_END_NAMESPACE