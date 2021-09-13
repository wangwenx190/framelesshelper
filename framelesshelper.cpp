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

#include "framelesshelper.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include "framelesswindowsmanager.h"
#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessHelper::FramelessHelper(QWindow *window)
    : QObject(window)
    , m_window(window)
{
    Q_ASSERT(window != nullptr && window->isTopLevel());
}

/*!
    Setup the window, make it frameless.
 */
void FramelessHelper::install()
{
    QRect origRect = m_window->geometry();

    resizeWindow(origRect.size());
}

/*!
    Restore the window to its original state
 */
void FramelessHelper::uninstall()
{
    resizeWindow(QSize());
}

/*!
    Resize non-client area
 */
void FramelessHelper::resizeWindow(const QSize& windowSize)
{
    if (windowSize == this->windowSize())
        return;

    setWindowSize(windowSize);
}

QRect FramelessHelper::titleBarRect()
{
    return QRect(0, 0, windowSize().width(), titleBarHeight());
}

QRect FramelessHelper::clientRect()
{
    QRect rect(0, 0, windowSize().width(), windowSize().height());
    rect = rect.adjusted(
        resizeBorderThickness(), titleBarHeight(),
        -resizeBorderThickness(), -resizeBorderThickness()
    );
    return rect;
}

QRegion FramelessHelper::nonClientRegion()
{
    QRegion region(QRect(QPoint(0, 0), windowSize()));
    region -= clientRect();
    return region;
}

bool FramelessHelper::eventFilter(QObject *object, QEvent *event)
{

}

FRAMELESSHELPER_END_NAMESPACE

#endif
