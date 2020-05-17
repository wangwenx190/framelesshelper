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

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#endif
#include <QQuickWindow>

#ifdef Q_OS_WINDOWS
namespace {

const int m_defaultBorderWidth = 8, m_defaultBorderHeight = 8,
          m_defaultTitleBarHeight = 30;

}
#endif

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent)
    : QQuickItem(parent) {}

int FramelessQuickHelper::borderWidth() const {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            return WinNativeEventFilter::getSystemMetric(
                hWnd, WinNativeEventFilter::SystemMetric::BorderWidth, false);
        }
    }
    return m_defaultBorderWidth;
#else
    return m_framelessHelper.getBorderWidth();
#endif
}

void FramelessQuickHelper::setBorderWidth(const int val) {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->borderWidth = val;
                Q_EMIT borderWidthChanged(val);
            }
        }
    }
#else
    m_framelessHelper.setBorderWidth(val);
    Q_EMIT borderWidthChanged(val);
#endif
}

int FramelessQuickHelper::borderHeight() const {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            return WinNativeEventFilter::getSystemMetric(
                hWnd, WinNativeEventFilter::SystemMetric::BorderHeight, false);
        }
    }
    return m_defaultBorderHeight;
#else
    return m_framelessHelper.getBorderHeight();
#endif
}

void FramelessQuickHelper::setBorderHeight(const int val) {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->borderHeight = val;
                Q_EMIT borderHeightChanged(val);
            }
        }
    }
#else
    m_framelessHelper.setBorderHeight(val);
    Q_EMIT borderHeightChanged(val);
#endif
}

int FramelessQuickHelper::titleBarHeight() const {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            return WinNativeEventFilter::getSystemMetric(
                hWnd, WinNativeEventFilter::SystemMetric::TitleBarHeight,
                false);
        }
    }
    return m_defaultTitleBarHeight;
#else
    return m_framelessHelper.getTitleBarHeight();
#endif
}

void FramelessQuickHelper::setTitleBarHeight(const int val) {
#ifdef Q_OS_WINDOWS
    const auto win = window();
    if (win) {
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->titleBarHeight = val;
                Q_EMIT titleBarHeightChanged(val);
            }
        }
    }
#else
    m_framelessHelper.setTitleBarHeight(val);
    Q_EMIT titleBarHeightChanged(val);
#endif
}

bool FramelessQuickHelper::resizable() const {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                return !data->fixedSize;
            }
        }
#else
        return m_framelessHelper.getResizable(win);
#endif
    }
    return true;
}

void FramelessQuickHelper::setResizable(const bool val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->fixedSize = !val;
                Q_EMIT resizableChanged(val);
            }
        }
#else
        m_framelessHelper.setResizable(win, val);
#endif
    }
}

bool FramelessQuickHelper::titleBarEnabled() const {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                return !data->disableTitleBar;
            }
        }
#else
        return m_framelessHelper.getTitleBarEnabled(win);
#endif
    }
    return true;
}

void FramelessQuickHelper::setTitleBarEnabled(const bool val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->disableTitleBar = !val;
                Q_EMIT titleBarEnabledChanged(val);
            }
        }
#else
        m_framelessHelper.setTitleBarEnabled(win, val);
#endif
    }
}

QSize FramelessQuickHelper::minimumSize() const {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                return data->minimumSize;
            }
        }
#else
        return win->minimumSize();
#endif
    }
    return {0, 0};
}

void FramelessQuickHelper::setMinimumSize(const QSize &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->minimumSize = val;
                Q_EMIT minimumSizeChanged(val);
            }
        }
#else
        win->setMinimumSize(val);
        Q_EMIT minimumSizeChanged(val);
#endif
    }
}

QSize FramelessQuickHelper::maximumSize() const {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                return data->maximumSize;
            }
        }
#else
        return win->maximumSize();
#endif
    }
    return {0, 0};
}

void FramelessQuickHelper::setMaximumSize(const QSize &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->maximumSize = val;
                Q_EMIT maximumSizeChanged(val);
            }
        }
#else
        win->setMaximumSize(val);
        Q_EMIT maximumSizeChanged(val);
