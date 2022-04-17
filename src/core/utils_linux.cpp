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
#include <QtCore/qregularexpression.h>
#include <QtGui/qcursor.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#if ((QT_VERSION >= QT_VERSION_CHECK(5, 9, 1)) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
#  include <QtPlatformHeaders/qxcbscreenfunctions.h>
#endif
#include <gtk/gtk.h>
#include <X11/Xlib.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPLEFT     = 0;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOP         = 1;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    = 2;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_RIGHT       = 3;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOM      = 5;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  = 6;
static constexpr const auto _NET_WM_MOVERESIZE_SIZE_LEFT        = 7;
static constexpr const auto _NET_WM_MOVERESIZE_MOVE             = 8;
#endif

static constexpr const char GTK_THEME_NAME_ENV_VAR[] = "GTK_THEME";
static constexpr const char GTK_THEME_NAME_PROP[] = "gtk-theme-name";
static constexpr const char GTK_THEME_PREFER_DARK_PROP[] = "gtk-application-prefer-dark-theme";

FRAMELESSHELPER_BYTEARRAY_CONSTANT(display)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(x11screen)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(rootwindow)

FRAMELESSHELPER_STRING_CONSTANT2(GTK_THEME_DARK_REGEX, "[:-]dark")

template<typename T>
[[nodiscard]] static inline T gtkSetting(const gchar *propertyName)
{
    Q_ASSERT(propertyName);
    if (!propertyName) {
        return {};
    }
    GtkSettings * const settings = gtk_settings_get_default();
    Q_ASSERT(settings);
    if (!settings) {
        return {};
    }
    T result = {};
    g_object_get(settings, propertyName, &result, nullptr);
    return result;
}

[[nodiscard]] static inline QString gtkSetting(const gchar *propertyName)
{
    Q_ASSERT(propertyName);
    if (!propertyName) {
        return {};
    }
    const auto propertyValue = gtkSetting<gchararray>(propertyName);
    const QString result = QString::fromUtf8(propertyValue);
    g_free(propertyValue);
    return result;
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
[[nodiscard]] static inline Qt::WindowFrameSection qtEdgesToQtWindowFrameSection(const Qt::Edges edges)
{
    if (edges == Qt::Edges{}) {
        return Qt::NoSection;
    }
    if (edges & Qt::TopEdge) {
        if (edges & Qt::LeftEdge) {
            return Qt::TopLeftSection;
        }
        if (edges & Qt::RightEdge) {
            return Qt::TopRightSection;
        }
        return Qt::TopSection;
    }
    if (edges & Qt::BottomEdge) {
        if (edges & Qt::LeftEdge) {
            return Qt::BottomLeftSection;
        }
        if (edges & Qt::RightEdge) {
            return Qt::BottomRightSection;
        }
        return Qt::BottomSection;
    }
    if (edges & Qt::LeftEdge) {
        return Qt::LeftSection;
    }
    if (edges & Qt::RightEdge) {
        return Qt::RightSection;
    }
    return Qt::NoSection;
}
#endif

[[nodiscard]] bool shouldAppsUseDarkMode_linux()
{
    /*
        https://docs.gtk.org/gtk3/running.html

        It's possible to set a theme variant after the theme name when using GTK_THEME:

            GTK_THEME=Adwaita:dark

        Some themes also have "-dark" as part of their name.

        We test this environment variable first because the documentation says
        it's mainly used for easy debugging, so it should be possible to use it
        to override any other settings.
    */
    static const QRegularExpression darkRegex(kGTK_THEME_DARK_REGEX, QRegularExpression::CaseInsensitiveOption);
    const QString envThemeName = qEnvironmentVariable(GTK_THEME_NAME_ENV_VAR);
    if (!envThemeName.isEmpty()) {
        return darkRegex.match(envThemeName).hasMatch();
    }

    /*
        https://docs.gtk.org/gtk3/property.Settings.gtk-application-prefer-dark-theme.html

        This setting controls which theme is used when the theme specified by
        gtk-theme-name provides both light and dark variants. We can save a
        regex check by testing this property first.
    */
    const auto preferDark = gtkSetting<bool>(GTK_THEME_PREFER_DARK_PROP);
    if (preferDark) {
        return true;
    }

    /*
        https://docs.gtk.org/gtk3/property.Settings.gtk-theme-name.html
    */
    const QString curThemeName = gtkSetting(GTK_THEME_NAME_PROP);
    if (!curThemeName.isEmpty()) {
        return darkRegex.match(curThemeName).hasMatch();
    }

    return false;
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
[[nodiscard]] static inline Display *x11_get_display()
{
    if (!qGuiApp) {
        return nullptr;
    }
    QPlatformNativeInterface * const iface = qGuiApp->platformNativeInterface();
    if (!iface) {
        return nullptr;
    }
    const auto display = iface->nativeResourceForIntegration(kdisplay);
    if (!display) {
        return nullptr;
    }
    return static_cast<Display *>(display);
}

[[nodiscard]] static inline qintptr x11_get_desktop()
{
    if (!qGuiApp) {
        return 0;
    }
    QPlatformNativeInterface * const iface = qGuiApp->platformNativeInterface();
    if (!iface) {
        return 0;
    }
    const auto screen = iface->nativeResourceForIntegration(kx11screen);
    if (!screen) {
        return 0;
    }
    return reinterpret_cast<qintptr>(screen);
}

[[nodiscard]] static inline QScreen *x11_getScreenForVirtualDesktop(const int virtualDesktopNumber)
{
#if ((QT_VERSION >= QT_VERSION_CHECK(5, 9, 1)) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
    if (virtualDesktopNumber == -1) {
        return QGuiApplication::primaryScreen();
    }
    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return nullptr;
    }
    for (auto &&screen : qAsConst(screens)) {
        if (QXcbScreenFunctions::virtualDesktopNumber(screen) == virtualDesktopNumber) {
            return screen;
        }
    }
    return nullptr;
#else
    Q_UNUSED(virtualDesktopNumber);
    return QGuiApplication::primaryScreen();
#endif
}

[[nodiscard]] static inline quintptr x11_get_window(const int desktop)
{
    if (!qGuiApp) {
        return 0;
    }
    QPlatformNativeInterface * const iface = qGuiApp->platformNativeInterface();
    if (!iface) {
        return 0;
    }
    QScreen * const screen = x11_getScreenForVirtualDesktop(desktop);
    if (!screen) {
        return 0;
    }
    const auto rootWindow = iface->nativeResourceForScreen(krootwindow, screen);
    if (!rootWindow) {
        return 0;
    }
    return reinterpret_cast<quintptr>(rootWindow);
}

static inline void x11_emulateButtonRelease(const WId windowId, const QPoint &globalPos, const QPoint &localPos)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const Window window = windowId;
    Display * const display = x11_get_display();
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xbutton.button = 0;
    event.xbutton.same_screen = True;
    event.xbutton.send_event = True;
    event.xbutton.window = window;
    event.xbutton.root = x11_get_window(x11_get_desktop());
    event.xbutton.x_root = globalPos.x();
    event.xbutton.y_root = globalPos.y();
    event.xbutton.x = localPos.x();
    event.xbutton.y = localPos.y();
    event.xbutton.type = ButtonRelease;
    event.xbutton.time = CurrentTime;
    if (XSendEvent(display, window, True, ButtonReleaseMask, &event) == 0) {
        qWarning() << "Failed to send ButtonRelease event for native dragging.";
    }
    XFlush(display);
}

