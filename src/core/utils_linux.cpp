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
#include "qtx11extras_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qregularexpression.h>
#include <QtGui/qwindow.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPLEFT     = 0;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOP         = 1;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    = 2;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_RIGHT       = 3;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOM      = 5;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  = 6;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_SIZE_LEFT        = 7;
[[maybe_unused]] static constexpr const auto _NET_WM_MOVERESIZE_MOVE             = 8;

[[maybe_unused]] static constexpr const char WM_MOVERESIZE_OPERATION_NAME[] = "_NET_WM_MOVERESIZE";

[[maybe_unused]] static constexpr const char GTK_THEME_NAME_ENV_VAR[] = "GTK_THEME";
[[maybe_unused]] static constexpr const char GTK_THEME_NAME_PROP[] = "gtk-theme-name";
[[maybe_unused]] static constexpr const char GTK_THEME_PREFER_DARK_PROP[] = "gtk-application-prefer-dark-theme";
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

[[maybe_unused]] static inline void
    doStartSystemMoveResize(const WId windowId, const QPoint &globalPos, const int edges)
{
    Q_ASSERT(windowId);
    Q_ASSERT(edges >= 0);
    if (!windowId || (edges < 0)) {
        return;
    }
    xcb_connection_t * const connection = QX11Info::connection();
    Q_ASSERT(connection);
    static const xcb_atom_t moveResize = [connection]() -> xcb_atom_t {
        const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false,
                             qstrlen(WM_MOVERESIZE_OPERATION_NAME), WM_MOVERESIZE_OPERATION_NAME);
        xcb_intern_atom_reply_t * const reply = xcb_intern_atom_reply(connection, cookie, nullptr);
        Q_ASSERT(reply);
        const xcb_atom_t atom = reply->atom;
        Q_ASSERT(atom);
        std::free(reply);
        return atom;
    }();
    const quint32 rootWindow = QX11Info::appRootWindow(QX11Info::appScreen());
    Q_ASSERT(rootWindow);
    xcb_client_message_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = moveResize;
    xev.window = windowId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = edges;
    xev.data.data32[3] = XCB_BUTTON_INDEX_1;
    xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
    xcb_send_event(connection, false, rootWindow,
                   (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY),
                   reinterpret_cast<const char *>(&xev));
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    Q_UNUSED(globalPos);
    window->startSystemMove();
#else
    // Qt always gives us logical coordinates, however, the native APIs
    // are expecting device coordinates.
    const qreal dpr = window->devicePixelRatio();
    const QPoint globalPos2 = QPointF(QPointF(globalPos) * dpr).toPoint();
    doStartSystemMoveResize(window->winId(), globalPos2, _NET_WM_MOVERESIZE_MOVE);
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    Q_UNUSED(globalPos);
    window->startSystemResize(edges);
#else
    const int section = qtEdgesToWmMoveOrResizeOperation(edges);
    if (section < 0) {
        return;
    }
    // Qt always gives us logical coordinates, however, the native APIs
    // are expecting device coordinates.
    const qreal dpr = window->devicePixelRatio();
    const QPoint globalPos2 = QPointF(QPointF(globalPos) * dpr).toPoint();
    doStartSystemMoveResize(window->winId(), globalPos2, section);
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
