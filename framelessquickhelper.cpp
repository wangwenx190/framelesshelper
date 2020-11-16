/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include "framelessquickhelper.h"

#include "framelesswindowsmanager.h"
#include <QQuickWindow>
#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#include <QOperatingSystemVersion>
#endif

namespace {

#ifdef Q_OS_WINDOWS
const char g_sPreserveWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char g_sDontExtendFrame[] = "WNEF_DO_NOT_EXTEND_FRAME";
const char g_sForceUseAcrylicEffect[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";
#endif

FramelessWindowsManager::WindowId getWindowId(QObject *object)
{
    Q_ASSERT(object);
    if (!object->isWindowType()) {
        qWarning() << object << "is not a window!";
        return nullptr;
    }
    const auto window = qobject_cast<QWindow *>(object);
    if (!window) {
        qWarning() << "Failed to convert" << object << "to QWindow!";
        return nullptr;
    }
#ifdef Q_OS_WINDOWS
    return reinterpret_cast<FramelessWindowsManager::WindowId>(window->winId());
#else
    return static_cast<FramelessWindowsManager::WindowId>(window);
#endif
}

} // namespace

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent) : QQuickItem(parent)
{
#ifdef Q_OS_WINDOWS
    startTimer(500);
#endif
}

int FramelessQuickHelper::borderWidth() const
{
    return FramelessWindowsManager::getBorderWidth(getWindowId(window()));
}

void FramelessQuickHelper::setBorderWidth(const int val)
{
    FramelessWindowsManager::setBorderWidth(getWindowId(window()), val);
    Q_EMIT borderWidthChanged(val);
}

int FramelessQuickHelper::borderHeight() const
{
    return FramelessWindowsManager::getBorderHeight(getWindowId(window()));
}

void FramelessQuickHelper::setBorderHeight(const int val)
{
    FramelessWindowsManager::setBorderHeight(getWindowId(window()), val);
    Q_EMIT borderHeightChanged(val);
}

int FramelessQuickHelper::titleBarHeight() const
{
    return FramelessWindowsManager::getTitleBarHeight(getWindowId(window()));
}

void FramelessQuickHelper::setTitleBarHeight(const int val)
{
    FramelessWindowsManager::setTitleBarHeight(getWindowId(window()), val);
    Q_EMIT titleBarHeightChanged(val);
}

bool FramelessQuickHelper::resizable() const
{
    return FramelessWindowsManager::getResizable(getWindowId(window()));
}

void FramelessQuickHelper::setResizable(const bool val)
{
    FramelessWindowsManager::setResizable(getWindowId(window()), val);
    Q_EMIT resizableChanged(val);
}

bool FramelessQuickHelper::titleBarEnabled() const
{
    return FramelessWindowsManager::getTitleBarEnabled(getWindowId(window()));
}

void FramelessQuickHelper::setTitleBarEnabled(const bool val)
{
    FramelessWindowsManager::setTitleBarEnabled(getWindowId(window()), val);
    Q_EMIT titleBarEnabledChanged(val);
}

#ifdef Q_OS_WINDOWS
bool FramelessQuickHelper::canHaveWindowFrame() const
{
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
}

bool FramelessQuickHelper::colorizationEnabled() const
{
    return WinNativeEventFilter::isColorizationEnabled();
}

QColor FramelessQuickHelper::colorizationColor() const
{
    return WinNativeEventFilter::getColorizationColor();
}

bool FramelessQuickHelper::lightThemeEnabled() const
{
    return WinNativeEventFilter::isLightThemeEnabled();
}

bool FramelessQuickHelper::darkThemeEnabled() const
{
    return WinNativeEventFilter::isDarkThemeEnabled();
}

bool FramelessQuickHelper::highContrastModeEnabled() const
{
    return WinNativeEventFilter::isHighContrastModeEnabled();
}

bool FramelessQuickHelper::darkFrameEnabled() const
{
    return WinNativeEventFilter::isDarkFrameEnabled(rawWindowHandle());
}

