/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QHash>
#include <QObject>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FRAMELESSHELPER_EXPORT FramelessHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessHelper)

public:
    explicit FramelessHelper(QObject *parent = nullptr);
    ~FramelessHelper() override = default;

    void removeWindowFrame(QWindow *window);

    int getBorderWidth() const;
    void setBorderWidth(const int val);

    int getBorderHeight() const;
    void setBorderHeight(const int val);

    int getTitleBarHeight() const;
    void setTitleBarHeight(const int val);

    void addIgnoreObject(const QWindow *window, QObject *val);
    QObjectList getIgnoreObjects(const QWindow *window) const;

    bool getResizable(const QWindow *window) const;
    void setResizable(const QWindow *window, const bool val);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    // ### FIXME: The default border width and height on Windows is 8 pixels if
    // the scale factor is 1.0. Don't know how to acquire these values on UNIX
    // platforms through native API.
    int m_borderWidth = 8, m_borderHeight = 8, m_titleBarHeight = 30;
    QHash<const QWindow *, QObjectList> m_ignoreObjects = {};
    QHash<const QWindow *, bool> m_fixedSize = {};
};
#endif
