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

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessWidgetsHelperPrivate;

class FRAMELESSHELPER_WIDGETS_API FramelessWidgetsHelper : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FramelessWidgetsHelper)
    Q_DISABLE_COPY_MOVE(FramelessWidgetsHelper)
    Q_PROPERTY(QWidget* titleBarWidget READ titleBarWidget WRITE setTitleBarWidget NOTIFY titleBarWidgetChanged FINAL)
    Q_PROPERTY(bool windowFixedSize READ isWindowFixedSize WRITE setWindowFixedSize NOTIFY windowFixedSizeChanged FINAL)
    Q_PROPERTY(bool blurBehindWindowEnabled READ isBlurBehindWindowEnabled WRITE setBlurBehindWindowEnabled NOTIFY blurBehindWindowEnabledChanged FINAL)

public:
    explicit FramelessWidgetsHelper(QObject *parent = nullptr);
    ~FramelessWidgetsHelper() override;

    Q_NODISCARD static FramelessWidgetsHelper *get(QObject *object);

    Q_NODISCARD QWidget *titleBarWidget() const;
    Q_NODISCARD bool isWindowFixedSize() const;
    Q_NODISCARD bool isBlurBehindWindowEnabled() const;

public Q_SLOTS:
    void extendsContentIntoTitleBar();

    void setTitleBarWidget(QWidget *widget);
    void setSystemButton(QWidget *widget, const Global::SystemButtonType buttonType);
    void setHitTestVisible(QWidget *widget, const bool visible = true);

    void showSystemMenu(const QPoint &pos);
    void windowStartSystemMove2(const QPoint &pos);
    void windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos);

    void moveWindowToDesktopCenter();
    void bringWindowToFront();
    void setWindowFixedSize(const bool value);
    void setBlurBehindWindowEnabled(const bool value);

Q_SIGNALS:
    void titleBarWidgetChanged();
    void windowFixedSizeChanged();
    void blurBehindWindowEnabledChanged();
    void ready();

private:
    QScopedPointer<FramelessWidgetsHelperPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE
