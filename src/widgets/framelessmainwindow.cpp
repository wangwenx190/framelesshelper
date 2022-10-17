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
#include "framelessmainwindow_p.h"
#include "framelesswidgetshelper.h"
#include "widgetssharedhelper_p.h"
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessMainWindow, "wangwenx190.framelesshelper.widgets.framelessmainwindow")
#define INFO qCInfo(lcFramelessMainWindow)
#define DEBUG qCDebug(lcFramelessMainWindow)
#define WARNING qCWarning(lcFramelessMainWindow)
#define CRITICAL qCCritical(lcFramelessMainWindow)

using namespace Global;

FramelessMainWindowPrivate::FramelessMainWindowPrivate(FramelessMainWindow *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessMainWindowPrivate::~FramelessMainWindowPrivate() = default;

FramelessMainWindowPrivate *FramelessMainWindowPrivate::get(FramelessMainWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessMainWindowPrivate *FramelessMainWindowPrivate::get(const FramelessMainWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void FramelessMainWindowPrivate::initialize()
{
    Q_Q(FramelessMainWindow);
    FramelessWidgetsHelper::get(q)->extendsContentIntoTitleBar();
    m_helper.reset(new WidgetsSharedHelper(this));
    m_helper->setup(q);
}

bool FramelessMainWindowPrivate::isNormal() const
{
    Q_Q(const FramelessMainWindow);
    return (Utils::windowStatesToWindowState(q->windowState()) == Qt::WindowNoState);
}

bool FramelessMainWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessMainWindow);
    return (q->isMaximized() || q->isFullScreen());
}

void FramelessMainWindowPrivate::toggleMaximized()
{
    Q_Q(FramelessMainWindow);
    if (q->isMaximized()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessMainWindowPrivate::toggleFullScreen()
{
    Q_Q(FramelessMainWindow);
    if (q->isFullScreen()) {
        q->setWindowState(m_savedWindowState);
    } else {
        m_savedWindowState = Utils::windowStatesToWindowState(q->windowState());
        q->showFullScreen();
    }
}

WidgetsSharedHelper *FramelessMainWindowPrivate::widgetsSharedHelper() const
{
    return (m_helper.isNull() ? nullptr : m_helper.data());
}

FramelessMainWindow::FramelessMainWindow(QWidget *parent, const Qt::WindowFlags flags)
    : QMainWindow(parent, flags), d_ptr(new FramelessMainWindowPrivate(this))
{
}

FramelessMainWindow::~FramelessMainWindow() = default;

bool FramelessMainWindow::isNormal() const
{
    Q_D(const FramelessMainWindow);
    return d->isNormal();
}

bool FramelessMainWindow::isZoomed() const
{
    Q_D(const FramelessMainWindow);
    return d->isZoomed();
}

void FramelessMainWindow::toggleMaximized()
{
    Q_D(FramelessMainWindow);
    d->toggleMaximized();
}

void FramelessMainWindow::toggleFullScreen()
{
    Q_D(FramelessMainWindow);
    d->toggleFullScreen();
}

FRAMELESSHELPER_END_NAMESPACE
