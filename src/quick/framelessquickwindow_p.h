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
#include <QtCore/qobject.h>
#include <QtGui/qwindow.h>

QT_BEGIN_NAMESPACE
class QQuickItem;
class QQuickRectangle;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessQuickWindow;

class FRAMELESSHELPER_QUICK_API FramelessQuickWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessQuickWindow)
    Q_DISABLE_COPY_MOVE(FramelessQuickWindowPrivate)

public:
    explicit FramelessQuickWindowPrivate(FramelessQuickWindow *q, const Options options);
    ~FramelessQuickWindowPrivate() override;

    Q_INVOKABLE Q_NODISCARD bool isZoomed() const;
    Q_INVOKABLE Q_NODISCARD bool isFixedSize() const;
    Q_INVOKABLE Q_NODISCARD QColor getFrameBorderColor() const;
    Q_INVOKABLE Q_NODISCARD bool isFrameBorderVisible() const;

public Q_SLOTS:
    void showMinimized2();
    void toggleMaximize();
    void toggleFullScreen();
    void showSystemMenu(const QPoint &pos);
    void startSystemMove2();
    void startSystemResize2(const Qt::Edges edges);
    void setTitleBarItem(QQuickItem *item);
    void setHitTestVisible(QQuickItem *item);
    void moveToDesktopCenter();
    void setFixedSize(const bool value);

private:
    void initialize();

private Q_SLOTS:
    void updateTopBorderColor();
    void updateTopBorderHeight();

private:
    FramelessQuickWindow *q_ptr = nullptr;
    bool m_initialized = false;
    QScopedPointer<QQuickRectangle> m_topBorderRectangle;
    QWindow::Visibility m_savedVisibility = QWindow::Windowed;
    Options m_options = {};
};

FRAMELESSHELPER_END_NAMESPACE
