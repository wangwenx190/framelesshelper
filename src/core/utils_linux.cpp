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
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qpa/qplatformscreen_p.h>
#  include <QtGui/qpa/qplatformscreen.h>
#else
#  include <QtPlatformHeaders/qxcbscreenfunctions.h>
#endif
#include <gtk/gtk.h>
#include <xcb/xcb.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

using Display = struct _XDisplay;

[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPLEFT     = 0;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOP         = 1;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    = 2;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_RIGHT       = 3;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOM      = 5;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  = 6;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_LEFT        = 7;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_MOVE             = 8;

[[maybe_unused]] static constexpr const char _NET_WM_MOVERESIZE_ATOM_NAME[] = "_NET_WM_MOVERESIZE\0";

[[maybe_unused]] static constexpr const auto _NET_WM_SENDEVENT_MASK =
    (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

[[maybe_unused]] static constexpr const char GTK_THEME_NAME_ENV_VAR[] = "GTK_THEME";
[[maybe_unused]] static constexpr const char GTK_THEME_NAME_PROP[] = "gtk-theme-name";
[[maybe_unused]] static constexpr const char GTK_THEME_PREFER_DARK_PROP[] = "gtk-application-prefer-dark-theme";
FRAMELESSHELPER_STRING_CONSTANT2(GTK_THEME_DARK_REGEX, "[:-]dark")

FRAMELESSHELPER_BYTEARRAY_CONSTANT(rootwindow)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(x11screen)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(apptime)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(appusertime)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(gettimestamp)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(startupid)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(display)
FRAMELESSHELPER_BYTEARRAY_CONSTANT(connection)

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

[[maybe_unused]] [[nodiscard]] static inline int
    qtEdgesToWmMoveOrResizeOperation(const Qt::Edges edges)
{
    if (edges == Qt::Edges{}) {
        return -1;
    }
    if (edges & Qt::TopEdge) {
        if (edges & Qt::LeftEdge) {
            return _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
        }
        if (edges & Qt::RightEdge) {
            return _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
        }
        return _NET_WM_MOVERESIZE_SIZE_TOP;
    }
    if (edges & Qt::BottomEdge) {
        if (edges & Qt::LeftEdge) {
            return _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
        }
        if (edges & Qt::RightEdge) {
            return _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
        }
        return _NET_WM_MOVERESIZE_SIZE_BOTTOM;
    }
    if (edges & Qt::LeftEdge) {
        return _NET_WM_MOVERESIZE_SIZE_LEFT;
    }
    if (edges & Qt::RightEdge) {
        return _NET_WM_MOVERESIZE_SIZE_RIGHT;
    }
    return -1;
}

[[maybe_unused]] [[nodiscard]] static inline
    QScreen *x11_findScreenForVirtualDesktop(const int virtualDesktopNumber)
{
    if (virtualDesktopNumber == -1) {
        return QGuiApplication::primaryScreen();
    }
    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return nullptr;
    }
    for (auto &&screen : qAsConst(screens)) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const auto qxcbScreen = dynamic_cast<QNativeInterface::Private::QXcbScreen *>(screen->handle());
        if (qxcbScreen && (qxcbScreen->virtualDesktopNumber() == virtualDesktopNumber)) {
            return screen;
        }
#else
        if (QXcbScreenFunctions::virtualDesktopNumber(screen) == virtualDesktopNumber) {
            return screen;
        }
#endif
    }
    return nullptr;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
[[maybe_unused]] [[nodiscard]] static inline
    unsigned long x11_appRootWindow(const int screen)
#else
[[maybe_unused]] [[nodiscard]] static inline
    quint32 x11_appRootWindow(const int screen)
#endif
{
    if (!qApp) {
        return 0;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return 0;
    }
    QScreen *scr = ((screen == -1) ? QGuiApplication::primaryScreen() : x11_findScreenForVirtualDesktop(screen));
    if (!scr) {
        return 0;
    }
    return static_cast<xcb_window_t>(reinterpret_cast<quintptr>(native->nativeResourceForScreen(krootwindow, scr)));
}

[[maybe_unused]] [[nodiscard]] static inline int x11_appScreen()
{
    if (!qApp) {
        return 0;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return 0;
    }
    return reinterpret_cast<qintptr>(native->nativeResourceForIntegration(kx11screen));
}

[[maybe_unused]] [[nodiscard]] static inline quint32 x11_appTime()
{
    if (!qApp) {
        return 0;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return 0;
    }
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return 0;
    }
    return static_cast<xcb_timestamp_t>(reinterpret_cast<quintptr>(native->nativeResourceForScreen(kapptime, screen)));
}

[[maybe_unused]] [[nodiscard]] static inline quint32 x11_appUserTime()
{
    if (!qApp) {
        return 0;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return 0;
    }
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return 0;
    }
    return static_cast<xcb_timestamp_t>(reinterpret_cast<quintptr>(native->nativeResourceForScreen(kappusertime, screen)));
}

[[maybe_unused]] [[nodiscard]] static inline quint32 x11_getTimestamp()
{
    if (!qApp) {
        return 0;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return 0;
    }
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return 0;
    }
    return static_cast<xcb_timestamp_t>(reinterpret_cast<quintptr>(native->nativeResourceForScreen(kgettimestamp, screen)));
}

[[maybe_unused]] [[nodiscard]] static inline QByteArray x11_nextStartupId()
{
    if (!qApp) {
        return {};
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return {};
    }
    return static_cast<char *>(native->nativeResourceForIntegration(kstartupid));
}

