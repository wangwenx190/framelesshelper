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

using namespace Global;

FramelessMainWindow::FramelessMainWindow(QWidget *parent, const Qt::WindowFlags flags, const UserSettings &settings) : QMainWindow(parent, flags)
{
    m_helper.reset(new FramelessWidgetsHelper(this, settings));
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

bool FramelessMainWindow::isFixedSize() const
{
    return m_helper->isFixedSize();
}

void FramelessMainWindow::setFixedSize(const bool value)
{
    m_helper->setFixedSize(value);
}

void FramelessMainWindow::setTitleBarWidget(QWidget *widget)
{
    m_helper->setTitleBarWidget(widget);
}

QWidget *FramelessMainWindow::titleBarWidget() const
{
    return m_helper->getTitleBarWidget();
}

void FramelessMainWindow::setHitTestVisible(QWidget *widget)
{
    m_helper->setHitTestVisible(widget);
}

void FramelessMainWindow::toggleMaximized()
{
    m_helper->toggleMaximized();
}

void FramelessMainWindow::toggleFullScreen()
{
    m_helper->toggleFullScreen();
}

void FramelessMainWindow::moveToDesktopCenter()
{
    m_helper->moveToDesktopCenter();
}

void FramelessMainWindow::bringToFront()
{
    m_helper->bringToFront();
}

void FramelessMainWindow::showSystemMenu(const QPoint &pos)
{
    m_helper->showSystemMenu(pos);
}

void FramelessMainWindow::startSystemMove2(const QPoint &pos)
{
    m_helper->startSystemMove2(pos);
}

void FramelessMainWindow::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    m_helper->startSystemResize2(edges, pos);
}

FRAMELESSHELPER_END_NAMESPACE
