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

class FRAMELESSHELPER_EXPORT FramelessQuickHelper : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessQuickHelper)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QML_NAMED_ELEMENT(FramelessHelper)
#endif
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int borderHeight READ borderHeight WRITE setBorderHeight NOTIFY borderHeightChanged)
    Q_PROPERTY(int titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(QColor nativeFrameColor READ nativeFrameColor NOTIFY nativeFrameColorChanged)

public:
    explicit FramelessQuickHelper(QQuickItem *parent = nullptr);
    ~FramelessQuickHelper() override = default;

    int borderWidth() const;
    void setBorderWidth(const int val);

    int borderHeight() const;
    void setBorderHeight(const int val);

    int titleBarHeight() const;
    void setTitleBarHeight(const int val);

    bool resizable() const;
    void setResizable(const bool val);

    QColor nativeFrameColor() const;

public Q_SLOTS:
    void removeWindowFrame();
    void addIgnoreObject(QQuickItem *val);

Q_SIGNALS:
    void borderWidthChanged();
    void borderHeightChanged();
    void titleBarHeightChanged();
    void resizableChanged();
    void nativeFrameColorChanged();

private:
    QMetaObject::Connection m_frameColorConnection = {};
};
