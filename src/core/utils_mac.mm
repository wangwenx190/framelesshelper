/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
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

#include "utils.h"
#include <QtCore/qdebug.h>
#include <QtCore/qhash.h>
#include <QtGui/qwindow.h>
#include <QtGui/private/qcoregraphics_p.h>
#include <objc/runtime.h>
#include <Cocoa/Cocoa.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

[[nodiscard]] bool shouldAppsUseDarkMode_macos()
{
#if QT_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_14)
    if (__builtin_available(macOS 10.14, *)) {
        const auto appearance = [NSApp.effectiveAppearance bestMatchFromAppearancesWithNames:
                                    @[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]];
        return [appearance isEqualToString:NSAppearanceNameDarkAqua];
    }
#endif
    return false;
}

class NSWindowProxy
{
    Q_DISABLE_COPY_MOVE(NSWindowProxy)

public:
    explicit NSWindowProxy(NSWindow *window)
    {
        Q_ASSERT(window);
        if (!window) {
            return;
        }
        nswindow = window;
        saveState();
    }

    ~NSWindowProxy()
    {
        restoreState();
        nswindow = nullptr;
    }

    void saveState()
    {
        oldStyleMask = nswindow.styleMask;
        oldTitlebarAppearsTransparent = nswindow.titlebarAppearsTransparent;
        oldTitleVisibility = nswindow.titleVisibility;
        oldHasShadow = nswindow.hasShadow;
        oldShowsToolbarButton = nswindow.showsToolbarButton;
        oldMovableByWindowBackground = nswindow.movableByWindowBackground;
        oldMovable = nswindow.movable;
        oldCloseButtonVisible = ![nswindow standardWindowButton:NSWindowCloseButton].hidden;
        oldMiniaturizeButtonVisible = ![nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden;
        oldZoomButtonVisible = ![nswindow standardWindowButton:NSWindowZoomButton].hidden;
    }

    void restoreState()
    {
        nswindow.styleMask = oldStyleMask;
        nswindow.titlebarAppearsTransparent = oldTitlebarAppearsTransparent;
        nswindow.titleVisibility = oldTitleVisibility;
        nswindow.hasShadow = oldHasShadow;
        nswindow.showsToolbarButton = oldShowsToolbarButton;
        nswindow.movableByWindowBackground = oldMovableByWindowBackground;
        nswindow.movable = oldMovable;
        [nswindow standardWindowButton:NSWindowCloseButton].hidden = !oldCloseButtonVisible;
        [nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden = !oldMiniaturizeButtonVisible;
        [nswindow standardWindowButton:NSWindowZoomButton].hidden = !oldZoomButtonVisible;
    }

    void setSystemTitleBarVisible(const bool visible)
    {
        NSView * const nsview = [nswindow contentView];
        Q_ASSERT(nsview);
        if (!nsview) {
            return;
        }
        nsview.wantsLayer = YES;
        if (visible) {
            nswindow.styleMask &= ~NSWindowStyleMaskFullSizeContentView;
        } else {
            nswindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
        }
        nswindow.titlebarAppearsTransparent = (visible ? NO : YES);
        nswindow.titleVisibility = (visible ? NSWindowTitleVisible : NSWindowTitleHidden);
        nswindow.hasShadow = YES;
        nswindow.showsToolbarButton = NO;
        nswindow.movableByWindowBackground = NO;
        nswindow.movable = NO;
        [nswindow standardWindowButton:NSWindowCloseButton].hidden = (visible ? NO : YES);
        [nswindow standardWindowButton:NSWindowMiniaturizeButton].hidden = (visible ? NO : YES);
        [nswindow standardWindowButton:NSWindowZoomButton].hidden = (visible ? NO : YES);
    }

private:
    NSWindow *nswindow = nullptr;
    NSWindowStyleMask oldStyleMask = 0;
    BOOL oldTitlebarAppearsTransparent = NO;
    BOOL oldHasShadow = NO;
    BOOL oldShowsToolbarButton = NO;
    BOOL oldMovableByWindowBackground = NO;
    BOOL oldMovable = NO;
    BOOL oldCloseButtonVisible = NO;
    BOOL oldMiniaturizeButtonVisible = NO;
    BOOL oldZoomButtonVisible = NO;
    NSWindowTitleVisibility oldTitleVisibility = NSWindowTitleVisible;
};

using NSWindowProxyHash = QHash<WId, NSWindowProxy *>;
Q_GLOBAL_STATIC(NSWindowProxyHash, g_nswindowOverrideHash);

[[nodiscard]] static inline NSWindow *mac_getNSWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return nullptr;
    }
    const auto nsview = reinterpret_cast<NSView *>(windowId);
    Q_ASSERT(nsview);
    if (!nsview) {
        return nullptr;
    }
    return [nsview window];
}

static inline void mac_windowStartNativeDrag(const WId windowId, const QPoint &globalPos)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const NSWindow * const nswindow = mac_getNSWindow(windowId);
    Q_ASSERT(nswindow);
    if (!nswindow) {
        return;
    }
    const CGEventRef clickDown = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseDown, CGPointMake(globalPos.x(), globalPos.y()), kCGMouseButtonLeft);
    NSEvent * const nsevent = [NSEvent eventWithCGEvent:clickDown];
    Q_ASSERT(nsevent);
    if (!nsevent) {
        CFRelease(clickDown);
        return;
    }
    [nswindow performWindowDragWithEvent:nsevent];
    CFRelease(clickDown);
}

SystemTheme Utils::getSystemTheme()
{
    // ### TODO: how to detect high contrast mode on macOS?
    return (shouldAppsUseDarkMode() ? SystemTheme::Dark : SystemTheme::Light);
}

void Utils::setSystemTitleBarVisible(const WId windowId, const bool visible)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    if (!g_nswindowOverrideHash()->contains(windowId)) {
        NSWindow * const nswindow = mac_getNSWindow(windowId);
        Q_ASSERT(nswindow);
        if (!nswindow) {
            return;
        }
        const auto proxy = new NSWindowProxy(nswindow);
        g_nswindowOverrideHash()->insert(windowId, proxy);
    }
    NSWindowProxy * const proxy = g_nswindowOverrideHash()->value(windowId);
    Q_ASSERT(proxy);
    if (!proxy) {
        return;
    }
    proxy->setSystemTitleBarVisible(visible);
}

void Utils::startSystemMove(QWindow *window, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    mac_windowStartNativeDrag(window->winId(), globalPos);
}

void Utils::startSystemResize(QWindow *window, const Qt::Edges edges, const QPoint &globalPos)
{
    Q_UNUSED(window);
    Q_UNUSED(edges);
    Q_UNUSED(globalPos);
}

QColor Utils::getControlsAccentColor()
{
    return qt_mac_toQColor([NSColor controlAccentColor]);
}

bool Utils::isTitleBarColorized()
{
    return false;
}

FRAMELESSHELPER_END_NAMESPACE
