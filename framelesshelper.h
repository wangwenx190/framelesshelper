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

#include <QHash>
#include <QObject>
#include <QRect>
#include <QVector>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FramelessHelper : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessHelper)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY
                   borderWidthChanged)
    Q_PROPERTY(int borderHeight READ borderHeight WRITE setBorderHeight NOTIFY
                   borderHeightChanged)
    Q_PROPERTY(int titlebarHeight READ titlebarHeight WRITE setTitlebarHeight
                   NOTIFY titlebarHeightChanged)
    Q_PROPERTY(Areas ignoreAreas READ ignoreAreas WRITE setIgnoreAreas NOTIFY
                   ignoreAreasChanged)
    Q_PROPERTY(Areas draggableAreas READ draggableAreas WRITE setDraggableAreas
                   NOTIFY draggableAreasChanged)
    Q_PROPERTY(QVector<QObject *> framelessWindows READ framelessWindows WRITE
                   setFramelessWindows NOTIFY framelessWindowsChanged)

public:
    using Areas = QHash<QObject *, QVector<QRect>>;

    explicit FramelessHelper(QObject *parent = nullptr);
    ~FramelessHelper() override = default;

    static void updateQtFrame(QWindow *window, int titlebarHeight);

    int borderWidth() const;
    void setBorderWidth(int val);

    int borderHeight() const;
    void setBorderHeight(int val);

    int titlebarHeight() const;
    void setTitlebarHeight(int val);

    Areas ignoreAreas() const;
    void setIgnoreAreas(const Areas &val);

    Areas draggableAreas() const;
    void setDraggableAreas(const Areas &val);

    QVector<QObject *> framelessWindows() const;
    void setFramelessWindows(const QVector<QObject *> &val);

protected:
#ifndef Q_OS_WINDOWS
    bool eventFilter(QObject *object, QEvent *event) override;
#endif

private:
    QWindow *getWindowHandle(QObject *val);
#ifdef Q_OS_WINDOWS
    void *getWindowRawHandle(QObject *object);
#endif
    void updateQtFrame_internal(int val);

Q_SIGNALS:
    void borderWidthChanged(int);
    void borderHeightChanged(int);
    void titlebarHeightChanged(int);
    void ignoreAreasChanged(const Areas &);
    void draggableAreasChanged(const Areas &);
    void framelessWindowsChanged(const QVector<QObject *> &);

private:
    int m_borderWidth = -1, m_borderHeight = -1, m_titlebarHeight = -1;
    Areas m_ignoreAreas, m_draggableAreas;
    QVector<QObject *> m_framelessWindows;
};
