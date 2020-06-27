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

#include <QQuickItem>

#if (defined(Q_OS_WIN) || defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_WINRT)) \
    && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

#ifndef Q_OS_WINDOWS
#include "framelesshelper.h"
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

class FramelessQuickHelper : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessQuickHelper)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QML_ELEMENT
#endif
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int borderHeight READ borderHeight WRITE setBorderHeight NOTIFY borderHeightChanged)
    Q_PROPERTY(
        int titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(QSize minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    Q_PROPERTY(QSize maximumSize READ maximumSize WRITE setMaximumSize NOTIFY maximumSizeChanged)
    Q_PROPERTY(bool titleBarEnabled READ titleBarEnabled WRITE setTitleBarEnabled NOTIFY
                   titleBarEnabledChanged)

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

    QSize minimumSize() const;
    void setMinimumSize(const QSize &val);

    QSize maximumSize() const;
    void setMaximumSize(const QSize &val);

    bool titleBarEnabled() const;
    void setTitleBarEnabled(const bool val);

public Q_SLOTS:
    void removeWindowFrame(const bool center = true);
    void moveWindowToDesktopCenter(const bool realCenter = true);
    QSize desktopSize() const;
    QRect desktopAvailableGeometry() const;
    QSize desktopAvailableSize() const;

    void setIgnoreAreas(const QList<QRect> &val);
    void clearIgnoreAreas();
    void addIgnoreArea(const QRect &val);

    void setDraggableAreas(const QList<QRect> &val);
    void clearDraggableAreas();
    void addDraggableArea(const QRect &val);

    void setIgnoreObjects(const QList<QQuickItem *> &val);
    void clearIgnoreObjects();
    void addIgnoreObject(QQuickItem *val);

    void setDraggableObjects(const QList<QQuickItem *> &val);
    void clearDraggableObjects();
    void addDraggableObject(QQuickItem *val);

Q_SIGNALS:
    void borderWidthChanged(int);
    void borderHeightChanged(int);
    void titleBarHeightChanged(int);
    void resizableChanged(bool);
    void minimumSizeChanged(const QSize &);
    void maximumSizeChanged(const QSize &);
    void titleBarEnabledChanged(bool);

private:
#ifndef Q_OS_WINDOWS
    FramelessHelper m_framelessHelper;
#endif
};
