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

#include "framelessquickwindow.h"
#include "framelessquickwindow_p.h"
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <framelesswindowsmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

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

bool FramelessQuickWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickWindow);
    const FramelessQuickWindow::Visibility visibility = q->visibility();
    return ((visibility == FramelessQuickWindow::Maximized) || (visibility == FramelessQuickWindow::FullScreen));
}

void FramelessQuickWindowPrivate::initialize()
{
    Q_Q(FramelessQuickWindow);
    FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    manager->addWindow(q);
    QQuickItem * const rootItem = q->contentItem();
    const QQuickItemPrivate * const rootItemPrivate = QQuickItemPrivate::get(rootItem);
    m_topBorderRectangle.reset(new QQuickRectangle(rootItem));
    updateTopBorderHeight();
    updateTopBorderColor();
    connect(q, &FramelessQuickWindow::visibilityChanged, this, &FramelessQuickWindowPrivate::updateTopBorderHeight);
    connect(q, &FramelessQuickWindow::activeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
    const auto topBorderAnchors = new QQuickAnchors(m_topBorderRectangle.data(), m_topBorderRectangle.data());
    topBorderAnchors->setTop(rootItemPrivate->top());
    topBorderAnchors->setLeft(rootItemPrivate->left());
    topBorderAnchors->setRight(rootItemPrivate->right());
    connect(q, &FramelessQuickWindow::visibilityChanged, q, &FramelessQuickWindow::zoomedChanged);
}

void FramelessQuickWindowPrivate::updateTopBorderColor()
{
    Q_Q(FramelessQuickWindow);
    m_topBorderRectangle->setColor(Utils::getFrameBorderColor(q->isActive()));
}

void FramelessQuickWindowPrivate::updateTopBorderHeight()
{
    Q_Q(FramelessQuickWindow);
    const qreal newHeight = ((q->visibility() == FramelessQuickWindow::Windowed) ? 1.0 : 0.0);
    m_topBorderRectangle->setHeight(newHeight);
}

FramelessQuickWindow::FramelessQuickWindow(QWindow *parent) : QQuickWindow(parent)
{
    d_ptr.reset(new FramelessQuickWindowPrivate(this));
}

FramelessQuickWindow::~FramelessQuickWindow() = default;

bool FramelessQuickWindow::zoomed() const
{
    Q_D(const FramelessQuickWindow);
    return d->isZoomed();
}

FRAMELESSHELPER_END_NAMESPACE
