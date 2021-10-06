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

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_FORWARD_DECLARE_CLASS(QMouseEvent)
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_API FramelessHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessHelper)

public:
    explicit FramelessHelper(QWindow *window);
    ~FramelessHelper() override = default;

    void install();
    void uninstall();

    QWindow *window() { return m_window; }

    QSize windowSize() { return m_windowSize; }
    void setWindowSize(const QSize& size) { m_windowSize = size; }
    void resizeWindow(const QSize& windowSize);

    int titleBarHeight() { return m_titleBarHeight; }
    void setTitleBarHeight(int height) { m_titleBarHeight = height; }
    QRect titleBarRect();
    QRegion titleBarRegion();

    int resizeBorderThickness() { return m_resizeBorderThickness; }
    void setResizeBorderThickness(int thickness) { m_resizeBorderThickness = thickness; }

    bool resizable() { return m_resizable; }
    void setResizable(bool resizable) { m_resizable = resizable; }

    QRect clientRect();
    QRegion nonClientRegion();

    bool isInTitlebarArea(const QPoint& pos);
    Qt::WindowFrameSection mapPosToFrameSection(const QPoint& pos);

    bool isHoverResizeHandler();
    bool isClickResizeHandler();

    QCursor cursorForFrameSection(Qt::WindowFrameSection frameSection);
    void setCursor(const QCursor& cursor);
    void unsetCursor();
    void updateCursor();

    void updateMouse(const QPoint& pos);
    void updateHoverStates(const QPoint& pos);

    void startMove(const QPoint &globalPos);
    void startResize(const QPoint &globalPos, Qt::WindowFrameSection frameSection);

    void setHitTestVisible(QObject *obj);
    bool isHitTestVisible(QObject *obj);
    QRect getHTVObjectRect(QObject *obj);

#ifdef Q_OS_WIN
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool handleNativeEvent(QWindow *window, const QByteArray &eventType, void *message, qintptr *result);
#else
    bool handleNativeEvent(QWindow *window, const QByteArray &eventType, void *message, long *result);
#endif
#endif // Q_OS_WIN

private:
#ifdef Q_OS_WIN
    void updateQtFrameMargins(const bool enable);
#endif // Q_OS_WIN

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void handleResizeHandlerDblClicked();

private:
    QWindow *m_window;
    QSize m_windowSize;
    int m_titleBarHeight;
    int m_resizeBorderThickness;
    bool m_resizable;
    Qt::WindowFlags m_origWindowFlags;
    bool m_cursorChanged;
    Qt::WindowFrameSection m_hoveredFrameSection;
    Qt::WindowFrameSection m_clickedFrameSection;
    QList<QObject*> m_HTVObjects;
};

FRAMELESSHELPER_END_NAMESPACE
