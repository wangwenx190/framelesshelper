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

#include "framelessquickwindow_p.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include "framelessquickwindow_p_p.h"
#include "framelessquickhelper.h"
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <framelessmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FramelessQuickWindowPrivate::FramelessQuickWindowPrivate(FramelessQuickWindow *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessQuickWindowPrivate::~FramelessQuickWindowPrivate() = default;

FramelessQuickWindowPrivate *FramelessQuickWindowPrivate::get(FramelessQuickWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessQuickWindowPrivate *FramelessQuickWindowPrivate::get(const FramelessQuickWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

bool FramelessQuickWindowPrivate::isHidden() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Hidden);
}

bool FramelessQuickWindowPrivate::isNormal() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Windowed);
}

bool FramelessQuickWindowPrivate::isMinimized() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Minimized);
}

bool FramelessQuickWindowPrivate::isMaximized() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Maximized);
}

bool FramelessQuickWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickWindow);
    return (isMaximized() || (q->visibility() == FramelessQuickWindow::FullScreen));
}

bool FramelessQuickWindowPrivate::isFullScreen() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::FullScreen);
}

QColor FramelessQuickWindowPrivate::getFrameBorderColor() const
{
#ifdef Q_OS_WINDOWS
    Q_Q(const FramelessQuickWindow);
    return Utils::getFrameBorderColor(q->isActive());
#else
    return {};
#endif
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderBottom() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::BottomAnchor);
}

void FramelessQuickWindowPrivate::showMinimized2()
{
    Q_Q(FramelessQuickWindow);
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

void FramelessQuickWindowPrivate::toggleMaximized()
{
    Q_Q(FramelessQuickWindow);
    if (isMaximized()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessQuickWindowPrivate::toggleFullScreen()
{
    Q_Q(FramelessQuickWindow);
    if (isFullScreen()) {
        q->setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = q->visibility();
        q->showFullScreen();
    }
}

void FramelessQuickWindowPrivate::initialize()
{
    Q_Q(FramelessQuickWindow);
    QQuickItem * const rootItem = q->contentItem();
    FramelessQuickHelper::get(rootItem)->extendsContentIntoTitleBar();
    m_topBorderRectangle.reset(new QQuickRectangle(rootItem));
    m_topBorderRectangle->setZ(999); // Make sure the frame border stays on top of eveything.
    m_topBorderRectangle->setColor(kDefaultTransparentColor);
    m_topBorderRectangle->setHeight(0.0);
    QQuickPen * const b = m_topBorderRectangle->border();
    b->setWidth(0.0);
    b->setColor(kDefaultTransparentColor);
    updateTopBorderHeight();
    updateTopBorderColor();
    m_topBorderAnchors.reset(new QQuickAnchors(m_topBorderRectangle.data(), m_topBorderRectangle.data()));
    const QQuickItemPrivate * const rootItemPrivate = QQuickItemPrivate::get(rootItem);
    m_topBorderAnchors->setTop(rootItemPrivate->top());
    m_topBorderAnchors->setLeft(rootItemPrivate->left());
    m_topBorderAnchors->setRight(rootItemPrivate->right());
    connect(q, &FramelessQuickWindow::visibilityChanged, this, [this, q](){
        updateTopBorderHeight();
        Q_EMIT q->hiddenChanged();
        Q_EMIT q->normalChanged();
        Q_EMIT q->minimizedChanged();
        Q_EMIT q->maximizedChanged();
        Q_EMIT q->zoomedChanged();
        Q_EMIT q->fullScreenChanged();
    });
    connect(q, &FramelessQuickWindow::activeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
}

bool FramelessQuickWindowPrivate::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
    return (Utils::isWindowFrameBorderVisible() && !isWin11OrGreater);
#else
    return false;
#endif
}

void FramelessQuickWindowPrivate::updateTopBorderColor()
{
#ifdef Q_OS_WINDOWS
    if (!shouldDrawFrameBorder()) {
        return;
    }
    m_topBorderRectangle->setColor(getFrameBorderColor());
#endif
}

void FramelessQuickWindowPrivate::updateTopBorderHeight()
{
#ifdef Q_OS_WINDOWS
    if (!shouldDrawFrameBorder()) {
        return;
    }
    const qreal newHeight = (isNormal() ? qreal(kDefaultWindowFrameBorderThickness) : 0.0);
    m_topBorderRectangle->setHeight(newHeight);
#endif
}

FramelessQuickWindow::FramelessQuickWindow(QWindow *parent)
    : QQuickWindow(parent), d_ptr(new FramelessQuickWindowPrivate(this))
{
}

FramelessQuickWindow::~FramelessQuickWindow() = default;

bool FramelessQuickWindow::isHidden() const
{
    Q_D(const FramelessQuickWindow);
    return d->isHidden();
}

bool FramelessQuickWindow::isNormal() const
{
    Q_D(const FramelessQuickWindow);
    return d->isNormal();
}

bool FramelessQuickWindow::isMinimized() const
{
    Q_D(const FramelessQuickWindow);
    return d->isMinimized();
}

bool FramelessQuickWindow::isMaximized() const
{
    Q_D(const FramelessQuickWindow);
    return d->isMaximized();
}

bool FramelessQuickWindow::isZoomed() const
{
    Q_D(const FramelessQuickWindow);
    return d->isZoomed();
}

bool FramelessQuickWindow::isFullScreen() const
{
    Q_D(const FramelessQuickWindow);
    return d->isFullScreen();
}

void FramelessQuickWindow::showMinimized2()
{
    Q_D(FramelessQuickWindow);
    d->showMinimized2();
}

void FramelessQuickWindow::toggleMaximized()
{
    Q_D(FramelessQuickWindow);
    d->toggleMaximized();
}

void FramelessQuickWindow::toggleFullScreen()
{
    Q_D(FramelessQuickWindow);
    d->toggleFullScreen();
}

FRAMELESSHELPER_END_NAMESPACE
#endif
