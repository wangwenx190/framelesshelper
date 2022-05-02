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
#include <QtGui/qwindow.h>

QT_BEGIN_NAMESPACE
class QLabel;
class QVBoxLayout;
class QShowEvent;
class QPaintEvent;
class QMouseEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class StandardSystemButton;

class FRAMELESSHELPER_WIDGETS_API FramelessWidgetsHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessWidgetsHelper)

public:
    explicit FramelessWidgetsHelper(QWidget *q, const Global::UserSettings &settings = {});
    ~FramelessWidgetsHelper() override;

    Q_NODISCARD static FramelessWidgetsHelper *get(QWidget *pub);
    Q_NODISCARD static const FramelessWidgetsHelper *get(const QWidget *pub);

    Q_NODISCARD Q_INVOKABLE bool isNormal() const;
    Q_NODISCARD Q_INVOKABLE bool isZoomed() const;
    Q_NODISCARD Q_INVOKABLE bool isFixedSize() const;
    Q_INVOKABLE void setFixedSize(const bool value, const bool force = false);

    Q_INVOKABLE void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD Q_INVOKABLE QWidget *getTitleBarWidget() const;

    Q_INVOKABLE void setContentWidget(QWidget *widget);
    Q_NODISCARD Q_INVOKABLE QWidget *getContentWidget() const;

    Q_INVOKABLE void showEventHandler(QShowEvent *event);
    Q_INVOKABLE void changeEventHandler(QEvent *event);
    Q_INVOKABLE void paintEventHandler(QPaintEvent *event);

public Q_SLOTS:
    void setHitTestVisible(QWidget *widget);
    void toggleMaximized();
    void toggleFullScreen();
    void moveToDesktopCenter();
    void bringToFront();
    void showSystemMenu(const QPoint &pos);
    void startSystemMove2(const QPoint &pos);
    void startSystemResize2(const Qt::Edges edges, const QPoint &pos);
    void setSystemButton(QWidget *widget, const Global::SystemButtonType buttonType);

protected:
    Q_NODISCARD bool eventFilter(QObject *object, QEvent *event) override;

private:
    void initialize();
    void createSystemTitleBar();
    void createUserContentContainer();
    void setupInitialUi();
    Q_NODISCARD QRect mapWidgetGeometryToScene(const QWidget * const widget) const;
    Q_NODISCARD bool isInSystemButtons(const QPoint &pos, Global::SystemButtonType *button) const;
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldDrawFrameBorder() const;
    Q_NODISCARD bool shouldIgnoreMouseEvents(const QPoint &pos) const;
    void setSystemButtonState(const Global::SystemButtonType button, const Global::ButtonState state);

private Q_SLOTS:
    void updateContentsMargins();
    void updateSystemTitleBarStyleSheet();
    void updateSystemMaximizeButton();

private:
    QPointer<QWidget> q = nullptr;
    QScopedPointer<QWidget> m_systemTitleBarWidget;
    QScopedPointer<QLabel> m_systemWindowTitleLabel;
    QPointer<QWidget> m_userTitleBarWidget = nullptr;
    QPointer<QWidget> m_userContentWidget = nullptr;
    QScopedPointer<QVBoxLayout> m_mainLayout;
    QWidgetList m_hitTestVisibleWidgets = {};
    QScopedPointer<QWidget> m_userContentContainerWidget;
    QScopedPointer<QVBoxLayout> m_userContentContainerLayout;
    Qt::WindowState m_savedWindowState = {};
    Global::UserSettings m_settings = {};
    Global::SystemParameters m_params = {};
    bool m_windowExposed = false;
};

FRAMELESSHELPER_END_NAMESPACE