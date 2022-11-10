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

#include "framelessquickapplicationwindow_p.h"
#include "framelessquickapplicationwindow_p_p.h"
#include "framelessquickhelper.h"
#include "quickwindowborder.h"
#include <QtQuick/private/qquickitem_p.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessQuickApplicationWindow, "wangwenx190.framelesshelper.quick.framelessquickapplicationwindow")

#ifdef FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessQuickApplicationWindow)
#  define DEBUG qCDebug(lcFramelessQuickApplicationWindow)
#  define WARNING qCWarning(lcFramelessQuickApplicationWindow)
#  define CRITICAL qCCritical(lcFramelessQuickApplicationWindow)
#endif

using namespace Global;

FramelessQuickApplicationWindowPrivate::FramelessQuickApplicationWindowPrivate(FramelessQuickApplicationWindow *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessQuickApplicationWindowPrivate::~FramelessQuickApplicationWindowPrivate() = default;

FramelessQuickApplicationWindowPrivate *FramelessQuickApplicationWindowPrivate::get(FramelessQuickApplicationWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessQuickApplicationWindowPrivate *FramelessQuickApplicationWindowPrivate::get(const FramelessQuickApplicationWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

bool FramelessQuickApplicationWindowPrivate::isHidden() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (q->visibility() == FramelessQuickApplicationWindow::Hidden);
}

bool FramelessQuickApplicationWindowPrivate::isNormal() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (q->visibility() == FramelessQuickApplicationWindow::Windowed);
}

bool FramelessQuickApplicationWindowPrivate::isMinimized() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (q->visibility() == FramelessQuickApplicationWindow::Minimized);
}

bool FramelessQuickApplicationWindowPrivate::isMaximized() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (q->visibility() == FramelessQuickApplicationWindow::Maximized);
}

bool FramelessQuickApplicationWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (isMaximized() || (q->visibility() == FramelessQuickApplicationWindow::FullScreen));
}

bool FramelessQuickApplicationWindowPrivate::isFullScreen() const
{
    Q_Q(const FramelessQuickApplicationWindow);
    return (q->visibility() == FramelessQuickApplicationWindow::FullScreen);
}

void FramelessQuickApplicationWindowPrivate::showMinimized2()
{
    Q_Q(FramelessQuickApplicationWindow);
#ifdef Q_OS_WINDOWS
    // Work-around a QtQuick bug: https://bugreports.qt.io/browse/QTBUG-69711
    // Don't use "SW_SHOWMINIMIZED" because it will activate the current window
    // instead of the next window in the Z order, which is not the default behavior
    // of native Win32 applications.
    ShowWindow(reinterpret_cast<HWND>(q->winId()), SW_MINIMIZE);
#else
    q->showMinimized();
#endif
}

void FramelessQuickApplicationWindowPrivate::toggleMaximized()
{
    Q_Q(FramelessQuickApplicationWindow);
    if (isMaximized()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessQuickApplicationWindowPrivate::toggleFullScreen()
{
    Q_Q(FramelessQuickApplicationWindow);
    if (isFullScreen()) {
        q->setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = q->visibility();
        q->showFullScreen();
    }
}

void FramelessQuickApplicationWindowPrivate::initialize()
{
    Q_Q(FramelessQuickApplicationWindow);
    QQuickItem * const rootItem = q->contentItem();
    FramelessQuickHelper::get(rootItem)->extendsContentIntoTitleBar();
    m_windowBorder.reset(new QuickWindowBorder);
    m_windowBorder->setParent(rootItem);
    m_windowBorder->setParentItem(rootItem);
    m_windowBorder->setZ(999); // Make sure it always stays on the top.
    QQuickItemPrivate::get(m_windowBorder.data())->anchors()->setFill(rootItem);
    connect(q, &FramelessQuickApplicationWindow::visibilityChanged, q, [q](){
        Q_EMIT q->hiddenChanged();
        Q_EMIT q->normalChanged();
        Q_EMIT q->minimizedChanged();
        Q_EMIT q->maximizedChanged();
        Q_EMIT q->zoomedChanged();
        Q_EMIT q->fullScreenChanged();
    });
}

FramelessQuickApplicationWindow::FramelessQuickApplicationWindow(QWindow *parent)
    : QQuickApplicationWindow(parent), d_ptr(new FramelessQuickApplicationWindowPrivate(this))
{
}

FramelessQuickApplicationWindow::~FramelessQuickApplicationWindow() = default;

bool FramelessQuickApplicationWindow::isHidden() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isHidden();
}

bool FramelessQuickApplicationWindow::isNormal() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isNormal();
}

bool FramelessQuickApplicationWindow::isMinimized() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isMinimized();
}

bool FramelessQuickApplicationWindow::isMaximized() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isMaximized();
}

bool FramelessQuickApplicationWindow::isZoomed() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isZoomed();
}

bool FramelessQuickApplicationWindow::isFullScreen() const
{
    Q_D(const FramelessQuickApplicationWindow);
    return d->isFullScreen();
}

void FramelessQuickApplicationWindow::showMinimized2()
{
    Q_D(FramelessQuickApplicationWindow);
    d->showMinimized2();
}

void FramelessQuickApplicationWindow::toggleMaximized()
{
    Q_D(FramelessQuickApplicationWindow);
    d->toggleMaximized();
}

void FramelessQuickApplicationWindow::toggleFullScreen()
{
    Q_D(FramelessQuickApplicationWindow);
    d->toggleFullScreen();
}

void FramelessQuickApplicationWindow::classBegin()
{
    QQuickApplicationWindow::classBegin();
}

void FramelessQuickApplicationWindow::componentComplete()
{
    QQuickApplicationWindow::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE
