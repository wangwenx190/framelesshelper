/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE

#include <FramelessHelper/Quick/framelesshelperquick_global.h>
#include <QtQuick/qquickwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessQuickApplicationWindow;
class QuickWindowBorder;

class FRAMELESSHELPER_QUICK_API FramelessQuickApplicationWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessQuickApplicationWindow)
    Q_DISABLE_COPY_MOVE(FramelessQuickApplicationWindowPrivate)

public:
    explicit FramelessQuickApplicationWindowPrivate(FramelessQuickApplicationWindow *q);
    ~FramelessQuickApplicationWindowPrivate() override;

    Q_NODISCARD static FramelessQuickApplicationWindowPrivate *get(FramelessQuickApplicationWindow *pub);
    Q_NODISCARD static const FramelessQuickApplicationWindowPrivate *get(const FramelessQuickApplicationWindow *pub);

    Q_INVOKABLE Q_NODISCARD bool isHidden() const;
    Q_INVOKABLE Q_NODISCARD bool isNormal() const;
    Q_INVOKABLE Q_NODISCARD bool isMinimized() const;
    Q_INVOKABLE Q_NODISCARD bool isMaximized() const;
    Q_INVOKABLE Q_NODISCARD bool isZoomed() const;
    Q_INVOKABLE Q_NODISCARD bool isFullScreen() const;

public Q_SLOTS:
    void showMinimized2();
    void toggleMaximized();
    void toggleFullScreen();

private:
    void initialize();

private:
    FramelessQuickApplicationWindow *q_ptr = nullptr;
    QuickWindowBorder *m_windowBorder = nullptr;
    QQuickWindow::Visibility m_savedVisibility = QQuickWindow::Windowed;
};

FRAMELESSHELPER_END_NAMESPACE

#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE
