#ifndef NSWINDOWPROXY_H
#define NSWINDOWPROXY_H

#include <objc/runtime.h>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include <Quartz/Quartz.h>

#include <QtCore/qpoint.h>

#include "framelesshelper.h"
#include "scoped_nsobject.h"
#include "window_buttons_proxy.h"

@class NSWindowProxyDelegate;

class NSWindowProxy
{
private:
    NSWindow* m_window;
    QWindow* m_qtwindow;
    scoped_nsobject<WindowButtonsProxy> m_buttonProxy;
    scoped_nsobject<NSWindowProxyDelegate> m_windowDelegate;
    bool m_windowButtonVisibility;
    QPoint m_trafficLightPosition;

public:
    NSWindowProxy(NSWindow *window, QWindow *qtwindow);
    ~NSWindowProxy();

    NSWindow* window() { return m_window; }

    QPoint trafficLightPosition() { return m_trafficLightPosition; }
    void setTrafficLightPosition(const QPoint &pos);

    bool windowButtonVisibility() { return m_windowButtonVisibility; }
    void setWindowButtonVisibility(bool visible);

    void redrawTrafficLights();

    bool isFullscreen() const;
    void setTitle(const QString& title);

    void notifyWindowEnterFullScreen();
    void notifyWindowLeaveFullScreen();
    void notifyWindowWillEnterFullScreen();
    void notifyWindowWillLeaveFullScreen();
    void notifyWindowCloseButtonClicked();

};

@interface NSWindowProxyDelegate : NSObject<NSWindowDelegate> {
 @private
  NSWindowProxy* m_windowProxy;
  bool m_isZooming;
  int m_level;
  bool m_isResizable;

  // Only valid during a live resize.
  // Used to keep track of whether a resize is happening horizontally or
  // vertically, even if physically the user is resizing in both directions.
  bool m_resizingHorizontally;
}
- (id)initWithWindowProxy:(NSWindowProxy*)proxy;
@end

#endif // NSWINDOWPROXY_H
