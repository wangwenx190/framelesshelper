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

#include <QQuickWindow>
#include <QScreen>
#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include "framelesshelper.h"
#endif

namespace {
#ifdef Q_OS_WINDOWS
const int m_defaultBorderWidth = 8, m_defaultBorderHeight = 8, m_defaultTitleBarHeight = 30;
#else
FramelessHelper m_framelessHelper;
#endif
} // namespace

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent) : QQuickItem(parent) {}

int FramelessQuickHelper::borderWidth() const
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        return WinNativeEventFilter::getSystemMetric(win,
                                                     WinNativeEventFilter::SystemMetric::BorderWidth);
    }
    return m_defaultBorderWidth;
#else
    return m_framelessHelper.getBorderWidth();
#endif
}

void FramelessQuickHelper::setBorderWidth(const int val)
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->borderWidth = val;
            Q_EMIT borderWidthChanged(val);
        }
    }
#else
    m_framelessHelper.setBorderWidth(val);
    Q_EMIT borderWidthChanged(val);
#endif
}

int FramelessQuickHelper::borderHeight() const
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        return WinNativeEventFilter::getSystemMetric(
            win, WinNativeEventFilter::SystemMetric::BorderHeight);
    }
    return m_defaultBorderHeight;
#else
    return m_framelessHelper.getBorderHeight();
#endif
}

void FramelessQuickHelper::setBorderHeight(const int val)
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->borderHeight = val;
            Q_EMIT borderHeightChanged(val);
        }
    }
#else
    m_framelessHelper.setBorderHeight(val);
    Q_EMIT borderHeightChanged(val);
#endif
}

int FramelessQuickHelper::titleBarHeight() const
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        return WinNativeEventFilter::getSystemMetric(
            win, WinNativeEventFilter::SystemMetric::TitleBarHeight);
    }
    return m_defaultTitleBarHeight;
#else
    return m_framelessHelper.getTitleBarHeight();
#endif
}

void FramelessQuickHelper::setTitleBarHeight(const int val)
{
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->titleBarHeight = val;
            Q_EMIT titleBarHeightChanged(val);
        }
    }
#else
    m_framelessHelper.setTitleBarHeight(val);
    Q_EMIT titleBarHeightChanged(val);
#endif
}

bool FramelessQuickHelper::resizable() const
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            return !data->fixedSize;
        }
#else
        return m_framelessHelper.getResizable(win);
#endif
    }
    return true;
}

void FramelessQuickHelper::setResizable(const bool val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->fixedSize = !val;
            Q_EMIT resizableChanged(val);
        }
#else
        m_framelessHelper.setResizable(win, val);
        Q_EMIT resizableChanged(val);
#endif
    }
}

bool FramelessQuickHelper::titleBarEnabled() const
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            return !data->disableTitleBar;
        }
#else
        return m_framelessHelper.getTitleBarEnabled(win);
#endif
    }
    return true;
}

void FramelessQuickHelper::setTitleBarEnabled(const bool val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->disableTitleBar = !val;
            Q_EMIT titleBarEnabledChanged(val);
        }
#else
        m_framelessHelper.setTitleBarEnabled(win, val);
        Q_EMIT titleBarEnabledChanged(val);
#endif
    }
}

QSize FramelessQuickHelper::minimumSize() const
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            return data->minimumSize;
        }
#else
        return win->minimumSize();
#endif
    }
    return {};
}

void FramelessQuickHelper::setMinimumSize(const QSize &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->minimumSize = val;
            Q_EMIT minimumSizeChanged(val);
        }
#else
        win->setMinimumSize(val);
        Q_EMIT minimumSizeChanged(val);
#endif
    }
}

QSize FramelessQuickHelper::maximumSize() const
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            return data->maximumSize;
        }
#else
        return win->maximumSize();
#endif
    }
    return {};
}

void FramelessQuickHelper::setMaximumSize(const QSize &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->maximumSize = val;
            Q_EMIT maximumSizeChanged(val);
        }
#else
        win->setMaximumSize(val);
        Q_EMIT maximumSizeChanged(val);
#endif
    }
}

void FramelessQuickHelper::removeWindowFrame(const bool center)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        WinNativeEventFilter::addFramelessWindow(win);
#else
        m_framelessHelper.removeWindowFrame(win);
#endif
    }
    if (center) {
        moveWindowToDesktopCenter();
    }
}

QSize FramelessQuickHelper::desktopSize() const
{
    const auto win = window();
    if (win) {
        const auto screen = win->screen();
        if (screen) {
            return screen->size();
        }
    }
    return {};
}