#endif
    }
}

void FramelessQuickHelper::removeWindowFrame(const bool center) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            WinNativeEventFilter::addFramelessWindow(hWnd);
        }
#else
        m_framelessHelper.removeWindowFrame(win);
#endif
    }
    if (center) {
        moveWindowToDesktopCenter();
    }
}

void FramelessQuickHelper::moveWindowToDesktopCenter() {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            WinNativeEventFilter::moveWindowToDesktopCenter(hWnd);
        }
#else
        FramelessHelper::moveWindowToDesktopCenter(win);
#endif
    }
}

void FramelessQuickHelper::setIgnoreAreas(const QVector<QRect> &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreAreas = val;
            }
        }
#else
        m_framelessHelper.setIgnoreAreas(win, val);
#endif
    }
}

void FramelessQuickHelper::clearIgnoreAreas() {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreAreas.clear();
            }
        }
#else
        m_framelessHelper.clearIgnoreAreas(win);
#endif
    }
}

void FramelessQuickHelper::addIgnoreArea(const QRect &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreAreas.append(val);
            }
        }
#else
        m_framelessHelper.addIgnoreArea(win, val);
#endif
    }
}

void FramelessQuickHelper::setDraggableAreas(const QVector<QRect> &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableAreas = val;
            }
        }
#else
        m_framelessHelper.setDraggableAreas(win, val);
#endif
    }
}

void FramelessQuickHelper::clearDraggableAreas() {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableAreas.clear();
            }
        }
#else
        m_framelessHelper.clearDraggableAreas(win);
#endif
    }
}

void FramelessQuickHelper::addDraggableArea(const QRect &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableAreas.append(val);
            }
        }
#else
        m_framelessHelper.addDraggableArea(win, val);
#endif
    }
}

void FramelessQuickHelper::setIgnoreObjects(const QVector<QQuickItem *> &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreObjects.clear();
                if (!val.isEmpty()) {
                    for (auto &&obj : qAsConst(val)) {
                        data->ignoreObjects.append(obj);
                    }
                }
            }
        }
#else
        QVector<QObject *> objs{};
        if (!val.isEmpty()) {
            for (auto &&obj : qAsConst(val)) {
                objs.append(obj);
            }
            m_framelessHelper.setIgnoreObjects(win, objs);
        }
#endif
    }
}

void FramelessQuickHelper::clearIgnoreObjects() {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreObjects.clear();
            }
        }
#else
        m_framelessHelper.clearIgnoreObjects(win);
#endif
    }
}

void FramelessQuickHelper::addIgnoreObject(QQuickItem *val) {
    const auto win = window();
    if (win && val) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->ignoreObjects.append(val);
            }
        }
#else
        m_framelessHelper.addIgnoreObject(win, val);
#endif
    }
}

void FramelessQuickHelper::setDraggableObjects(
    const QVector<QQuickItem *> &val) {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableObjects.clear();
                if (!val.isEmpty()) {
                    for (auto &&obj : qAsConst(val)) {
                        data->draggableObjects.append(obj);
                    }
                }
            }
        }
#else
        QVector<QObject *> objs{};
        if (!val.isEmpty()) {
            for (auto &&obj : qAsConst(val)) {
                objs.append(obj);
            }
            m_framelessHelper.setDraggableObjects(win, objs);
        }
#endif
    }
}

void FramelessQuickHelper::clearDraggableObjects() {
    const auto win = window();
    if (win) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableObjects.clear();
            }
        }
#else
        m_framelessHelper.clearDraggableObjects(win);
#endif
    }
}

void FramelessQuickHelper::addDraggableObject(QQuickItem *val) {
    const auto win = window();
    if (win && val) {
#ifdef Q_OS_WINDOWS
        const auto hWnd = reinterpret_cast<HWND>(win->winId());
        if (hWnd) {
            const auto data = WinNativeEventFilter::windowData(hWnd);
            if (data) {
                data->draggableObjects.append(val);
            }
        }
#else
        m_framelessHelper.addDraggableObject(win, val);
#endif
    }
}
