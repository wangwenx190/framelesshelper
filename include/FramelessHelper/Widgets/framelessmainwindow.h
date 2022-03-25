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
#include <QtWidgets/qmainwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessWidgetsHelper;

class FRAMELESSHELPER_WIDGETS_API FramelessMainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessMainWindow)
    Q_PROPERTY(bool zoomed READ isZoomed NOTIFY zoomedChanged FINAL)
    Q_PROPERTY(bool fixedSize READ isFixedSize WRITE setFixedSize NOTIFY fixedSizeChanged FINAL)
    Q_PROPERTY(QWidget* titleBarWidget READ titleBarWidget WRITE setTitleBarWidget NOTIFY titleBarWidgetChanged FINAL)

public:
    explicit FramelessMainWindow(QWidget *parent = nullptr, const Qt::WindowFlags flags = {}, const Global::Options options = {});
    ~FramelessMainWindow() override;

    Q_NODISCARD Q_INVOKABLE bool isNormal() const;

    Q_NODISCARD bool isZoomed() const;

    Q_NODISCARD bool isFixedSize() const;
    void setFixedSize(const bool value);

    void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD QWidget *titleBarWidget() const;

public Q_SLOTS:
    void setHitTestVisible(QWidget *widget);
    void toggleMaximized();
    void toggleFullScreen();
    void moveToDesktopCenter();

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void zoomedChanged();
    void fixedSizeChanged();
    void titleBarWidgetChanged();
    void systemThemeChanged();

private:
    QScopedPointer<FramelessWidgetsHelper> m_helper;
};

FRAMELESSHELPER_END_NAMESPACE
