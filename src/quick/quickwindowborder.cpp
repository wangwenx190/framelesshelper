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

#include "quickwindowborder.h"
#include "quickwindowborder_p.h"
#include <windowborderpainter.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/qquickwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickWindowBorder, "wangwenx190.framelesshelper.quick.quickwindowborder")
#define INFO qCInfo(lcQuickWindowBorder)
#define DEBUG qCDebug(lcQuickWindowBorder)
#define WARNING qCWarning(lcQuickWindowBorder)
#define CRITICAL qCCritical(lcQuickWindowBorder)

using namespace Global;

[[nodiscard]] static inline QuickGlobal::WindowEdges edgesToQuickEdges(const WindowEdges edges)
{
    QuickGlobal::WindowEdges result = {};
    if (edges & WindowEdge::Left) {
        result |= QuickGlobal::WindowEdge::Left;
    }
    if (edges & WindowEdge::Top) {
        result |= QuickGlobal::WindowEdge::Top;
    }
    if (edges & WindowEdge::Right) {
        result |= QuickGlobal::WindowEdge::Right;
    }
    if (edges & WindowEdge::Bottom) {
        result |= QuickGlobal::WindowEdge::Bottom;
    }
    return result;
}

[[nodiscard]] static inline WindowEdges quickEdgesToEdges(const QuickGlobal::WindowEdges edges)
{
    WindowEdges result = {};
    if (edges & QuickGlobal::WindowEdge::Left) {
        result |= WindowEdge::Left;
    }
    if (edges & QuickGlobal::WindowEdge::Top) {
        result |= WindowEdge::Top;
    }
    if (edges & QuickGlobal::WindowEdge::Right) {
        result |= WindowEdge::Right;
    }
    if (edges & QuickGlobal::WindowEdge::Bottom) {
        result |= WindowEdge::Bottom;
    }
    return result;
}

QuickWindowBorderPrivate::QuickWindowBorderPrivate(QuickWindowBorder *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickWindowBorderPrivate::~QuickWindowBorderPrivate() = default;

QuickWindowBorderPrivate *QuickWindowBorderPrivate::get(QuickWindowBorder *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

const QuickWindowBorderPrivate *QuickWindowBorderPrivate::get(const QuickWindowBorder *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

void QuickWindowBorderPrivate::paint(QPainter *painter) const
{
    Q_ASSERT(painter);
    if (!painter || m_borderPainter.isNull()) {
        return;
    }
    Q_Q(const QuickWindowBorder);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const QSize size = q->size().toSize();
#else
    const QSize size = QSizeF(q->width(), q->height()).toSize();
#endif
    m_borderPainter->paint(painter, size, (q->window() && q->window()->isActive()));
}

void QuickWindowBorderPrivate::update()
{
    Q_Q(QuickWindowBorder);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    q->update();
    q->setVisible(window->visibility() == QQuickWindow::Windowed);
}

void QuickWindowBorderPrivate::initialize()
{
    Q_Q(QuickWindowBorder);
    q->setClip(true);
    q->setSmooth(true);
    // We can't enable antialising for this element due to we are drawing
    // some very thin lines that are too fragile.
    q->setAntialiasing(false);

    m_borderPainter.reset(new WindowBorderPainter);
    connect(m_borderPainter.data(), &WindowBorderPainter::thicknessChanged,
        q, &QuickWindowBorder::thicknessChanged);
    connect(m_borderPainter.data(), &WindowBorderPainter::edgesChanged,
        q, &QuickWindowBorder::edgesChanged);
    connect(m_borderPainter.data(), &WindowBorderPainter::activeColorChanged,
        q, &QuickWindowBorder::activeColorChanged);
    connect(m_borderPainter.data(), &WindowBorderPainter::inactiveColorChanged,
        q, &QuickWindowBorder::inactiveColorChanged);
    connect(m_borderPainter.data(), &WindowBorderPainter::nativeBorderChanged,
        q, &QuickWindowBorder::nativeBorderChanged);
    connect(m_borderPainter.data(), &WindowBorderPainter::shouldRepaint, q, [q](){ q->update(); });
}

void QuickWindowBorderPrivate::rebindWindow()
{
    Q_Q(QuickWindowBorder);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    QQuickItem * const rootItem = window->contentItem();
    q->setParent(rootItem);
    q->setParentItem(rootItem);
    QQuickItemPrivate::get(q)->anchors()->setFill(rootItem);
    q->setZ(999); // Make sure we always stays on the top most place.
    if (m_activeChangeConnection) {
        disconnect(m_activeChangeConnection);
        m_activeChangeConnection = {};
    }
    if (m_visibilityChangeConnection) {
        disconnect(m_visibilityChangeConnection);
        m_visibilityChangeConnection = {};
    }
    m_activeChangeConnection = connect(window, &QQuickWindow::activeChanged,
        this, &QuickWindowBorderPrivate::update);
    m_visibilityChangeConnection = connect(window, &QQuickWindow::visibilityChanged,
        this, &QuickWindowBorderPrivate::update);
}

QuickWindowBorder::QuickWindowBorder(QQuickItem *parent)
    : QQuickPaintedItem(parent), d_ptr(new QuickWindowBorderPrivate(this))
{
}

QuickWindowBorder::~QuickWindowBorder() = default;

void QuickWindowBorder::paint(QPainter *painter)
{
    Q_D(QuickWindowBorder);
    d->paint(painter);
}

qreal QuickWindowBorder::thickness() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? 0 : d->m_borderPainter->thickness());
}

QuickGlobal::WindowEdges QuickWindowBorder::edges() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QuickGlobal::WindowEdges()
        : edgesToQuickEdges(d->m_borderPainter->edges()));
}

