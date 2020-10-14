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
    explicit FramelessWindowsManager();
    ~FramelessWindowsManager() = default;

    static void addWindow(QObject *window, const bool center = false);

    static void moveWindowToDesktopCenter(QObject *window, const bool realCenter = true);

    static QSize getDesktopSize(QObject *window = nullptr);
    static QRect getDesktopAvailableGeometry(QObject *window = nullptr);
    static QSize getDesktopAvailableSize(QObject *window = nullptr);

    static void addIgnoreArea(QObject *window, const QRect &area);
    static void addDraggableArea(QObject *window, const QRect &area);

    static void addIgnoreObject(QObject *window, QObject *object);
    static void addDraggableObject(QObject *window, QObject *object);

    static int getBorderWidth(QObject *window);
    static void setBorderWidth(QObject *window, const int value);

    static int getBorderHeight(QObject *window);
    static void setBorderHeight(QObject *window, const int value);

    static int getTitleBarHeight(QObject *window);
    static void setTitleBarHeight(QObject *window, const int value);

    static bool getResizable(QObject *window);
    static void setResizable(QObject *window, const bool value = true);

    static QSize getMinimumSize(QObject *window);
    static void setMinimumSize(QObject *window, const QSize &value);

    static QSize getMaximumSize(QObject *window);
    static void setMaximumSize(QObject *window, const QSize &value);

    static bool getTitleBarEnabled(QObject *window);
    static void setTitleBarEnabled(QObject *window, const bool value = true);
};