static inline void x11_moveOrResizeWindow(const WId windowId, const QPoint &pos, const int section)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }

    Display * const display = x11_get_display();
    static const Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);

    // First we need to ungrab the pointer that may have been
    // automatically grabbed by Qt on ButtonPressEvent
    XUngrabPointer(display, CurrentTime);

    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.window = windowId;
    event.xclient.message_type = netMoveResize;
    event.xclient.serial = 0;
    event.xclient.display = display;
    event.xclient.send_event = True;
    event.xclient.format = 32;
    event.xclient.data.l[0] = pos.x();
    event.xclient.data.l[1] = pos.y();
    event.xclient.data.l[2] = section;
    event.xclient.data.l[3] = Button1;
    event.xclient.data.l[4] = 0; // unused
    if (XSendEvent(display, x11_get_window(x11_get_desktop()),
        False, (SubstructureRedirectMask | SubstructureNotifyMask), &event) == 0) {
        qWarning() << "Failed to send _NET_WM_MOVERESIZE event for native dragging.";
    }
    XFlush(display);
}

static inline void x11_windowStartNativeDrag(const WId windowId, const QPoint &globalPos, const QPoint &localPos, const Qt::WindowFrameSection frameSection)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    int nativeSection = -1;
    switch (frameSection)
    {
    case Qt::LeftSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_LEFT;
        break;
    case Qt::TopLeftSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
        break;
    case Qt::TopSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_TOP;
        break;
    case Qt::TopRightSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
        break;
    case Qt::RightSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_RIGHT;
        break;
    case Qt::BottomRightSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
        break;
    case Qt::BottomSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_BOTTOM;
        break;
    case Qt::BottomLeftSection:
        nativeSection = _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
        break;
    case Qt::TitleBarArea:
        nativeSection = _NET_WM_MOVERESIZE_MOVE;
        break;
    default:
        break;
    }
    if (nativeSection == -1) {
        return;
    }
    // Before start the drag we need to tell Qt that the mouse is Released!
    x11_emulateButtonRelease(windowId, globalPos, localPos);
    x11_moveOrResizeWindow(windowId, globalPos, nativeSection);
}
#endif

SystemTheme Utils::getSystemTheme()
{
    // ### TODO: how to detect high contrast mode on Linux?
    return (shouldAppsUseDarkMode() ? SystemTheme::Dark : SystemTheme::Light);
}

void Utils::startSystemMove(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    const qreal dpr = window->devicePixelRatio();
    const QPoint globalPos = QPointF(QPointF(QCursor::pos(window->screen())) * dpr).toPoint();
    const QPoint localPos = QPointF(QPointF(window->mapFromGlobal(globalPos)) * dpr).toPoint();
    x11_windowStartNativeDrag(window->winId(), globalPos, localPos, Qt::TitleBarArea);
#endif
}

void Utils::startSystemResize(QWindow *window, const Qt::Edges edges)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemResize(edges);
#else
    const qreal dpr = window->devicePixelRatio();
    const QPoint globalPos = QPointF(QPointF(QCursor::pos(window->screen())) * dpr).toPoint();
    const QPoint localPos = QPointF(QPointF(window->mapFromGlobal(globalPos)) * dpr).toPoint();
    x11_windowStartNativeDrag(window->winId(), globalPos, localPos, qtEdgesToQtWindowFrameSection(edges));
#endif
}

FRAMELESSHELPER_END_NAMESPACE