QColor QuickWindowBorder::activeColor() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QColor() : d->m_borderPainter->activeColor());
}

QColor QuickWindowBorder::inactiveColor() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QColor() : d->m_borderPainter->inactiveColor());
}

qreal QuickWindowBorder::nativeThickness() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? 0 : d->m_borderPainter->nativeThickness());
}

QuickGlobal::WindowEdges QuickWindowBorder::nativeEdges() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QuickGlobal::WindowEdges()
        : edgesToQuickEdges(d->m_borderPainter->nativeEdges()));
}

QColor QuickWindowBorder::nativeActiveColor() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QColor() : d->m_borderPainter->nativeActiveColor());
}

QColor QuickWindowBorder::nativeInactiveColor() const
{
    Q_D(const QuickWindowBorder);
    return (d->m_borderPainter.isNull() ? QColor() : d->m_borderPainter->nativeInactiveColor());
}

void QuickWindowBorder::setThickness(const qreal value)
{
    Q_D(QuickWindowBorder);
    if (d->m_borderPainter.isNull()) {
        return;
    }
    if (qFuzzyCompare(thickness(), value)) {
        return;
    }
    d->m_borderPainter->setThickness(qRound(value));
}

void QuickWindowBorder::setEdges(const QuickGlobal::WindowEdges value)
{
    Q_D(QuickWindowBorder);
    if (d->m_borderPainter.isNull()) {
        return;
    }
    if (edges() == value) {
        return;
    }
    d->m_borderPainter->setEdges(quickEdgesToEdges(value));
}

void QuickWindowBorder::setActiveColor(const QColor &value)
{
    Q_D(QuickWindowBorder);
    if (d->m_borderPainter.isNull()) {
        return;
    }
    if (activeColor() == value) {
        return;
    }
    d->m_borderPainter->setActiveColor(value);
}

void QuickWindowBorder::setInactiveColor(const QColor &value)
{
    Q_D(QuickWindowBorder);
    if (d->m_borderPainter.isNull()) {
        return;
    }
    if (inactiveColor() == value) {
        return;
    }
    d->m_borderPainter->setInactiveColor(value);
}

void QuickWindowBorder::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickPaintedItem::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        Q_D(QuickWindowBorder);
        d->rebindWindow();
    }
}

void QuickWindowBorder::classBegin()
{
    QQuickPaintedItem::classBegin();
}

void QuickWindowBorder::componentComplete()
{
    QQuickPaintedItem::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE
