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

#pragma once

#include "framelesshelperquick_global.h"
#include "framelessquickwindow_p.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

class QuickWindowBorder;

class FRAMELESSHELPER_QUICK_API FramelessQuickWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessQuickWindow)
    Q_DISABLE_COPY_MOVE(FramelessQuickWindowPrivate)

public:
    explicit FramelessQuickWindowPrivate(FramelessQuickWindow *q);
    ~FramelessQuickWindowPrivate() override;

    Q_NODISCARD static FramelessQuickWindowPrivate *get(FramelessQuickWindow *pub);
    Q_NODISCARD static const FramelessQuickWindowPrivate *get(const FramelessQuickWindow *pub);

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
    QPointer<FramelessQuickWindow> q_ptr = nullptr;
    QScopedPointer<QuickWindowBorder> m_windowBorder;
    QQuickWindow::Visibility m_savedVisibility = QQuickWindow::Windowed;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessQuickWindowPrivate))
