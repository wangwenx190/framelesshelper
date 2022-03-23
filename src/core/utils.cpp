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

// The "Q_INIT_RESOURCE()" macro can't be used within a namespace,
// so we wrap it into a separate function outside of the namespace and
// then call it instead inside the namespace, that's also the recommended
// workaround provided by Qt's official documentation.
static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelpercore);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

static const QString kImageResourcePrefix = QStringLiteral(":/org.wangwenx190.FramelessHelper/images");

Qt::CursorShape Utils::calculateCursorShape(const QWindow *window, const QPoint &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    if (((pos.x() < kDefaultResizeBorderThickness) && (pos.y() < kDefaultResizeBorderThickness))
        || ((pos.x() >= (window->width() - kDefaultResizeBorderThickness)) && (pos.y() >= (window->height() - kDefaultResizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((pos.x() >= (window->width() - kDefaultResizeBorderThickness)) && (pos.y() < kDefaultResizeBorderThickness))
        || ((pos.x() < kDefaultResizeBorderThickness) && (pos.y() >= (window->height() - kDefaultResizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((pos.x() < kDefaultResizeBorderThickness) || (pos.x() >= (window->width() - kDefaultResizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((pos.y() < kDefaultResizeBorderThickness) || (pos.y() >= (window->height() - kDefaultResizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

Qt::Edges Utils::calculateWindowEdges(const QWindow *window, const QPoint &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    if (pos.x() < kDefaultResizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= (window->width() - kDefaultResizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() < kDefaultResizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= (window->height() - kDefaultResizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

bool Utils::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    if (window->flags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    return (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize));
}

QVariant Utils::getSystemButtonIconResource
    (const SystemButtonType button, const SystemTheme theme, const ResourceType type)
{
    const QString resourceUri = [button, theme]() -> QString {
        const QString szButton = [button]() -> QString {
            switch (button) {
            case SystemButtonType::WindowIcon:
                break;
            case SystemButtonType::Minimize:
                return QStringLiteral("minimize");
            case SystemButtonType::Maximize:
                return QStringLiteral("maximize");
            case SystemButtonType::Restore:
                return QStringLiteral("restore");
            case SystemButtonType::Close:
                return QStringLiteral("close");
            }
            return {};
        }();
        const QString szTheme = [theme]() -> QString {
            switch (theme) {
            case SystemTheme::Light:
                return QStringLiteral("light");
            case SystemTheme::Dark:
                return QStringLiteral("dark");
            case SystemTheme::HighContrastLight:
                return QStringLiteral("hc-light");
            case SystemTheme::HighContrastDark:
                return QStringLiteral("hc-dark");
            }
            return {};
        }();
        return QStringLiteral("%1/%2/chrome-%3.svg").arg(kImageResourcePrefix, szTheme, szButton);
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

QWindow *Utils::findWindow(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == winId) {
                return window;
            }
        }
    }
    return nullptr;
}

void Utils::moveWindowToDesktopCenter(const GetWindowScreenCallback &getWindowScreen,
                                      const GetWindowSizeCallback &getWindowSize,
                                      const MoveWindowCallback &moveWindow,
                                      const bool considerTaskBar)
{
    Q_ASSERT(getWindowScreen);
    Q_ASSERT(getWindowSize);
    Q_ASSERT(moveWindow);
    if (!getWindowScreen || !getWindowSize || !moveWindow) {
        return;
    }
    const QSize windowSize = getWindowSize();
    if (windowSize.isEmpty() || (windowSize == kInvalidWindowSize)) {
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
    moveWindow(newX + offset.x(), newY + offset.y());
}

FRAMELESSHELPER_END_NAMESPACE
