/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "framelesshelper_global.h"
#include <QtQuick/qquickitem.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_API FramelessQuickHelper : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessQuickHelper)
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessHelper)
#endif
    Q_PROPERTY(qreal resizeBorderThickness READ resizeBorderThickness WRITE setResizeBorderThickness NOTIFY resizeBorderThicknessChanged)
    Q_PROPERTY(qreal titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)

public:
    explicit FramelessQuickHelper(QQuickItem *parent = nullptr);
    ~FramelessQuickHelper() override = default;

    Q_NODISCARD qreal resizeBorderThickness() const;
    void setResizeBorderThickness(const qreal val);

    Q_NODISCARD qreal titleBarHeight() const;
    void setTitleBarHeight(const qreal val);

    Q_NODISCARD bool resizable() const;
    void setResizable(const bool val);

    Q_NODISCARD Q_INVOKABLE bool isWindowFrameless() const;

public Q_SLOTS:
    void removeWindowFrame();
    void bringBackWindowFrame();
    void setHitTestVisible(QQuickItem *item, const bool visible);
    void showMinimized();

Q_SIGNALS:
    void resizeBorderThicknessChanged(qreal);
    void titleBarHeightChanged(qreal);
    void resizableChanged(bool);
};

FRAMELESSHELPER_END_NAMESPACE
