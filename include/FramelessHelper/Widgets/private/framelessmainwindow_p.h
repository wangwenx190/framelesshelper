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

#include "framelesshelperwidgets_global.h"
#include "framelessmainwindow.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

class WidgetsSharedHelper;

class FRAMELESSHELPER_WIDGETS_API FramelessMainWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessMainWindow)
    Q_DISABLE_COPY_MOVE(FramelessMainWindowPrivate)

public:
    explicit FramelessMainWindowPrivate(FramelessMainWindow *q);
    ~FramelessMainWindowPrivate() override;

    Q_NODISCARD static FramelessMainWindowPrivate *get(FramelessMainWindow *pub);
    Q_NODISCARD static const FramelessMainWindowPrivate *get(const FramelessMainWindow *pub);

    Q_NODISCARD bool isNormal() const;
    Q_NODISCARD bool isZoomed() const;

    void toggleMaximized();
    void toggleFullScreen();

    Q_NODISCARD WidgetsSharedHelper *widgetsSharedHelper() const;

private:
    void initialize();

private:
    QPointer<FramelessMainWindow> q_ptr = nullptr;
    Qt::WindowState m_savedWindowState = Qt::WindowNoState;
    QScopedPointer<WidgetsSharedHelper> m_helper;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessMainWindowPrivate))
