#include "nswindow_proxy.h"

#include <QtGui/qguiapplication.h>
#include <QtGui/qwindow.h>

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

typedef BOOL (*isFlippedType)(id, SEL);
static isFlippedType gOrigIsFlipped = nullptr;
static BOOL __isFlipped(id obj, SEL sel)
{
    if (!gFlsWindows.contains(reinterpret_cast<NSWindow *>(obj)))
        return true;

    if (gOrigIsFlipped != nullptr)
        return gOrigIsFlipped(obj, sel);

    return false;
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
        //gOrigIsFlipped = (isFlippedType) replaceMethod(
        //            cls, @selector (isFlipped), (IMP) __isFlipped);

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

        //restoreMethod(cls, @selector(isFlipped), (IMP) gOrigIsFlipped);
        //gOrigIsFlipped = nullptr;

        gNSWindowOverrode = false;
    }

}

NSWindowProxy::NSWindowProxy(NSWindow *window, QWindow *qtwindow)
    : m_windowButtonVisibility(false)
    , m_buttonProxy(nullptr)
    , m_window(window)
    , m_qtwindow(qtwindow)
{
    overrideNSWindowMethods(window);
    m_buttonProxy.reset([[WindowButtonsProxy alloc] initWithWindow:window]);
    m_windowDelegate.reset([[NSWindowProxyDelegate alloc] initWithWindowProxy:this]);
    [m_window setDelegate:m_windowDelegate.get()];
}

NSWindowProxy::~NSWindowProxy()
{
    restoreNSWindowMethods(m_window);
    [m_buttonProxy release];
}

void NSWindowProxy::setTrafficLightPosition(const QPoint &pos) {
    m_trafficLightPosition = pos;
    if (m_buttonProxy) {
        [m_buttonProxy setMargin:m_trafficLightPosition];
    }
}

void NSWindowProxy::setWindowButtonVisibility(bool visible) {
  m_windowButtonVisibility = visible;
  // The visibility of window buttons are managed by |buttons_proxy_| if the
  // style is customButtonsOnHover.
  if (false /*title_bar_style_ == TitleBarStyle::kCustomButtonsOnHover*/)
    [m_buttonProxy setVisible:visible];
  else {
      [[m_window standardWindowButton:NSWindowCloseButton] setHidden:!visible];
      [[m_window standardWindowButton:NSWindowMiniaturizeButton] setHidden:!visible];
      [[m_window standardWindowButton:NSWindowZoomButton] setHidden:!visible];
  }
}

bool NSWindowProxy::isFullscreen() const {
  return [m_window styleMask] & NSWindowStyleMaskFullScreen;
}

void NSWindowProxy::redrawTrafficLights() {
  if (m_buttonProxy && !isFullscreen())
    [m_buttonProxy redraw];
}

void NSWindowProxy::setTitle(const QString& title) {
  [m_window setTitle:title.toNSString()];
  if (m_buttonProxy)
    [m_buttonProxy redraw];
}

void NSWindowProxy::notifyWindowEnterFullScreen() {
    // Restore the window title under fullscreen mode.
    if (m_buttonProxy) {
        [m_window setTitleVisibility:NSWindowTitleVisible];
    }
}

void NSWindowProxy::notifyWindowLeaveFullScreen() {
    // Restore window buttons.
    if (m_buttonProxy && m_windowButtonVisibility) {
        [m_buttonProxy redraw];
        [m_buttonProxy setVisible:YES];
    }
}

void NSWindowProxy::notifyWindowWillEnterFullScreen() {

}

void NSWindowProxy::notifyWindowWillLeaveFullScreen() {
    if (m_buttonProxy) {
        // Hide window title when leaving fullscreen.
        [m_window setTitleVisibility:NSWindowTitleHidden];
        // Hide the container otherwise traffic light buttons jump.
        [m_buttonProxy setVisible:NO];
    }
}

void NSWindowProxy::notifyWindowCloseButtonClicked() {
    // Call QWindow::close() when button clicked.
    m_qtwindow->close();
}

@implementation NSWindowProxyDelegate
- (id)initWithWindowProxy:(NSWindowProxy*)proxy {
    m_windowProxy = proxy;
    return self;
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
    m_windowProxy->redrawTrafficLights();
}

- (void)windowDidResignMain:(NSNotification*)notification {
    m_windowProxy->redrawTrafficLights();
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    m_windowProxy->redrawTrafficLights();
}

- (void)windowDidResignKey:(NSNotification*)notification {
    // If our app is still active and we're still the key window, ignore this
    // message, since it just means that a menu extra (on the "system status bar")
    // was activated; we'll get another |-windowDidResignKey| if we ever really
    // lose key window status.
    if ([NSApp isActive] && ([NSApp keyWindow] == [notification object]))
        return;

    m_windowProxy->redrawTrafficLights();
}

- (void)windowDidResize:(NSNotification*)notification {
    m_windowProxy->redrawTrafficLights();
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification {
    m_windowProxy->notifyWindowWillEnterFullScreen();
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
    m_windowProxy->notifyWindowEnterFullScreen();
}

- (void)windowWillExitFullScreen:(NSNotification*)notification {
    m_windowProxy->notifyWindowWillLeaveFullScreen();
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    m_windowProxy->notifyWindowLeaveFullScreen();
}

- (BOOL)windowShouldClose:(id)window {
    // We will override default close behavior
    m_windowProxy->notifyWindowCloseButtonClicked();
    return NO;
}
@end
