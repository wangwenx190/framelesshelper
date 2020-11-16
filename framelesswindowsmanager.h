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

#pragma once

#include "framelesshelper_global.h"
#include <QRect>

#if (defined(Q_OS_WIN) || defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_WINRT)) \
    && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QObject)
QT_END_NAMESPACE

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

class FRAMELESSHELPER_EXPORT FramelessWindowsManager
{
    Q_DISABLE_COPY_MOVE(FramelessWindowsManager)

public:
    using WindowId =
#ifdef Q_OS_WINDOWS
        void *
#else
        QObject *
#endif
        ;

    explicit FramelessWindowsManager();
    ~FramelessWindowsManager() = default;

    static void addWindow(WindowId window, const bool center = false);

    static void moveWindowToDesktopCenter(WindowId window);

    static void addIgnoreArea(WindowId window, const QRect &area);
    static void addDraggableArea(WindowId window, const QRect &area);

    static void addIgnoreObject(WindowId window, QObject *object);
    static void addDraggableObject(WindowId window, QObject *object);

    static int getBorderWidth(WindowId window);
    static void setBorderWidth(WindowId window, const int value);

    static int getBorderHeight(WindowId window);
    static void setBorderHeight(WindowId window, const int value);

    static int getTitleBarHeight(WindowId window);
    static void setTitleBarHeight(WindowId window, const int value);

    static bool getResizable(WindowId window);
    static void setResizable(WindowId window, const bool value = true);

    static QSize getMinimumSize(WindowId window);
    static void setMinimumSize(WindowId window, const QSize &value);

    static QSize getMaximumSize(WindowId window);
    static void setMaximumSize(WindowId window, const QSize &value);

    static bool getTitleBarEnabled(WindowId window);
    static void setTitleBarEnabled(WindowId window, const bool value = true);
};
