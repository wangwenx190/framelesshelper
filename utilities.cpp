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

#include "utilities.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

Qt::CursorShape Utilities::calculateCursorShape(const QWindow *window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    if (((pos.x() < kDefaultResizeBorderThickness) && (pos.y() < kDefaultResizeBorderThickness))
        || ((pos.x() >= (window->width() - kDefaultResizeBorderThickness)) && (pos.y() >= (window->height() - kDefaultResizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((pos.x() >= (window->width() - kDefaultResizeBorderThickness)) && (pos.y() < kDefaultResizeBorderThickness))
        || ((pos.x() < kDefaultResizeBorderThickness) && (pos.y() >= (window->height() - kDefaultResizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((pos.x() < kDefaultResizeBorderThickness) || (pos.x() >= (window->width() - kDefaultResizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((pos.y() < kDefaultResizeBorderThickness) || (pos.y() >= (window->height() - kDefaultResizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

Qt::Edges Utilities::calculateWindowEdges(const QWindow *window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    if (pos.x() < kDefaultResizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= (window->width() - kDefaultResizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() < kDefaultResizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= (window->height() - kDefaultResizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

bool Utilities::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    if (window->flags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    return (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize));
}

FRAMELESSHELPER_END_NAMESPACE
