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
#include "widgetssharedhelper_p.h"
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FramelessMainWindow::FramelessMainWindow(QWidget *parent, const Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    FramelessWidgetsHelper::get(this)->attach();
    m_helper.reset(new WidgetsSharedHelper(this));
    m_helper->setup(this);
}

FramelessMainWindow::~FramelessMainWindow() = default;

bool FramelessMainWindow::isNormal() const
{
    return (Utils::windowStatesToWindowState(windowState()) == Qt::WindowNoState);
}

bool FramelessMainWindow::isZoomed() const
{
    return (isMaximized() || isFullScreen());
}

void FramelessMainWindow::toggleMaximized()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void FramelessMainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        setWindowState(m_savedWindowState);
    } else {
        m_savedWindowState = Utils::windowStatesToWindowState(windowState());
        showFullScreen();
    }
}

FRAMELESSHELPER_END_NAMESPACE
