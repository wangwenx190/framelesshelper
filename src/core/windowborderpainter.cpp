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

#include "windowborderpainter.h"
#include "windowborderpainter_p.h"
#include "utils.h"
#include "framelessmanager.h"
#ifdef Q_OS_WINDOWS
#  include "winverhelper_p.h"
#endif
#include <QtGui/qpainter.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcWindowBorderPainter, "wangwenx190.framelesshelper.core.windowborderpainter")
#define INFO qCInfo(lcWindowBorderPainter)
#define DEBUG qCDebug(lcWindowBorderPainter)
#define WARNING qCWarning(lcWindowBorderPainter)
#define CRITICAL qCCritical(lcWindowBorderPainter)

using namespace Global;

static constexpr const int kMaximumBorderThickness = 100;

WindowBorderPainterPrivate::WindowBorderPainterPrivate(WindowBorderPainter *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

WindowBorderPainterPrivate::~WindowBorderPainterPrivate() = default;

WindowBorderPainterPrivate *WindowBorderPainterPrivate::get(WindowBorderPainter *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

const WindowBorderPainterPrivate *WindowBorderPainterPrivate::get(const WindowBorderPainter *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

int WindowBorderPainterPrivate::getNativeBorderThickness()
{
    // Qt will scale it to the appropriate value for us automatically,
    // based on the current system DPI and scale factor rounding policy.
    return kDefaultWindowFrameBorderThickness;
}

QColor WindowBorderPainterPrivate::getNativeBorderColor(const bool active)
{
    return Utils::getFrameBorderColor(active);
}

WindowEdges WindowBorderPainterPrivate::getNativeBorderEdges()
{
#ifdef Q_OS_WINDOWS
    if (Utils::isWindowFrameBorderVisible() && !WindowsVersionHelper::isWin11OrGreater()) {
        return {WindowEdge::Top};
    }
#endif
    return {};
}

void WindowBorderPainterPrivate::paint(QPainter *painter, const QSize &size, const bool active) const
{
    Q_ASSERT(painter);
    Q_ASSERT(!size.isEmpty());
    if (!painter || size.isEmpty()) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QList<QLine> lines = {};
#else
    QVector<QLine> lines = {};
#endif
    const QPoint leftTop = {0, 0};
    // In fact, we should use "size.width() - 1" here in theory but we can't
    // because Qt's drawing system has some rounding errors internally and if
    // we minus one here we'll get a one pixel gap, so sad. But drawing a line
    // with a little extra pixels won't hurt anyway.
    const QPoint rightTop = {size.width(), 0};
    // Same here as above: we should use "size.height() - 1" ideally but we
    // can't, sadly.
    const QPoint rightBottom = {size.width(), size.height()};
    const QPoint leftBottom = {0, size.height()};
    const WindowEdges edges = m_edges.value_or(getNativeBorderEdges());
    if (edges & WindowEdge::Left) {
        lines.append({leftBottom, leftTop});
    }
    if (edges & WindowEdge::Top) {
        lines.append({leftTop, rightTop});
    }
    if (edges & WindowEdge::Right) {
        lines.append({rightTop, rightBottom});
    }
    if (edges & WindowEdge::Bottom) {
        lines.append({rightBottom, leftBottom});
    }
    if (lines.isEmpty()) {
        return;
    }
    painter->save();
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // We can't enable antialiasing here, because the border is too thin and antialiasing
    // will break it's painting.
    painter->setRenderHint(QPainter::Antialiasing, false);
    QPen pen = {};
    pen.setColor([active, this]() -> QColor {
        QColor color = {};
        if (active) {
            color = m_activeColor.value_or(getNativeBorderColor(true));
        } else {
            color = m_inactiveColor.value_or(getNativeBorderColor(false));
        }
        if (color.isValid()) {
            return color;
        }
        return (active ? kDefaultBlackColor : kDefaultDarkGrayColor);
    }());
    pen.setWidth(m_thickness.value_or(getNativeBorderThickness()));
    painter->setPen(pen);
    painter->drawLines(lines);
    painter->restore();
}

void WindowBorderPainterPrivate::initialize()
{
    Q_Q(WindowBorderPainter);
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged,
        q, &WindowBorderPainter::nativeBorderChanged);
    connect(q, &WindowBorderPainter::nativeBorderChanged, q, &WindowBorderPainter::shouldRepaint);
}

WindowBorderPainter::WindowBorderPainter(QObject *parent)
    : QObject(parent), d_ptr(new WindowBorderPainterPrivate(this))
{
}

WindowBorderPainter::~WindowBorderPainter() = default;

int WindowBorderPainter::thickness() const
{
    Q_D(const WindowBorderPainter);
    return d->m_thickness.value_or(d->getNativeBorderThickness());
}

WindowEdges WindowBorderPainter::edges() const
{
    Q_D(const WindowBorderPainter);
    return d->m_edges.value_or(d->getNativeBorderEdges());
}

QColor WindowBorderPainter::activeColor() const
{
    Q_D(const WindowBorderPainter);
    return d->m_activeColor.value_or(d->getNativeBorderColor(true));
}

QColor WindowBorderPainter::inactiveColor() const
{
    Q_D(const WindowBorderPainter);
    return d->m_inactiveColor.value_or(d->getNativeBorderColor(false));
}

int WindowBorderPainter::nativeThickness() const
{
    Q_D(const WindowBorderPainter);
    return d->getNativeBorderThickness();
}

WindowEdges WindowBorderPainter::nativeEdges() const
{
    Q_D(const WindowBorderPainter);
    return d->getNativeBorderEdges();
}

QColor WindowBorderPainter::nativeActiveColor() const
{
    Q_D(const WindowBorderPainter);
    return d->getNativeBorderColor(true);
}

QColor WindowBorderPainter::nativeInactiveColor() const
{
    Q_D(const WindowBorderPainter);
    return d->getNativeBorderColor(false);
}

void WindowBorderPainter::paint(QPainter *painter, const QSize &size, const bool active) const
{
    Q_D(const WindowBorderPainter);
    d->paint(painter, size, active);
}

void WindowBorderPainter::setThickness(const int value)
{
    Q_ASSERT(value >= 0);
    Q_ASSERT(value < kMaximumBorderThickness);
    if ((value < 0) || (value >= kMaximumBorderThickness)) {
        return;
    }
    if (thickness() == value) {
        return;
    }
    Q_D(WindowBorderPainter);
    d->m_thickness = value;
    Q_EMIT thicknessChanged();
    Q_EMIT shouldRepaint();
}

void WindowBorderPainter::setEdges(const Global::WindowEdges value)
{
    if (edges() == value) {
        return;
    }
    Q_D(WindowBorderPainter);
    d->m_edges = value;
    Q_EMIT edgesChanged();
    Q_EMIT shouldRepaint();
}

void WindowBorderPainter::setActiveColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (activeColor() == value) {
        return;
    }
    Q_D(WindowBorderPainter);
    d->m_activeColor = value;
    Q_EMIT activeColorChanged();
    Q_EMIT shouldRepaint();
}

void WindowBorderPainter::setInactiveColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (inactiveColor() == value) {
        return;
    }
    Q_D(WindowBorderPainter);
    d->m_inactiveColor = value;
    Q_EMIT inactiveColorChanged();
    Q_EMIT shouldRepaint();
}

FRAMELESSHELPER_END_NAMESPACE
