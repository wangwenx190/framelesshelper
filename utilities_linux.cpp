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

void Utilities::sendX11ButtonRelease(QWindow *w, const QPoint &pos)
{
    QPoint clientPos = w->mapFromGlobal(pos);
    Display *display = QX11Info::display();
    int screen = QX11Info::appScreen();
    unsigned long rootWindow = QX11Info::appRootWindow(screen);

    XEvent event;
    memset(&event, 0, sizeof (event));

    event.xbutton.button = 0;
    event.xbutton.same_screen = True;
    event.xbutton.send_event = True;
    Window window = w->winId();
    event.xbutton.window = window;
    event.xbutton.root = rootWindow;
    event.xbutton.x_root = pos.x();
    event.xbutton.y_root = pos.y();
    event.xbutton.x = clientPos.x();
    event.xbutton.y = clientPos.y();
    event.xbutton.type = ButtonRelease;
    event.xbutton.time = CurrentTime;

    if (XSendEvent(display, window, True, ButtonReleaseMask, &event) == 0)
        qWarning() << "Cant send ButtonRelease event.";
    XFlush(display);
}

void Utilities::startX11Moving(QWindow *w, const QPoint &pos)
{
    Display *display = QX11Info::display();
    int screen = QX11Info::appScreen();
    unsigned long rootWindow = QX11Info::appRootWindow(screen);
    static Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);

    XUngrabPointer(display, CurrentTime);

    XEvent event;
    memset(&event, 0x00, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.window = w->winId();
    event.xclient.message_type = netMoveResize;
    event.xclient.serial = 0;
    event.xclient.display = display;
    event.xclient.send_event = True;
    event.xclient.format = 32;
    event.xclient.data.l[0] = pos.x();
    event.xclient.data.l[1] = pos.y();
    event.xclient.data.l[2] = _NET_WM_MOVERESIZE_MOVE;
    event.xclient.data.l[3] = Button1;
    event.xclient.data.l[4] = 0; /* unused */
    if (XSendEvent(display, rootWindow,
        False, SubstructureRedirectMask | SubstructureNotifyMask, &event) == 0)
        qWarning() << "Cant send Move event.";
    XFlush(display);
}


FRAMELESSHELPER_END_NAMESPACE