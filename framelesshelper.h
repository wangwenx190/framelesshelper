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

#include <QMap>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QVector>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FramelessHelper : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessHelper)

public:
    explicit FramelessHelper(QObject *parent = nullptr);
    ~FramelessHelper() override = default;

    static void updateQtFrame(QWindow *const window, const int titleBarHeight);

    int getBorderWidth() const;
    void setBorderWidth(const int val);

    int getBorderHeight() const;
    void setBorderHeight(const int val);

    int getTitleBarHeight() const;
    void setTitleBarHeight(const int val);

    QVector<QRect> getIgnoreAreas(QObject *const obj) const;
    void setIgnoreAreas(QObject *const obj, const QVector<QRect> &val);

    QVector<QRect> getDraggableAreas(QObject *const obj) const;
    void setDraggableAreas(QObject *const obj, const QVector<QRect> &val);

    QVector<QPointer<QObject>> getIgnoreObjects(QObject *const obj) const;
    void setIgnoreObjects(QObject *const obj,
                          const QVector<QPointer<QObject>> &val);

    QVector<QPointer<QObject>> getDraggableObjects(QObject *const obj) const;
    void setDraggableObjects(QObject *const obj,
                             const QVector<QPointer<QObject>> &val);

    void removeWindowFrame(QObject *const obj);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    using Areas = QMap<QPointer<QObject>, QVector<QRect>>;
    using Objects = QMap<QPointer<QObject>, QVector<QPointer<QObject>>>;

    int m_borderWidth = -1, m_borderHeight = -1, m_titleBarHeight = -1;
    Areas m_ignoreAreas = {}, m_draggableAreas = {};
    Objects m_ignoreObjects = {}, m_draggableObjects = {};
};
