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
QT_FORWARD_DECLARE_CLASS(QWindow)
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
    explicit FramelessWindowsManager();
    ~FramelessWindowsManager() = default;

    static void addWindow(const QWindow *window, const bool center = false);

    static void moveWindowToDesktopCenter(const QWindow *window);

    static void addIgnoreArea(const QWindow *window, const QRect &area);
    static void addDraggableArea(const QWindow *window, const QRect &area);

    static void addIgnoreObject(const QWindow *window, QObject *object);
    static void addDraggableObject(const QWindow *window, QObject *object);

    static int getBorderWidth(const QWindow *window);
    static void setBorderWidth(const QWindow *window, const int value);

    static int getBorderHeight(const QWindow *window);
    static void setBorderHeight(const QWindow *window, const int value);

    static int getTitleBarHeight(const QWindow *window);
    static void setTitleBarHeight(const QWindow *window, const int value);

    static bool getResizable(const QWindow *window);
    static void setResizable(const QWindow *window, const bool value = true);

    static QSize getMinimumSize(const QWindow *window);
    static void setMinimumSize(const QWindow *window, const QSize &value);

    static QSize getMaximumSize(const QWindow *window);
    static void setMaximumSize(const QWindow *window, const QSize &value);

    static bool getTitleBarEnabled(const QWindow *window);
    static void setTitleBarEnabled(const QWindow *window, const bool value = true);
};
