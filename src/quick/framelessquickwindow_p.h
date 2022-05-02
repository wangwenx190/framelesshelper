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
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickanchors_p_p.h>

QT_BEGIN_NAMESPACE
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
    explicit FramelessQuickWindowPrivate(FramelessQuickWindow *q, const Global::UserSettings &settings = {});
    ~FramelessQuickWindowPrivate() override;

    Q_NODISCARD static FramelessQuickWindowPrivate *get(FramelessQuickWindow *pub);
    Q_NODISCARD static const FramelessQuickWindowPrivate *get(const FramelessQuickWindow *pub);

    Q_INVOKABLE Q_NODISCARD bool isHidden() const;
    Q_INVOKABLE Q_NODISCARD bool isNormal() const;
    Q_INVOKABLE Q_NODISCARD bool isMinimized() const;
    Q_INVOKABLE Q_NODISCARD bool isZoomed() const;
    Q_INVOKABLE Q_NODISCARD bool isFullScreen() const;

    Q_INVOKABLE Q_NODISCARD QColor getFrameBorderColor() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderTop() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderBottom() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderLeft() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderRight() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderHorizontalCenter() const;
    Q_INVOKABLE Q_NODISCARD QQuickAnchorLine getTopBorderVerticalCenter() const;

public Q_SLOTS:
    void showMinimized2();
    void toggleMaximized();
    void toggleFullScreen();
    void snapToTopBorder(QQuickItem *item, const QuickGlobal::Anchor itemAnchor, const QuickGlobal::Anchor topBorderAnchor);

private:
    void initialize();
    Q_NODISCARD bool shouldDrawFrameBorder() const;

private Q_SLOTS:
    void updateTopBorderColor();
    void updateTopBorderHeight();

private:
    FramelessQuickWindow *q_ptr = nullptr;
    QScopedPointer<QQuickRectangle> m_topBorderRectangle;
    QScopedPointer<QQuickAnchors> m_topBorderAnchors;
    QQuickWindow::Visibility m_savedVisibility = QQuickWindow::Windowed;
    Global::UserSettings m_settings = {};
    bool m_windowExposed = false;
};

FRAMELESSHELPER_END_NAMESPACE