[[maybe_unused]] [[nodiscard]] static inline Display *x11_display()
{
    if (!qApp) {
        return nullptr;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    using namespace QNativeInterface;
    const auto native = qApp->nativeInterface<QX11Application>();
#else
    const auto native = qApp->platformNativeInterface();
#endif
    if (!native) {
        return nullptr;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    return native->display();
#else
    return reinterpret_cast<Display *>(native->nativeResourceForIntegration(kdisplay));
#endif
}

[[maybe_unused]] [[nodiscard]] static inline xcb_connection_t *x11_connection()
{
    if (!qApp) {
        return nullptr;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    using namespace QNativeInterface;
    const auto native = qApp->nativeInterface<QX11Application>();
#else
    const auto native = qApp->platformNativeInterface();
#endif
    if (!native) {
        return nullptr;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    return native->connection();
#else
    return reinterpret_cast<xcb_connection_t *>(native->nativeResourceForIntegration(kconnection));
#endif
}

static inline void
    emulateMouseButtonRelease(const WId windowId, const QPoint &globalPos, const QPoint &localPos)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    xcb_connection_t * const connection = x11_connection();
    Q_ASSERT(connection);
    const quint32 rootWindow = x11_appRootWindow(x11_appScreen());
    Q_ASSERT(rootWindow);
    xcb_button_release_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_BUTTON_RELEASE;
    xev.time = x11_appTime();
    xev.root = rootWindow;
    xev.root_x = globalPos.x();
    xev.root_y = globalPos.y();
    xev.event = windowId;
    xev.event_x = localPos.x();
    xev.event_y = localPos.y();
    xev.same_screen = true;
    xcb_send_event(connection, false, rootWindow, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                   reinterpret_cast<const char *>(&xev));
    xcb_flush(connection);
}

[[maybe_unused]] static inline void
    doStartSystemMoveResize(const WId windowId, const QPoint &globalPos, const int edges)
{
    Q_ASSERT(windowId);
    Q_ASSERT(edges >= 0);
    if (!windowId || (edges < 0)) {
        return;
    }
    xcb_connection_t * const connection = x11_connection();
    Q_ASSERT(connection);
    static const xcb_atom_t netMoveResize = [connection]() -> xcb_atom_t {
        const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false,
                             qstrlen(_NET_WM_MOVERESIZE_ATOM_NAME), _NET_WM_MOVERESIZE_ATOM_NAME);
        xcb_intern_atom_reply_t * const reply = xcb_intern_atom_reply(connection, cookie, nullptr);
        Q_ASSERT(reply);
        const xcb_atom_t atom = reply->atom;
        Q_ASSERT(atom);
        std::free(reply);
        return atom;
    }();
    const quint32 rootWindow = x11_appRootWindow(x11_appScreen());
    Q_ASSERT(rootWindow);
    xcb_client_message_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = netMoveResize;
    xev.window = windowId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = edges;
    xev.data.data32[3] = XCB_BUTTON_INDEX_1;
    // First we need to ungrab the pointer that may have been
    // automatically grabbed by Qt on ButtonPressEvent.
    xcb_ungrab_pointer(connection, x11_appTime());
    xcb_send_event(connection, false, rootWindow, _NET_WM_SENDEVENT_MASK,
                   reinterpret_cast<const char *>(&xev));
    xcb_flush(connection);
}

static inline void sendMouseReleaseEvent(QWindow *window, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const qreal dpr = window->devicePixelRatio();
    const QPoint deviceGlobalPos = QPointF(QPointF(globalPos) * dpr).toPoint();
    const QPoint logicalLocalPos = window->mapFromGlobal(globalPos);
    const QPoint deviceLocalPos = QPointF(QPointF(logicalLocalPos) * dpr).toPoint();
    emulateMouseButtonRelease(window->winId(), deviceGlobalPos, deviceLocalPos);
}

SystemTheme Utils::getSystemTheme()
{
    // ### TODO: how to detect high contrast mode on Linux?
    return (shouldAppsUseDarkMode() ? SystemTheme::Dark : SystemTheme::Light);
}

void Utils::startSystemMove(QWindow *window, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    // Before we start the dragging we need to tell Qt that the mouse is released.
    sendMouseReleaseEvent(window, globalPos);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    const QPoint deviceGlobalPos = QPointF(QPointF(globalPos) * window->devicePixelRatio()).toPoint();
    doStartSystemMoveResize(window->winId(), deviceGlobalPos, _NET_WM_MOVERESIZE_MOVE);
#endif
}

void Utils::startSystemResize(QWindow *window, const Qt::Edges edges, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
    // Before we start the resizing we need to tell Qt that the mouse is released.
    sendMouseReleaseEvent(window, globalPos);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemResize(edges);
#else
    const int section = qtEdgesToWmMoveOrResizeOperation(edges);
    if (section < 0) {
        return;
    }
    const QPoint deviceGlobalPos = QPointF(QPointF(globalPos) * window->devicePixelRatio()).toPoint();
    doStartSystemMoveResize(window->winId(), deviceGlobalPos, section);
#endif
}

bool Utils::isTitleBarColorized()
{
    // ### TODO
    return false;
}

QColor Utils::getWmThemeColor()
{
    // ### TODO
    return {};
}

bool Utils::shouldAppsUseDarkMode_linux()
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

FRAMELESSHELPER_END_NAMESPACE
