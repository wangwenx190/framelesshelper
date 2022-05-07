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
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE
class QPaintEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_WIDGETS_API WidgetsSharedHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WidgetsSharedHelper)

public:
    explicit WidgetsSharedHelper(QObject *parent = nullptr);
    ~WidgetsSharedHelper() override;

    void setup(QWidget *widget);

protected:
    Q_NODISCARD bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void updateContentsMargins();

private:
    void changeEventHandler(QEvent *event);
    void paintEventHandler(QPaintEvent *event);
    Q_NODISCARD bool shouldDrawFrameBorder() const;

private:
    QPointer<QWidget> m_targetWidget = nullptr;
};

FRAMELESSHELPER_END_NAMESPACE
