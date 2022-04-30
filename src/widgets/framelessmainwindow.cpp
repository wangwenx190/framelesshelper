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
#include "framelesswidgetshelper_p.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FramelessMainWindow::FramelessMainWindow(QWidget *parent, const Qt::WindowFlags flags, const UserSettings &settings) : QMainWindow(parent, flags)
{
    d_ptr.reset(new FramelessWidgetsHelper(this, settings));
}

FramelessMainWindow::~FramelessMainWindow() = default;

bool FramelessMainWindow::isNormal() const
{
    return d_ptr->isNormal();
}

bool FramelessMainWindow::isZoomed() const
{
    return d_ptr->isZoomed();
}

bool FramelessMainWindow::isFixedSize() const
{
    return d_ptr->isFixedSize();
}

void FramelessMainWindow::setFixedSize(const bool value)
{
    d_ptr->setFixedSize(value);
}

void FramelessMainWindow::setTitleBarWidget(QWidget *widget)
{
    d_ptr->setTitleBarWidget(widget);
}

QWidget *FramelessMainWindow::titleBarWidget() const
{
    return d_ptr->getTitleBarWidget();
}

void FramelessMainWindow::setHitTestVisible(QWidget *widget)
{
    d_ptr->setHitTestVisible(widget);
}

void FramelessMainWindow::toggleMaximized()
{
    d_ptr->toggleMaximized();
}

void FramelessMainWindow::toggleFullScreen()
{
    d_ptr->toggleFullScreen();
}

void FramelessMainWindow::moveToDesktopCenter()
{
    d_ptr->moveToDesktopCenter();
}

void FramelessMainWindow::bringToFront()
{
    d_ptr->bringToFront();
}

void FramelessMainWindow::showSystemMenu(const QPoint &pos)
{
    d_ptr->showSystemMenu(pos);
}

void FramelessMainWindow::startSystemMove2(const QPoint &pos)
{
    d_ptr->startSystemMove2(pos);
}

void FramelessMainWindow::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    d_ptr->startSystemResize2(edges, pos);
}

void FramelessMainWindow::setSystemButton(QWidget *widget, const SystemButtonType buttonType)
{
    d_ptr->setSystemButton(widget, buttonType);
}

FRAMELESSHELPER_END_NAMESPACE
