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

#include "framelessquickutils.h"
#include <QtQuick/qquickwindow.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif
#include <framelesswindowsmanager.h>
#include <utils.h>
#ifdef Q_OS_WINDOWS
#  include <framelesshelper_windows.h>
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessQuickUtils::FramelessQuickUtils(QObject *parent) : QObject(parent)
{
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, [this](){
        Q_EMIT frameBorderActiveColorChanged();
        Q_EMIT frameBorderInactiveColorChanged();
        Q_EMIT darkModeEnabledChanged();
        Q_EMIT systemAccentColorChanged();
        Q_EMIT titleBarColorVisibleChanged();
    });
}

FramelessQuickUtils::~FramelessQuickUtils() = default;

qreal FramelessQuickUtils::titleBarHeight()
{
    return 30;
}

bool FramelessQuickUtils::frameBorderVisible()
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater());
#else
    return false;
#endif
}

qreal FramelessQuickUtils::frameBorderThickness()
{
    return 1;
}

QColor FramelessQuickUtils::frameBorderActiveColor()
{
#ifdef Q_OS_WINDOWS
    return Utils::getFrameBorderColor(true);
#else
    return {};
#endif
}

QColor FramelessQuickUtils::frameBorderInactiveColor()
{
#ifdef Q_OS_WINDOWS
    return Utils::getFrameBorderColor(false);
#else
    return {};
#endif
}

bool FramelessQuickUtils::darkModeEnabled()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
#else
#  ifdef Q_OS_WINDOWS
    return Utils::shouldAppsUseDarkMode();
#  else
    return false;
#  endif
#endif
}

QColor FramelessQuickUtils::systemAccentColor()
{
#ifdef Q_OS_WINDOWS
    return Utils::getDwmColorizationColor();
#else
    return {};
#endif
}

bool FramelessQuickUtils::titleBarColorVisible()
{
#ifdef Q_OS_WINDOWS
    return Utils::isTitleBarColorized();
#else
    return false;
#endif
}

void FramelessQuickUtils::showMinimized2(QQuickWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef Q_OS_WINDOWS
    // Work-around a QtQuick bug: https://bugreports.qt.io/browse/QTBUG-69711
    // Don't use "SW_SHOWMINIMIZED" because it will activate the current window
    // instead of the next window in the Z order, which is not the default behavior
    // of native Win32 applications.
    ShowWindow(reinterpret_cast<HWND>(window->winId()), SW_MINIMIZE);
#else
    window->showMinimized();
#endif
}

void FramelessQuickUtils::showSystemMenu(QQuickWindow *window, const QPoint &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint globalPos = window->mapToGlobal(pos);
#  else
    const QPoint globalPos = window->mapToGlobal(pos);
#  endif
    const QPoint nativePos = QPointF(QPointF(globalPos) * window->effectiveDevicePixelRatio()).toPoint();
    Utils::showSystemMenu(window->winId(), nativePos);
#endif
}

void FramelessQuickUtils::startSystemMove2(QQuickWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
#else
    Utils::startSystemMove(window);
#endif
}

void FramelessQuickUtils::startSystemResize2(QQuickWindow *window, const Qt::Edges edges)
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
    Utils::startSystemResize(window, edges);
#endif
}

FRAMELESSHELPER_END_NAMESPACE