bool FramelessQuickHelper::transparencyEffectEnabled() const
{
    return WinNativeEventFilter::isTransparencyEffectEnabled();
}
#endif

QSize FramelessQuickHelper::minimumSize() const
{
    return FramelessWindowsManager::getMinimumSize(getWindowId(window()));
}

void FramelessQuickHelper::setMinimumSize(const QSize &val)
{
    FramelessWindowsManager::setMinimumSize(getWindowId(window()), val);
    Q_EMIT minimumSizeChanged(val);
}

QSize FramelessQuickHelper::maximumSize() const
{
    return FramelessWindowsManager::getMaximumSize(getWindowId(window()));
}

void FramelessQuickHelper::setMaximumSize(const QSize &val)
{
    FramelessWindowsManager::setMaximumSize(getWindowId(window()), val);
    Q_EMIT maximumSizeChanged(val);
}

void FramelessQuickHelper::removeWindowFrame(const bool center)
{
    FramelessWindowsManager::addWindow(getWindowId(window()), center);
}

void FramelessQuickHelper::moveWindowToDesktopCenter()
{
    FramelessWindowsManager::moveWindowToDesktopCenter(getWindowId(window()));
}

void FramelessQuickHelper::addIgnoreArea(const QRect &val)
{
    FramelessWindowsManager::addIgnoreArea(getWindowId(window()), val);
}

void FramelessQuickHelper::addDraggableArea(const QRect &val)
{
    FramelessWindowsManager::addDraggableArea(getWindowId(window()), val);
}

void FramelessQuickHelper::addIgnoreObject(QQuickItem *val)
{
    Q_ASSERT(val);
    FramelessWindowsManager::addIgnoreObject(getWindowId(window()), val);
}

void FramelessQuickHelper::addDraggableObject(QQuickItem *val)
{
    Q_ASSERT(val);
    FramelessWindowsManager::addDraggableObject(getWindowId(window()), val);
}

#ifdef Q_OS_WINDOWS
void FramelessQuickHelper::timerEvent(QTimerEvent *event)
{
    QQuickItem::timerEvent(event);
    Q_EMIT colorizationEnabledChanged(colorizationEnabled());
    Q_EMIT colorizationColorChanged(colorizationColor());
    Q_EMIT lightThemeEnabledChanged(lightThemeEnabled());
    Q_EMIT darkThemeEnabledChanged(darkThemeEnabled());
    Q_EMIT highContrastModeEnabledChanged(highContrastModeEnabled());
    Q_EMIT darkFrameEnabledChanged(darkFrameEnabled());
    Q_EMIT transparencyEffectEnabledChanged(transparencyEffectEnabled());
}

void *FramelessQuickHelper::rawWindowHandle() const
{
    const QWindow *win = window();
    if (win) {
        return reinterpret_cast<void *>(win->winId());
    }
    return nullptr;
}

void FramelessQuickHelper::setWindowFrameVisible(const bool value)
{
    if (value) {
        qputenv(g_sPreserveWindowFrame, "1");
        qputenv(g_sDontExtendFrame, "1");
    } else {
        qunsetenv(g_sPreserveWindowFrame);
        qunsetenv(g_sDontExtendFrame);
    }
}

void FramelessQuickHelper::displaySystemMenu(const QPointF &pos)
{
    WinNativeEventFilter::displaySystemMenu(rawWindowHandle(), pos);
}

void FramelessQuickHelper::setBlurEffectEnabled(const bool enabled,
                                                const bool forceAcrylic,
                                                const QColor &gradientColor)
{
    if (forceAcrylic) {
        qputenv(g_sForceUseAcrylicEffect, "1");
    } else {
        qunsetenv(g_sForceUseAcrylicEffect);
    }
    WinNativeEventFilter::setBlurEffectEnabled(rawWindowHandle(), enabled, gradientColor);
}
#endif
