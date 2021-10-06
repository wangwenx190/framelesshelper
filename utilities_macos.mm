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

#include <objc/runtime.h>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include <QtGui/qguiapplication.h>
#include <QtCore/qlist.h>

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

static QList<NSWindow*> gFlsWindows;
static bool gNSWindowOverrode = false;

typedef void (*setStyleMaskType)(id, SEL, NSWindowStyleMask);
static setStyleMaskType gOrigSetStyleMask = nullptr;
static void __setStyleMask(id obj, SEL sel, NSWindowStyleMask styleMask)
{
    if (gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
    {
        styleMask = styleMask | NSWindowStyleMaskFullSizeContentView;
    }

    if (gOrigSetStyleMask != nullptr)
        gOrigSetStyleMask(obj, sel, styleMask);
}

typedef void (*setTitlebarAppearsTransparentType)(id, SEL, BOOL);
static setTitlebarAppearsTransparentType gOrigSetTitlebarAppearsTransparent = nullptr;
static void __setTitlebarAppearsTransparent(id obj, SEL sel, BOOL transparent)
{
    if (gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
        transparent = true;

    if (gOrigSetTitlebarAppearsTransparent != nullptr)
        gOrigSetTitlebarAppearsTransparent(obj, sel, transparent);
}

typedef BOOL (*canBecomeKeyWindowType)(id, SEL);
static canBecomeKeyWindowType gOrigCanBecomeKeyWindow = nullptr;
static BOOL __canBecomeKeyWindow(id obj, SEL sel)
{
    if (gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
    {
        return true;
    }

    if (gOrigCanBecomeKeyWindow != nullptr)
        return gOrigCanBecomeKeyWindow(obj, sel);

    return true;
}

typedef BOOL (*canBecomeMainWindowType)(id, SEL);
static canBecomeMainWindowType gOrigCanBecomeMainWindow = nullptr;
static BOOL __canBecomeMainWindow(id obj, SEL sel)
{
    if (gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
    {
        return true;
    }

    if (gOrigCanBecomeMainWindow != nullptr)
        return gOrigCanBecomeMainWindow(obj, sel);
    return true;
}

typedef void (*sendEventType)(id, SEL, NSEvent*);
static sendEventType gOrigSendEvent = nullptr;
static void __sendEvent(id obj, SEL sel, NSEvent* event)
{
    if (gOrigSendEvent != nullptr)
        gOrigSendEvent(obj, sel, event);


    if (!gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
        return;

    if (event.type == NSEventTypeLeftMouseDown)
        QGuiApplication::processEvents();
}

/*!
    Replace origin method \a origSEL of class \a cls with new one \a newIMP ,
    then return old method as function pointer.
 */
static void* replaceMethod(Class cls, SEL origSEL, IMP newIMP)
{
    Method origMethod = class_getInstanceMethod(cls, origSEL);
    void *funcPtr = (void *)method_getImplementation(origMethod);
    if (!class_addMethod(cls, origSEL, newIMP, method_getTypeEncoding(origMethod))) {
        method_setImplementation(origMethod, newIMP);
    }

    return funcPtr;
}

static void restoreMethod(Class cls, SEL origSEL, IMP oldIMP)
{
    Method method = class_getInstanceMethod(cls, origSEL);
    method_setImplementation(method, oldIMP);
}

static void overrideNSWindowMethods(NSWindow* window)
{
    if (!gNSWindowOverrode) {
        Class cls = [window class];

        gOrigSetStyleMask = (setStyleMaskType) replaceMethod(
                    cls, @selector(setStyleMask:), (IMP) __setStyleMask);
        gOrigSetTitlebarAppearsTransparent = (setTitlebarAppearsTransparentType) replaceMethod(
                    cls, @selector(setTitlebarAppearsTransparent:), (IMP) __setTitlebarAppearsTransparent);
        gOrigCanBecomeKeyWindow = (canBecomeKeyWindowType) replaceMethod(
                    cls, @selector(canBecomeKeyWindow), (IMP) __canBecomeKeyWindow);
        gOrigCanBecomeMainWindow = (canBecomeMainWindowType) replaceMethod(
                    cls, @selector(canBecomeMainWindow), (IMP) __canBecomeMainWindow);
        gOrigSendEvent = (sendEventType) replaceMethod(
                    cls, @selector(sendEvent:), (IMP) __sendEvent);

        gNSWindowOverrode = true;
    }

    gFlsWindows.append(window);
}

static void restoreNSWindowMethods(NSWindow* window)
{
    gFlsWindows.removeAll(window);
    if (gFlsWindows.size() == 0) {
        Class cls = [window class];

        restoreMethod(cls, @selector(setStyleMask:), (IMP) gOrigSetStyleMask);
        gOrigSetStyleMask = nullptr;

        restoreMethod(cls, @selector(setTitlebarAppearsTransparent:), (IMP) gOrigSetTitlebarAppearsTransparent);
        gOrigSetTitlebarAppearsTransparent = nullptr;

        restoreMethod(cls, @selector(canBecomeKeyWindow), (IMP) gOrigCanBecomeKeyWindow);
        gOrigCanBecomeKeyWindow = nullptr;

        restoreMethod(cls, @selector(canBecomeMainWindow), (IMP) gOrigCanBecomeMainWindow);
        gOrigCanBecomeMainWindow = nullptr;

        restoreMethod(cls, @selector(sendEvent:), (IMP) gOrigSendEvent);
        gOrigSendEvent = nullptr;

        gNSWindowOverrode = false;
    }

}

static QHash<QWindow*, NSWindow*> gQWindowToNSWindow;

static NSWindow* getNSWindow(QWindow* w)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr)
        return nullptr;
    return [view window];
}

bool setMacWindowHook(QWindow* w)
{
    NSWindow* nswindow = getNSWindow(w);
    if (nswindow == nullptr)
        return false;

    gQWindowToNSWindow.insert(w, nswindow);
    overrideNSWindowMethods(nswindow);

    return true;
}

bool unsetMacWindowHook(QWindow* w)
{
    if (!gQWindowToNSWindow.contains(w))
        return false;
    NSWindow* obj = gQWindowToNSWindow[w];
    gQWindowToNSWindow.remove(w);
    restoreNSWindowMethods(obj);

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

bool showMacWindowButton(QWindow *w)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr)
        return false;
    NSWindow* nswindow = [view window];
    if (nswindow == nullptr)
        return false;

    nswindow.showsToolbarButton = true;
    [nswindow standardWindowButton:NSWindowCloseButton].hidden = false;
    [nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden = false;
    [nswindow standardWindowButton:NSWindowZoomButton].hidden = false;

    return true;
}

bool startMacDrag(QWindow* w, const QPoint& pos)
{
    NSView* view = reinterpret_cast<NSView *>(w->winId());
    if (view == nullptr)
        return false;
    NSWindow* nswindow = [view window];
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

} // namespace Utilities

FRAMELESSHELPER_END_NAMESPACE
