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

using namespace Global;

FramelessQuickWindowPrivate::FramelessQuickWindowPrivate(FramelessQuickWindow *q, const UserSettings &settings) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    m_settings = settings;
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

bool FramelessQuickWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickWindow);
    const FramelessQuickWindow::Visibility visibility = q->visibility();
    return ((visibility == FramelessQuickWindow::Maximized) ||
            ((m_settings.options & Option::DontTreatFullScreenAsZoomed)
                 ? false : (visibility == FramelessQuickWindow::FullScreen)));
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

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderTop() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::TopAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderBottom() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::BottomAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderLeft() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::LeftAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderRight() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::RightAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderHorizontalCenter() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::HCenterAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderVerticalCenter() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::VCenterAnchor);
}

void FramelessQuickWindowPrivate::snapToTopBorder(QQuickItem *item, const QuickGlobal::Anchor itemAnchor, const QuickGlobal::Anchor topBorderAnchor)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    const QQuickAnchorLine targetAnchorLine = [this, topBorderAnchor]() -> QQuickAnchorLine {
        switch (topBorderAnchor) {
        case QuickGlobal::Anchor::Top:
            return getTopBorderTop();
        case QuickGlobal::Anchor::Bottom:
            return getTopBorderBottom();
        case QuickGlobal::Anchor::Left:
            return getTopBorderLeft();
        case QuickGlobal::Anchor::Right:
            return getTopBorderRight();
        case QuickGlobal::Anchor::HorizontalCenter:
            return getTopBorderHorizontalCenter();
        case QuickGlobal::Anchor::VerticalCenter:
            return getTopBorderVerticalCenter();
        default:
            break;
        }
        return {};
    }();
    const QQuickItemPrivate * const itemPrivate = QQuickItemPrivate::get(item);
    QQuickAnchors * const anchors = itemPrivate->anchors();
    switch (itemAnchor) {
    case QuickGlobal::Anchor::Top:
        anchors->setTop(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Bottom:
        anchors->setBottom(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Left:
        anchors->setLeft(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Right:
        anchors->setRight(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::HorizontalCenter:
        anchors->setHorizontalCenter(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::VerticalCenter:
        anchors->setVerticalCenter(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Center:
        anchors->setCenterIn(m_topBorderRectangle.data());
        break;
    }
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
    if (isZoomed()) {
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
    if (m_settings.options & Option::TransparentWindowBackground) {
        q->setColor(kDefaultTransparentColor);
    }
    QQuickItem * const rootItem = q->contentItem();
    const QQuickItemPrivate * const rootItemPrivate = QQuickItemPrivate::get(rootItem);
    m_topBorderRectangle.reset(new QQuickRectangle(rootItem));
    m_topBorderRectangle->setColor(kDefaultTransparentColor);
    m_topBorderRectangle->setHeight(0.0);
    QQuickPen * const _border = m_topBorderRectangle->border();
    _border->setWidth(0.0);
    _border->setColor(kDefaultTransparentColor);
    updateTopBorderHeight();
    updateTopBorderColor();
    m_topBorderAnchors.reset(new QQuickAnchors(m_topBorderRectangle.data(), m_topBorderRectangle.data()));
    m_topBorderAnchors->setTop(rootItemPrivate->top());
    m_topBorderAnchors->setLeft(rootItemPrivate->left());
    m_topBorderAnchors->setRight(rootItemPrivate->right());
    connect(q, &FramelessQuickWindow::visibilityChanged, this, [this, q](){
        updateTopBorderHeight();
        Q_EMIT q->hiddenChanged();
        Q_EMIT q->normalChanged();
        Q_EMIT q->minimizedChanged();
        Q_EMIT q->zoomedChanged();
        Q_EMIT q->fullScreenChanged();
    });
    connect(q, &FramelessQuickWindow::activeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, [this, q](){
        updateTopBorderColor();
        Q_EMIT q->frameBorderColorChanged();
    });
}

bool FramelessQuickWindowPrivate::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2)
            && !(m_settings.options & Option::DontDrawTopWindowFrameBorder));
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
    const qreal newHeight = (isNormal() ? 1.0 : 0.0);
    m_topBorderRectangle->setHeight(newHeight);
#endif
}

FramelessQuickWindow::FramelessQuickWindow(QWindow *parent, const UserSettings &settings)
    : QQuickWindow(parent), d_ptr(new FramelessQuickWindowPrivate(this, settings))
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

QColor FramelessQuickWindow::frameBorderColor() const
{
    Q_D(const FramelessQuickWindow);
    return d->getFrameBorderColor();
}

void FramelessQuickWindow::snapToTopBorder(QQuickItem *item, const QuickGlobal::Anchor itemAnchor, const QuickGlobal::Anchor topBorderAnchor)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->snapToTopBorder(item, itemAnchor, topBorderAnchor);
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