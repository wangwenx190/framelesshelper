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

#include "framelessmainwindow.h"
#include "framelesswidgetshelper.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessMainWindow::FramelessMainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    m_helper.reset(new FramelessWidgetsHelper(this, WindowLayout::Custom));
}

FramelessMainWindow::~FramelessMainWindow() = default;

bool FramelessMainWindow::isNormal() const
{
    return m_helper->isNormal();
}

bool FramelessMainWindow::isZoomed() const
{
    return m_helper->isZoomed();
}

void FramelessMainWindow::setTitleBarWidget(QWidget *widget)
{
    m_helper->setTitleBarWidget(widget);
}

QWidget *FramelessMainWindow::titleBarWidget() const
{
    return m_helper->titleBarWidget();
}

void FramelessMainWindow::setHitTestVisible(QWidget *widget, const bool visible)
{
    m_helper->setHitTestVisible(widget, visible);
}

void FramelessMainWindow::toggleMaximized()
{
    m_helper->toggleMaximized();
}

void FramelessMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    m_helper->showEventHandler(event);
}

void FramelessMainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    m_helper->changeEventHandler(event);
}

void FramelessMainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    m_helper->paintEventHandler(event);
}

void FramelessMainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
    m_helper->mousePressEventHandler(event);
}

void FramelessMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
    m_helper->mouseReleaseEventHandler(event);
}

void FramelessMainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QMainWindow::mouseDoubleClickEvent(event);
    m_helper->mouseDoubleClickEventHandler(event);
}

FRAMELESSHELPER_END_NAMESPACE
