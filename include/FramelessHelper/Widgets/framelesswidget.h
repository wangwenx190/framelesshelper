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
#include <QtWidgets/qwidget.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessWidgetsHelper;

class FRAMELESSHELPER_WIDGETS_API FramelessWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessWidget)
    Q_PROPERTY(bool hidden READ isHidden NOTIFY hiddenChanged FINAL)
    Q_PROPERTY(bool normal READ isNormal NOTIFY normalChanged FINAL)
    Q_PROPERTY(bool zoomed READ isZoomed NOTIFY zoomedChanged FINAL)
    Q_PROPERTY(bool fixedSize READ isFixedSize WRITE setFixedSize NOTIFY fixedSizeChanged FINAL)
    Q_PROPERTY(QWidget* titleBarWidget READ titleBarWidget WRITE setTitleBarWidget NOTIFY titleBarWidgetChanged FINAL)
    Q_PROPERTY(QWidget* contentWidget READ contentWidget WRITE setContentWidget NOTIFY contentWidgetChanged FINAL)

public:
    explicit FramelessWidget(QWidget *parent = nullptr, const Global::UserSettings &settings = {});
    ~FramelessWidget() override;

    Q_NODISCARD bool isNormal() const;
    Q_NODISCARD bool isZoomed() const;

    Q_NODISCARD bool isFixedSize() const;
    void setFixedSize(const bool value);

    void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD QWidget *titleBarWidget() const;

    void setContentWidget(QWidget *widget);
    Q_NODISCARD QWidget *contentWidget() const;

public Q_SLOTS:
    void setHitTestVisible(QWidget *widget);
    void toggleMaximize();
    void toggleFullScreen();
    void moveToDesktopCenter();
    void bringToFront();
    void showSystemMenu(const QPoint &pos);
    void startSystemMove2();
    void startSystemResize2(const Qt::Edges edges);

Q_SIGNALS:
    void hiddenChanged();
    void normalChanged();
    void zoomedChanged();
    void fixedSizeChanged();
    void titleBarWidgetChanged();
    void contentWidgetChanged();
    void systemThemeChanged();

private:
    QScopedPointer<FramelessWidgetsHelper> m_helper;
};

FRAMELESSHELPER_END_NAMESPACE
