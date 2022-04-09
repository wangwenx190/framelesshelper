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
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessQuickWindowPrivate;

class FRAMELESSHELPER_QUICK_API FramelessQuickWindow : public QQuickWindow
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessWindow)
#endif
    Q_DECLARE_PRIVATE(FramelessQuickWindow)
    Q_DISABLE_COPY_MOVE(FramelessQuickWindow)
    Q_PROPERTY(bool hidden READ isHidden NOTIFY hiddenChanged FINAL)
    Q_PROPERTY(bool normal READ isNormal NOTIFY normalChanged FINAL)
    Q_PROPERTY(bool minimized READ isMinimized NOTIFY minimizedChanged FINAL)
    Q_PROPERTY(bool zoomed READ isZoomed NOTIFY zoomedChanged FINAL)
    Q_PROPERTY(bool fullScreen READ isFullScreen NOTIFY fullScreenChanged FINAL)
    Q_PROPERTY(bool fixedSize READ fixedSize WRITE setFixedSize NOTIFY fixedSizeChanged FINAL)
    Q_PROPERTY(QColor frameBorderColor READ frameBorderColor NOTIFY frameBorderColorChanged FINAL)

public:
    explicit FramelessQuickWindow(QWindow *parent = nullptr, const Global::UserSettings &settings = {});
    ~FramelessQuickWindow() override;

    Q_NODISCARD bool isHidden() const;
    Q_NODISCARD bool isNormal() const;
    Q_NODISCARD bool isMinimized() const;
    Q_NODISCARD bool isZoomed() const;
    Q_NODISCARD bool isFullScreen() const;

    Q_NODISCARD bool fixedSize() const;
    void setFixedSize(const bool value);

    Q_NODISCARD QColor frameBorderColor() const;

public Q_SLOTS:
    void showMinimized2();
    void toggleMaximized();
    void toggleFullScreen();
    void showSystemMenu(const QPoint &pos);
    void startSystemMove2();
    void startSystemResize2(const Qt::Edges edges);
    void setTitleBarItem(QQuickItem *item);
    void setHitTestVisible(QQuickItem *item);
    void moveToDesktopCenter();
    void bringToFront();
    void snapToTopBorder(QQuickItem *item, const Global::Anchor itemAnchor, const Global::Anchor topBorderAnchor);

Q_SIGNALS:
    void hiddenChanged();
    void normalChanged();
    void minimizedChanged();
    void zoomedChanged();
    void fullScreenChanged();
    void fixedSizeChanged();
    void frameBorderColorChanged();
    void systemButtonStateChanged(const Global::SystemButtonType, const Global::ButtonState);

private:
    QScopedPointer<FramelessQuickWindowPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE
