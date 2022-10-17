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
#ifdef Q_OS_WINDOWS
#  include "winverhelper_p.h"
#endif
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#  include <QtGui/qstylehints.h>
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcUtilsCommon, "wangwenx190.framelesshelper.core.utils.common")
#define INFO qCInfo(lcUtilsCommon)
#define DEBUG qCDebug(lcUtilsCommon)
#define WARNING qCWarning(lcUtilsCommon)
#define CRITICAL qCCritical(lcUtilsCommon)

using namespace Global;

struct FONT_ICON
{
    quint32 segoe = 0;
    quint32 micon = 0;
};

static const QHash<int, FONT_ICON> g_fontIconsTable = {
    {static_cast<int>(SystemButtonType::Unknown), {0x0000, 0x0000}},
    {static_cast<int>(SystemButtonType::WindowIcon), {0xE756, 0xEB06}},
    {static_cast<int>(SystemButtonType::Help), {0xE897, 0xEC04}},
    {static_cast<int>(SystemButtonType::Minimize), {0xE921, 0xEAE0}},
    {static_cast<int>(SystemButtonType::Maximize), {0xE922, 0xEADE}},
    {static_cast<int>(SystemButtonType::Restore), {0xE923, 0xEAE2}},
    {static_cast<int>(SystemButtonType::Close), {0xE8BB, 0xEADA}}
};

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

QString Utils::getSystemButtonIconCode(const SystemButtonType button)
{
    const auto index = static_cast<int>(button);
    if (!g_fontIconsTable.contains(index)) {
        WARNING << "FIXME: Add FONT_ICON value for button" << button;
        return {};
    }
    const FONT_ICON icon = g_fontIconsTable.value(index);
#ifdef Q_OS_WINDOWS
    // Windows 11: Segoe Fluent Icons (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-fluent-icons-font)
    // Windows 10: Segoe MDL2 Assets (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font)
    // Windows 7~8.1: Micon (http://xtoolkit.github.io/Micon/)
    if (WindowsVersionHelper::isWin10OrGreater()) {
        return QChar(icon.segoe);
    }
#endif
    // We always use Micon on UNIX platforms because Microsoft doesn't allow distributing
    // the Segoe icon font to other platforms than Windows.
    return QChar(icon.micon);
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
    const int newX = qRound(qreal(screenSize.width() - windowSize.width()) / 2.0);
    const int newY = qRound(qreal(screenSize.height() - windowSize.height()) / 2.0);
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
    // QGuiApplication will only deliver theme change events to top level Q(Quick)Windows,
    // QWidgets won't get such notifications, no matter whether it's top level widget or not.
    // QEvent::ThemeChange: Send by the Windows QPA.
    // QEvent::ApplicationPaletteChange: All other platforms (Linux & macOS).
    const QEvent::Type type = event->type();
    return ((type == QEvent::ThemeChange) || (type == QEvent::ApplicationPaletteChange));
}

QColor Utils::calculateSystemButtonBackgroundColor(const SystemButtonType button, const ButtonState state)
{
    if (state == ButtonState::Unspecified) {
        return kDefaultTransparentColor;
    }
    const bool isClose = (button == SystemButtonType::Close);
    const bool isTitleColor = isTitleBarColorized();
    const bool isHovered = (state == ButtonState::Hovered);
    const QColor result = [isClose, isTitleColor]() -> QColor {
        if (isClose) {
            return kDefaultSystemCloseButtonBackgroundColor;
        }
        if (isTitleColor) {
#ifdef Q_OS_WINDOWS
            return getDwmAccentColor();
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
    if (isClose) {
        return (isHovered ? result.lighter(110) : result.lighter(140));
    }
    if (!isTitleColor) {
        return (isHovered ? result.lighter(110) : result);
    }
    return (isHovered ? result.lighter(150) : result.lighter(120));
}

bool Utils::shouldAppsUseDarkMode()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    return (QGuiApplication::styleHints()->appearance() == Qt::Appearance::Dark);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
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
