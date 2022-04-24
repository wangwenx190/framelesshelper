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

#include "framelesswidget.h"
#include "framelesswidgetshelper_p.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FramelessWidget::FramelessWidget(QWidget *parent, const UserSettings &settings) : QWidget(parent)
{
    d_ptr.reset(new FramelessWidgetsHelper(this, settings));
}

FramelessWidget::~FramelessWidget() = default;

bool FramelessWidget::isNormal() const
{
    return d_ptr->isNormal();
}

bool FramelessWidget::isZoomed() const
{
    return d_ptr->isZoomed();
}

bool FramelessWidget::isFixedSize() const
{
    return d_ptr->isFixedSize();
}

void FramelessWidget::setFixedSize(const bool value)
{
    d_ptr->setFixedSize(value);
}

void FramelessWidget::setTitleBarWidget(QWidget *widget)
{
    d_ptr->setTitleBarWidget(widget);
}

QWidget *FramelessWidget::titleBarWidget() const
{
    return d_ptr->getTitleBarWidget();
}

void FramelessWidget::setContentWidget(QWidget *widget)
{
    d_ptr->setContentWidget(widget);
}

QWidget *FramelessWidget::contentWidget() const
{
    return d_ptr->getContentWidget();
}

void FramelessWidget::setHitTestVisible(QWidget *widget)
{
    d_ptr->setHitTestVisible(widget);
}

void FramelessWidget::toggleMaximized()
{
    d_ptr->toggleMaximized();
}

void FramelessWidget::toggleFullScreen()
{
    d_ptr->toggleFullScreen();
}

void FramelessWidget::moveToDesktopCenter()
{
    d_ptr->moveToDesktopCenter();
}

void FramelessWidget::bringToFront()
{
    d_ptr->bringToFront();
}

void FramelessWidget::showSystemMenu(const QPoint &pos)
{
    d_ptr->showSystemMenu(pos);
}

void FramelessWidget::startSystemMove2(const QPoint &pos)
{
    d_ptr->startSystemMove2(pos);
}

void FramelessWidget::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    d_ptr->startSystemResize2(edges, pos);
}

FRAMELESSHELPER_END_NAMESPACE
