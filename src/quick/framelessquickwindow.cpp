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
#include "framelessquickhelper.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

FramelessQuickWindowPrivate::FramelessQuickWindowPrivate(FramelessQuickWindow *q, const Options options) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    m_options = options;
    initialize();
}

FramelessQuickWindowPrivate::~FramelessQuickWindowPrivate() = default;

bool FramelessQuickWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickWindow);
    const FramelessQuickWindow::Visibility visibility = q->visibility();
    return ((visibility == FramelessQuickWindow::Maximized) ||
            ((m_options & Option::DontTreatFullScreenAsZoomed) ? false : (visibility == FramelessQuickWindow::FullScreen)));
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

void FramelessQuickWindowPrivate::setTitleBarItem(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    FramelessQuickHelper::setTitleBarItem(q, item);
}

void FramelessQuickWindowPrivate::setHitTestVisible(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    FramelessQuickHelper::setHitTestVisible(q, item);
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

void FramelessQuickWindowPrivate::toggleMaximize()
{
    Q_Q(FramelessQuickWindow);
    if (Utils::isWindowFixedSize(q)) {
        return;
    }
    if (isZoomed()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessQuickWindowPrivate::toggleFullScreen()
{
    Q_Q(FramelessQuickWindow);
    if (Utils::isWindowFixedSize(q)) {
        return;
    }
    const QWindow::Visibility visibility = q->visibility();
    if (visibility == QWindow::FullScreen) {
        q->setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = visibility;
        q->showFullScreen();
    }
}

void FramelessQuickWindowPrivate::showSystemMenu(const QPoint &pos)
{
#ifdef Q_OS_WINDOWS
    Q_Q(FramelessQuickWindow);
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint globalPos = q->mapToGlobal(pos);
#  else
    const QPoint globalPos = q->mapToGlobal(pos);
#  endif
    const QPoint nativePos = QPointF(QPointF(globalPos) * q->effectiveDevicePixelRatio()).toPoint();
    Utils::showSystemMenu(q, nativePos);
#endif
}

void FramelessQuickWindowPrivate::startSystemMove2()
{
    Q_Q(FramelessQuickWindow);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    q->startSystemMove();
#else
    Utils::startSystemMove(q);
#endif
}

void FramelessQuickWindowPrivate::startSystemResize2(const Qt::Edges edges)
{
    if (edges == Qt::Edges{}) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    if (Utils::isWindowFixedSize(q)) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    q->startSystemResize(edges);
#else
    Utils::startSystemResize(q, edges);
#endif
}

void FramelessQuickWindowPrivate::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    Q_Q(FramelessQuickWindow);
    q->setProperty(kInternalOptionsFlag, QVariant::fromValue(m_options));
    FramelessQuickHelper::addWindow(q);
#ifdef Q_OS_WINDOWS
    if (isFrameBorderVisible()) {
        QQuickItem * const rootItem = q->contentItem();
        const QQuickItemPrivate * const rootItemPrivate = QQuickItemPrivate::get(rootItem);
        m_topBorderRectangle.reset(new QQuickRectangle(rootItem));
        updateTopBorderHeight();
        updateTopBorderColor();
        connect(q, &FramelessQuickWindow::visibilityChanged, this, [this, q](){
            updateTopBorderHeight();
            Q_EMIT q->zoomedChanged();
        });
        connect(q, &FramelessQuickWindow::activeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
        connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, [this, q](){
            updateTopBorderColor();
            Q_EMIT q->frameBorderColorChanged();
        });
        const auto topBorderAnchors = new QQuickAnchors(m_topBorderRectangle.data(), m_topBorderRectangle.data());
        topBorderAnchors->setTop(rootItemPrivate->top());
        topBorderAnchors->setLeft(rootItemPrivate->left());
        topBorderAnchors->setRight(rootItemPrivate->right());
    }
#endif
    Utils::moveWindowToDesktopCenter([q]() -> QScreen * { return q->screen(); },
                                     [q]() -> QSize { return q->size(); },
                                     [q](const int x, const int y) -> void {
                                         q->setX(x);
                                         q->setY(y);
                                     }, true);
}

bool FramelessQuickWindowPrivate::isFrameBorderVisible() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater());
#else
    return false;
#endif
}

void FramelessQuickWindowPrivate::updateTopBorderColor()
{
    if (!isFrameBorderVisible()) {
        return;
    }
    m_topBorderRectangle->setColor(getFrameBorderColor());
}

void FramelessQuickWindowPrivate::updateTopBorderHeight()
{
    if (!isFrameBorderVisible()) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    const qreal newHeight = ((q->visibility() == FramelessQuickWindow::Windowed) ? 1.0 : 0.0);
    m_topBorderRectangle->setHeight(newHeight);
}

FramelessQuickWindow::FramelessQuickWindow(QWindow *parent, const Options options) : QQuickWindow(parent)
{
    d_ptr.reset(new FramelessQuickWindowPrivate(this, options));
}

FramelessQuickWindow::~FramelessQuickWindow() = default;

bool FramelessQuickWindow::zoomed() const
{
    Q_D(const FramelessQuickWindow);
    return d->isZoomed();
}

QColor FramelessQuickWindow::frameBorderColor() const
{
    Q_D(const FramelessQuickWindow);
    return d->getFrameBorderColor();
}

void FramelessQuickWindow::setTitleBarItem(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->setTitleBarItem(item);
}

void FramelessQuickWindow::setHitTestVisible(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->setHitTestVisible(item);
}

void FramelessQuickWindow::showMinimized2()
{
    Q_D(FramelessQuickWindow);
    d->showMinimized2();
}

void FramelessQuickWindow::toggleMaximize()
{
    Q_D(FramelessQuickWindow);
    d->toggleMaximize();
}

void FramelessQuickWindow::toggleFullScreen()
{
    Q_D(FramelessQuickWindow);
    d->toggleFullScreen();
}

void FramelessQuickWindow::showSystemMenu(const QPoint &pos)
{
    Q_D(FramelessQuickWindow);
    d->showSystemMenu(pos);
}

void FramelessQuickWindow::startSystemMove2()
{
    Q_D(FramelessQuickWindow);
    d->startSystemMove2();
}

void FramelessQuickWindow::startSystemResize2(const Qt::Edges edges)
{
    Q_D(FramelessQuickWindow);
    d->startSystemResize2(edges);
}

FRAMELESSHELPER_END_NAMESPACE
