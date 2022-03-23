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
#include <QtCore/qobject.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QVBoxLayout;
class QShowEvent;
class QPaintEvent;
class QMouseEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_WIDGETS_API FramelessWidgetsHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessWidgetsHelper)

public:
    explicit FramelessWidgetsHelper(QWidget *q, const Options options = {});
    ~FramelessWidgetsHelper() override;

    Q_NODISCARD Q_INVOKABLE bool isNormal() const;
    Q_NODISCARD Q_INVOKABLE bool isZoomed() const;

    Q_INVOKABLE void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD Q_INVOKABLE QWidget *titleBarWidget() const;

    Q_INVOKABLE void setContentWidget(QWidget *widget);
    Q_NODISCARD Q_INVOKABLE QWidget *contentWidget() const;

    Q_INVOKABLE void setHitTestVisible(QWidget *widget);

    Q_INVOKABLE void toggleMaximized();
    Q_INVOKABLE void toggleFullScreen();

    Q_INVOKABLE void changeEventHandler(QEvent *event);
    Q_INVOKABLE void paintEventHandler(QPaintEvent *event);
    Q_INVOKABLE void mousePressEventHandler(QMouseEvent *event);
    Q_INVOKABLE void mouseReleaseEventHandler(QMouseEvent *event);
    Q_INVOKABLE void mouseDoubleClickEventHandler(QMouseEvent *event);

private:
    void initialize();
    void createSystemTitleBar();
    void createUserContentContainer();
    void setupInitialUi();
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldDrawFrameBorder() const;
    Q_NODISCARD bool shouldIgnoreMouseEvents(const QPoint &pos) const;

private Q_SLOTS:
    void updateContentsMargins();
    void updateSystemTitleBarStyleSheet();
    void updateSystemButtonsIcon();

private:
    QWidget *q = nullptr;
    bool m_initialized = false;
    Options m_options = {};
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
    Qt::WindowStates m_savedWindowState = {};
    QWindow *m_window = nullptr;
};

FRAMELESSHELPER_END_NAMESPACE
