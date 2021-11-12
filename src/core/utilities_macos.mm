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

#include <QtGui/qguiapplication.h>
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>

#include "nswindow_proxy.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

namespace Utilities {

static constexpr int kDefaultResizeBorderThickness = 8;
static constexpr int kDefaultCaptionHeight = 23;

int getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }

    // The Apple platforms implement scaling and coordinate system virtualization,
    // so there is no need to scale again.

    switch (metric) {
    case SystemMetric::ResizeBorderThickness: {
        // ### TO BE IMPLEMENTED: Retrieve system value through official API
        return kDefaultResizeBorderThickness;

    }
    case SystemMetric::CaptionHeight: {
        // ### TO BE IMPLEMENTED: Retrieve system value through official API
        return kDefaultCaptionHeight;
    }
    case SystemMetric::TitleBarHeight: {
        const int captionHeight = getSystemMetric(window, SystemMetric::CaptionHeight,
                                                  dpiScale, forceSystemValue);
        const int resizeBorderThickness = getSystemMetric(window, SystemMetric::ResizeBorderThickness,
                                                          dpiScale, forceSystemValue);
        return (((window->windowState() == Qt::WindowMaximized)
                    || (window->windowState() == Qt::WindowFullScreen))
                ? captionHeight : (captionHeight + resizeBorderThickness));
    }
    }
    return 0;
}


QColor getColorizationColor()
{
    // ### TO BE IMPLEMENTED
    return Qt::darkGray;
}

int getWindowVisibleFrameBorderThickness(const WId winId)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    return 1;
}

bool shouldAppsUseDarkMode()
{
    // ### TO BE IMPLEMENTED
    return false;
}

ColorizationArea getColorizationArea()
{
    // ### TO BE IMPLEMENTED
    return ColorizationArea::NoArea;
}

bool isThemeChanged(const void *data)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    return false;
}

bool isSystemMenuRequested(const void *data, QPointF *pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(data);
    Q_UNUSED(pos);
    return false;
}

bool showSystemMenu(const WId winId, const QPointF &pos)
{
    // ### TO BE IMPLEMENTED
    Q_UNUSED(winId);
    Q_UNUSED(pos);
    return false;
}

static QHash<QWindow*, NSWindowProxy*> gQWindowToNSWindow;

static NSWindow* getNSWindow(QWindow* w)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr) {
        qWarning() << "Unable to get NSView.";
        return nullptr;
    }
    NSWindow* nswindow = [view window];
    if (nswindow == nullptr) {
        qWarning() << "Unable to get NSWindow.";
        return nullptr;
    }

    return nswindow;
}

bool setMacWindowHook(QWindow* w)
{
    NSWindow* nswindow = getNSWindow(w);
    if (nswindow == nullptr)
        return false;

    NSWindowProxy *obj = new NSWindowProxy(nswindow, w);
    gQWindowToNSWindow.insert(w, obj);

    return true;
}

bool unsetMacWindowHook(QWindow* w)
{
    if (!gQWindowToNSWindow.contains(w))
        return false;

    NSWindowProxy* obj = gQWindowToNSWindow[w];
    gQWindowToNSWindow.remove(w);
    delete obj;

    return true;
}

bool setMacWindowFrameless(QWindow* w)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr)
        return false;
    NSWindow* nswindow = [view window];
    if (nswindow == nullptr)
        return false;

    view.wantsLayer = YES;

    nswindow.styleMask = nswindow.styleMask | NSWindowStyleMaskFullSizeContentView;
    nswindow.titlebarAppearsTransparent = true;
    nswindow.titleVisibility = NSWindowTitleHidden;
    nswindow.hasShadow = true;

    nswindow.movableByWindowBackground = false;
    nswindow.movable = false;

    nswindow.showsToolbarButton = false;
    [nswindow standardWindowButton:NSWindowCloseButton].hidden = true;
    [nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden = true;
    [nswindow standardWindowButton:NSWindowZoomButton].hidden = true;

    [nswindow makeKeyWindow];
    return true;
}

bool unsetMacWindowFrameless(QWindow* w)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr)
        return false;
    NSWindow* nswindow = [view window];
    if (nswindow == nullptr)
        return false;

    view.wantsLayer = NO;

    nswindow.styleMask = nswindow.styleMask & ~NSWindowStyleMaskFullSizeContentView;
    nswindow.titlebarAppearsTransparent = false;
    nswindow.titleVisibility = NSWindowTitleVisible;
    nswindow.hasShadow = true;

    nswindow.movableByWindowBackground = false;
    nswindow.movable = true;

    nswindow.showsToolbarButton = true;
    [nswindow standardWindowButton:NSWindowCloseButton].hidden = false;
    [nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden = false;
    [nswindow standardWindowButton:NSWindowZoomButton].hidden = false;

    return true;
}

bool startMacDrag(QWindow* w, const QPoint& pos)
{
    NSWindow* nswindow = getNSWindow(w);
    if (nswindow == nullptr)
        return false;

    CGEventRef clickDown = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseDown, CGPointMake(pos.x(), pos.y()), kCGMouseButtonLeft);
    NSEvent *nsevent = [NSEvent eventWithCGEvent:clickDown];
    [nswindow performWindowDragWithEvent:nsevent];
    CFRelease(clickDown);
    return true;
}

Qt::MouseButtons getMacMouseButtons()
{
    return static_cast<Qt::MouseButtons>((uint)(NSEvent.pressedMouseButtons & Qt::MouseButtonMask));
}

bool setStandardWindowButtonsVisibility(QWindow *w, bool visible)
{
    NSWindowProxy* obj = gQWindowToNSWindow[w];
    obj->setWindowButtonVisibility(visible);
    return true;
}

/*! The origin of \a pos is top-left of window. */
bool setStandardWindowButtonsPosition(QWindow *w, const QPoint &pos)
{
    NSWindowProxy* obj = gQWindowToNSWindow[w];
    obj->setWindowButtonVisibility(true);
    obj->setTrafficLightPosition(pos);
    return true;
}

QSize standardWindowButtonsSize(QWindow *w)
{
    NSWindow* nswindow = getNSWindow(w);
    if (nswindow == nullptr)
        return QSize();

    NSButton* left = [nswindow standardWindowButton:NSWindowCloseButton];
    NSButton* right = [nswindow standardWindowButton:NSWindowZoomButton];
    float height = NSHeight(left.frame);
    float width = NSMaxX(right.frame) - NSMinX(left.frame);
    return QSize(width, height);
}

} // namespace Utilities

FRAMELESSHELPER_END_NAMESPACE
