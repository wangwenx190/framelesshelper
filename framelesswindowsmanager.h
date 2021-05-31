/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FRAMELESSHELPER_API FramelessWindowsManager
{
    Q_DISABLE_COPY_MOVE(FramelessWindowsManager)

public:
    explicit FramelessWindowsManager();
    ~FramelessWindowsManager() = default;

    static void addWindow(QWindow *window);

    static void setHitTestVisible(QWindow *window, QObject *object, const bool value = true);

    static int getBorderWidth(const QWindow *window);
    static void setBorderWidth(QWindow *window, const int value);

    static int getBorderHeight(const QWindow *window);
    static void setBorderHeight(QWindow *window, const int value);

    static int getTitleBarHeight(const QWindow *window);
    static void setTitleBarHeight(QWindow *window, const int value);

    static bool getResizable(const QWindow *window);
    static void setResizable(QWindow *window, const bool value = true);
};
