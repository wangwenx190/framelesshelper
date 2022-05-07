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
#include <QtCore/qvariant.h>
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif

// The "Q_INIT_RESOURCE()" macro can't be used within a namespace,
// so we wrap it into a separate function outside of the namespace and
// then call it instead inside the namespace, that's also the recommended
// workaround provided by Qt's official documentation.
static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelpercore);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(ImageResourcePrefix, ":/org.wangwenx190.FramelessHelper/images")
FRAMELESSHELPER_STRING_CONSTANT2(SystemButtonImageResourceTemplate, "%1/%2/chrome-%3.svg")
FRAMELESSHELPER_STRING_CONSTANT(windowicon)
FRAMELESSHELPER_STRING_CONSTANT(help)
FRAMELESSHELPER_STRING_CONSTANT(minimize)
FRAMELESSHELPER_STRING_CONSTANT(maximize)
FRAMELESSHELPER_STRING_CONSTANT(restore)
FRAMELESSHELPER_STRING_CONSTANT(close)
FRAMELESSHELPER_STRING_CONSTANT(light)
FRAMELESSHELPER_STRING_CONSTANT(dark)
FRAMELESSHELPER_STRING_CONSTANT(highcontrast)

Qt::CursorShape Utils::calculateCursorShape(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return Qt::ArrowCursor;
#else
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    const int x = pos.x();
    const int y = pos.y();
    const int w = window->width();
    const int h = window->height();
    if (((x < kDefaultResizeBorderThickness) && (y < kDefaultResizeBorderThickness))
        || ((x >= (w - kDefaultResizeBorderThickness)) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((x >= (w - kDefaultResizeBorderThickness)) && (y < kDefaultResizeBorderThickness))
        || ((x < kDefaultResizeBorderThickness) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((x < kDefaultResizeBorderThickness) || (x >= (w - kDefaultResizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((y < kDefaultResizeBorderThickness) || (y >= (h - kDefaultResizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
#endif
}

Qt::Edges Utils::calculateWindowEdges(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return {};
#else
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    const int x = pos.x();
    const int y = pos.y();
    if (x < kDefaultResizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (x >= (window->width() - kDefaultResizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (y < kDefaultResizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (y >= (window->height() - kDefaultResizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
#endif
}

QVariant Utils::getSystemButtonIconResource
    (const SystemButtonType button, const SystemTheme theme, const ResourceType type)
{
    const QString resourceUri = [button, theme]() -> QString {
        const QString szButton = [button]() -> QString {
            switch (button) {
            case SystemButtonType::Unknown:
                return {};
            case SystemButtonType::WindowIcon:
                return kwindowicon;
            case SystemButtonType::Help:
                return khelp;
            case SystemButtonType::Minimize:
                return kminimize;
            case SystemButtonType::Maximize:
                return kmaximize;
            case SystemButtonType::Restore:
                return krestore;
            case SystemButtonType::Close:
                return kclose;
            }
            return {};
        }();
        const QString szTheme = [theme]() -> QString {
            switch (theme) {
            case SystemTheme::Unknown:
                return {};
            case SystemTheme::Light:
                return klight;
            case SystemTheme::Dark:
                return kdark;
            case SystemTheme::HighContrast:
                return khighcontrast;
            }
            return {};
        }();
        return kSystemButtonImageResourceTemplate.arg(kImageResourcePrefix, szTheme, szButton);
    }();
    initResource();
    switch (type) {
    case ResourceType::Image:
        return QImage(resourceUri);
    case ResourceType::Pixmap:
        return QPixmap(resourceUri);
    case ResourceType::Icon:
        return QIcon(resourceUri);
    }
    return {};
}

QWindow *Utils::findWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == windowId) {
                return window;
            }
        }
    }
    return nullptr;
}

void Utils::moveWindowToDesktopCenter(const GetWindowScreenCallback &getWindowScreen,
                                      const GetWindowSizeCallback &getWindowSize,
                                      const SetWindowPositionCallback &setWindowPosition,
                                      const bool considerTaskBar)
{
    Q_ASSERT(getWindowScreen);
    Q_ASSERT(getWindowSize);
    Q_ASSERT(setWindowPosition);
    if (!getWindowScreen || !getWindowSize || !setWindowPosition) {
        return;
    }
    const QSize windowSize = getWindowSize();
    if (windowSize.isEmpty() || (windowSize == kDefaultWindowSize)) {
        return;
    }
    const QScreen *screen = getWindowScreen();
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    Q_ASSERT(screen);
    if (!screen) {
        return;
    }
    const QSize screenSize = (considerTaskBar ? screen->availableSize() : screen->size());
    const QPoint offset = (considerTaskBar ? screen->availableGeometry().topLeft() : QPoint(0, 0));
    const auto newX = static_cast<int>(qRound(qreal(screenSize.width() - windowSize.width()) / 2.0));
    const auto newY = static_cast<int>(qRound(qreal(screenSize.height() - windowSize.height()) / 2.0));
    setWindowPosition(QPoint(newX + offset.x(), newY + offset.y()));
}

Qt::WindowState Utils::windowStatesToWindowState(const Qt::WindowStates states)
{
    if (states & Qt::WindowFullScreen) {
        return Qt::WindowFullScreen;
    }
    if (states & Qt::WindowMaximized) {
        return Qt::WindowMaximized;
    }
    if (states & Qt::WindowMinimized) {
        return Qt::WindowMinimized;
    }
    return Qt::WindowNoState;
}

bool Utils::isThemeChangeEvent(const QEvent * const event)
{
    Q_ASSERT(event);
    if (!event) {
        return false;
    }
    const QEvent::Type type = event->type();
    return ((type == QEvent::ThemeChange) || (type == QEvent::ApplicationPaletteChange));
}

QColor Utils::calculateSystemButtonBackgroundColor(const SystemButtonType button, const ButtonState state)
{
    if (state == ButtonState::Unspecified) {
        return kDefaultTransparentColor;
    }
    const QColor result = [button]() -> QColor {
        if (button == SystemButtonType::Close) {
            return kDefaultSystemCloseButtonBackgroundColor;
        }
        if (isTitleBarColorized()) {
#ifdef Q_OS_WINDOWS
            return getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
            return getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
            return getControlsAccentColor();
#endif
        }
        return kDefaultSystemButtonBackgroundColor;
    }();
    return ((state == ButtonState::Hovered) ? result.lighter(110) : result.lighter(105));
}

bool Utils::shouldAppsUseDarkMode()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
#else
#  ifdef Q_OS_WINDOWS
    return shouldAppsUseDarkMode_windows();
#  elif defined(Q_OS_LINUX)
    return shouldAppsUseDarkMode_linux();
#  elif defined(Q_OS_MACOS)
    return shouldAppsUseDarkMode_macos();
#  else
    return false;
#  endif
#endif
}

FRAMELESSHELPER_END_NAMESPACE