QRect FramelessQuickHelper::desktopAvailableGeometry() const
{
    const auto win = window();
    if (win) {
        const auto screen = win->screen();
        if (screen) {
            return screen->availableGeometry();
        }
    }
    return {};
}

QSize FramelessQuickHelper::desktopAvailableSize() const
{
    const auto win = window();
    if (win) {
        const auto screen = win->screen();
        if (screen) {
            return screen->availableSize();
        }
    }
    return {};
}

void FramelessQuickHelper::moveWindowToDesktopCenter(const bool realCenter)
{
    const auto win = window();
    if (win) {
        if (realCenter) {
#ifdef Q_OS_WINDOWS
            WinNativeEventFilter::moveWindowToDesktopCenter(win);
#else
            FramelessHelper::moveWindowToDesktopCenter(win);
#endif
        } else {
            const QSize windowSize = win->size();
            const QSize screenSize = desktopAvailableSize();
            const int newX = qRound(static_cast<qreal>(screenSize.width() - windowSize.width())
                                    / 2.0);
            const int newY = qRound(static_cast<qreal>(screenSize.height() - windowSize.height())
                                    / 2.0);
            const QRect screenGeometry = desktopAvailableGeometry();
            win->setX(newX + screenGeometry.x());
            win->setY(newY + screenGeometry.y());
        }
    }
}

void FramelessQuickHelper::setIgnoreAreas(const QList<QRect> &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreAreas = val;
        }
#else
        m_framelessHelper.setIgnoreAreas(win, val);
#endif
    }
}

void FramelessQuickHelper::clearIgnoreAreas()
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreAreas.clear();
        }
#else
        m_framelessHelper.clearIgnoreAreas(win);
#endif
    }
}

void FramelessQuickHelper::addIgnoreArea(const QRect &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreAreas.append(val);
        }
#else
        m_framelessHelper.addIgnoreArea(win, val);
#endif
    }
}

void FramelessQuickHelper::setDraggableAreas(const QList<QRect> &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableAreas = val;
        }
#else
        m_framelessHelper.setDraggableAreas(win, val);
#endif
    }
}

void FramelessQuickHelper::clearDraggableAreas()
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableAreas.clear();
        }
#else
        m_framelessHelper.clearDraggableAreas(win);
#endif
    }
}

void FramelessQuickHelper::addDraggableArea(const QRect &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableAreas.append(val);
        }
#else
        m_framelessHelper.addDraggableArea(win, val);
#endif
    }
}

void FramelessQuickHelper::setIgnoreObjects(const QList<QQuickItem *> &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreObjects.clear();
            if (!val.isEmpty()) {
                for (auto &&obj : qAsConst(val)) {
                    data->ignoreObjects.append(obj);
                }
            }
        }
#else
        QList<QObject *> objs{};
        if (!val.isEmpty()) {
            for (auto &&obj : qAsConst(val)) {
                objs.append(obj);
            }
            m_framelessHelper.setIgnoreObjects(win, objs);
        }
#endif
    }
}

void FramelessQuickHelper::clearIgnoreObjects()
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreObjects.clear();
        }
#else
        m_framelessHelper.clearIgnoreObjects(win);
#endif
    }
}

void FramelessQuickHelper::addIgnoreObject(QQuickItem *val)
{
    Q_ASSERT(val);
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->ignoreObjects.append(val);
        }
#else
        m_framelessHelper.addIgnoreObject(win, val);
#endif
    }
}

void FramelessQuickHelper::setDraggableObjects(const QList<QQuickItem *> &val)
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableObjects.clear();
            if (!val.isEmpty()) {
                for (auto &&obj : qAsConst(val)) {
                    data->draggableObjects.append(obj);
                }
            }
        }
#else
        QList<QObject *> objs{};
        if (!val.isEmpty()) {
            for (auto &&obj : qAsConst(val)) {
                objs.append(obj);
            }
            m_framelessHelper.setDraggableObjects(win, objs);
        }
#endif
    }
}

void FramelessQuickHelper::clearDraggableObjects()
{
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableObjects.clear();
        }
#else
        m_framelessHelper.clearDraggableObjects(win);
#endif
    }
}

void FramelessQuickHelper::addDraggableObject(QQuickItem *val)
{
    Q_ASSERT(val);
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto data = WinNativeEventFilter::windowData(win);
        if (data) {
            data->draggableObjects.append(val);
        }
#else
        m_framelessHelper.addDraggableObject(win, val);
#endif
    }
}
