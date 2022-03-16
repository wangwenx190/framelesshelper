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

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QVBoxLayout;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_WIDGETS_API FramelessWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessWidget)
    Q_PROPERTY(QWidget* titleBarWidget READ titleBarWidget WRITE setTitleBarWidget NOTIFY titleBarWidgetChanged FINAL)
    Q_PROPERTY(QWidget* contentWidget READ contentWidget WRITE setContentWidget NOTIFY contentWidgetChanged FINAL)

public:
    explicit FramelessWidget(QWidget *parent = nullptr);
    ~FramelessWidget() override;

    Q_NODISCARD bool isNormal() const;
    Q_NODISCARD bool isZoomed() const;

    void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD QWidget *titleBarWidget() const;

    void setContentWidget(QWidget *widget);
    Q_NODISCARD QWidget *contentWidget() const;

    Q_INVOKABLE void setHitTestVisible(QWidget *widget, const bool visible);

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void setupFramelessHelperOnce();
    void createSystemTitleBar();
    void createUserContentContainer();
    void setupInitialUi();
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldDrawFrameBorder() const;

private Q_SLOTS:
    void updateContentsMargins();
    void updateSystemTitleBarStyleSheet();
    void updateSystemButtonsIcon();

Q_SIGNALS:
    void titleBarWidgetChanged();
    void contentWidgetChanged();
    void systemThemeChanged();
    void systemMenuRequested(const QPointF &);

private:
    bool m_framelessHelperInited = false;
    QWidget *m_systemTitleBarWidget = nullptr;
    QLabel *m_systemWindowTitleLabel = nullptr;
    QPushButton *m_systemMinimizeButton = nullptr;
    QPushButton *m_systemMaximizeButton = nullptr;
    QPushButton *m_systemCloseButton = nullptr;
    QWidget *m_userTitleBarWidget = nullptr;
    QWidget *m_userContentWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QWidgetList m_hitTestVisibleWidgets = {};
    QWidget *m_userContentContainerWidget = nullptr;
    QVBoxLayout *m_userContentContainerLayout = nullptr;
};

FRAMELESSHELPER_END_NAMESPACE
