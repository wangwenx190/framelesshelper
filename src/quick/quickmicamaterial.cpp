/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#include "quickmicamaterial.h"
#include "quickmicamaterial_p.h"
#include <FramelessHelper/Core/micamaterial.h>
#include <QtCore/qloggingcategory.h>
#include <QtQuick/qquickwindow.h>
#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
#  include <QtQuick/private/qquickitem_p.h>
#  include <QtQuick/private/qquickanchors_p.h>
#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcQuickMicaMaterial, "wangwenx190.framelesshelper.quick.quickmicamaterial")

#ifdef FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcQuickMicaMaterial)
#  define DEBUG qCDebug(lcQuickMicaMaterial)
#  define WARNING qCWarning(lcQuickMicaMaterial)
#  define CRITICAL qCCritical(lcQuickMicaMaterial)
#endif

using namespace Global;

QuickMicaMaterialPrivate::QuickMicaMaterialPrivate(QuickMicaMaterial *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

QuickMicaMaterialPrivate::~QuickMicaMaterialPrivate() = default;

QuickMicaMaterialPrivate *QuickMicaMaterialPrivate::get(QuickMicaMaterial *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

const QuickMicaMaterialPrivate *QuickMicaMaterialPrivate::get(const QuickMicaMaterial *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

void QuickMicaMaterialPrivate::initialize()
{
    Q_Q(QuickMicaMaterial);

    // No smooth needed. The blurry image is already low quality, enabling
    // smooth won't help much and we also don't want it to slow down the
    // general performance.
    q->setSmooth(false);
    // We don't need anti-aliasing. Same reason as above.
    q->setAntialiasing(false);
    // Enable clipping, to improve performance in some certain cases.
    q->setClip(true);
    // Disable mipmap, we don't need high quality scaling here.
    q->setMipmap(false);
    // Mica material should not be translucent anyway, enabling this option
    // will disable the alpha blending of this item, which can also improve
    // the rendering performance.
    q->setOpaquePainting(true);
    // Set an invalid fill color to prevent QQuickPaintedItem from drawing the background,
    // we don't need it anyway and it can improve the general performance as well.
    q->setFillColor(QColor{});

    m_micaMaterial = new MicaMaterial(this);
    connect(m_micaMaterial, &MicaMaterial::tintColorChanged, q, &QuickMicaMaterial::tintColorChanged);
    connect(m_micaMaterial, &MicaMaterial::tintOpacityChanged, q, &QuickMicaMaterial::tintOpacityChanged);
    connect(m_micaMaterial, &MicaMaterial::fallbackColorChanged, q, &QuickMicaMaterial::fallbackColorChanged);
    connect(m_micaMaterial, &MicaMaterial::noiseOpacityChanged, q, &QuickMicaMaterial::noiseOpacityChanged);
    connect(m_micaMaterial, &MicaMaterial::fallbackEnabledChanged, q, &QuickMicaMaterial::fallbackEnabledChanged);
    connect(m_micaMaterial, &MicaMaterial::shouldRedraw, q, [q](){ q->update(); });
}

void QuickMicaMaterialPrivate::rebindWindow()
{
    Q_Q(QuickMicaMaterial);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    QQuickItem * const rootItem = window->contentItem();
    q->setParent(rootItem);
    q->setParentItem(rootItem);
#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
    QQuickItemPrivate::get(q)->anchors()->setFill(rootItem);
#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE
    q->setZ(-999); // Make sure we always stays on the bottom most place.
    if (m_rootWindowXChangedConnection) {
        disconnect(m_rootWindowXChangedConnection);
        m_rootWindowXChangedConnection = {};
    }
    if (m_rootWindowYChangedConnection) {
        disconnect(m_rootWindowYChangedConnection);
        m_rootWindowYChangedConnection = {};
    }
    if (m_rootWindowActiveChangedConnection) {
        disconnect(m_rootWindowActiveChangedConnection);
        m_rootWindowActiveChangedConnection = {};
    }
    m_rootWindowXChangedConnection = connect(window, &QQuickWindow::xChanged, q, [q](){ q->update(); });
    m_rootWindowYChangedConnection = connect(window, &QQuickWindow::yChanged, q, [q](){ q->update(); });
    m_rootWindowActiveChangedConnection = connect(window, &QQuickWindow::activeChanged, q, [q](){ q->update(); });
}

void QuickMicaMaterialPrivate::repaint(QPainter *painter)
{
    Q_ASSERT(painter);
    Q_ASSERT(m_micaMaterial);
    if (!painter || !m_micaMaterial) {
        return;
    }
    Q_Q(QuickMicaMaterial);
    const bool isActive = q->window() ? q->window()->isActive() : false;
    const QPoint originPoint = q->mapToGlobal(QPointF{ 0, 0 }).toPoint();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const QSize size = q->size().toSize();
#else // (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    const QSize size = QSizeF{ q->width(), q->height() }.toSize();
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    m_micaMaterial->paint(painter, QRect{ originPoint, size }, isActive);
}

QuickMicaMaterial::QuickMicaMaterial(QQuickItem *parent)
    : QQuickPaintedItem(parent), d_ptr(new QuickMicaMaterialPrivate(this))
{
}

QuickMicaMaterial::~QuickMicaMaterial() = default;

void QuickMicaMaterial::paint(QPainter *painter)
{
    Q_ASSERT(painter);
    if (!painter) {
        return;
    }
    Q_D(QuickMicaMaterial);
    d->repaint(painter);
}

QColor QuickMicaMaterial::tintColor() const
{
    Q_D(const QuickMicaMaterial);
    return d->m_micaMaterial->tintColor();
}

void QuickMicaMaterial::setTintColor(const QColor &value)
{
    Q_D(QuickMicaMaterial);
    d->m_micaMaterial->setTintColor(value);
}

qreal QuickMicaMaterial::tintOpacity() const
{
    Q_D(const QuickMicaMaterial);
    return d->m_micaMaterial->tintOpacity();
}

void QuickMicaMaterial::setTintOpacity(const qreal value)
{
    Q_D(QuickMicaMaterial);
    d->m_micaMaterial->setTintOpacity(value);
}

QColor QuickMicaMaterial::fallbackColor() const
{
    Q_D(const QuickMicaMaterial);
    return d->m_micaMaterial->fallbackColor();
}

void QuickMicaMaterial::setFallbackColor(const QColor &value)
{
    Q_D(QuickMicaMaterial);
    d->m_micaMaterial->setFallbackColor(value);
}

qreal QuickMicaMaterial::noiseOpacity() const
{
    Q_D(const QuickMicaMaterial);
    return d->m_micaMaterial->noiseOpacity();
}

void QuickMicaMaterial::setNoiseOpacity(const qreal value)
{
    Q_D(QuickMicaMaterial);
    d->m_micaMaterial->setNoiseOpacity(value);
}

bool QuickMicaMaterial::isFallbackEnabled() const
{
    Q_D(const QuickMicaMaterial);
    return d->m_micaMaterial->isFallbackEnabled();
}

void QuickMicaMaterial::setFallbackEnabled(const bool value)
{
    Q_D(QuickMicaMaterial);
    d->m_micaMaterial->setFallbackEnabled(value);
}

void QuickMicaMaterial::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickPaintedItem::itemChange(change, value);
    Q_D(QuickMicaMaterial);
    switch (change) {
    case ItemDevicePixelRatioHasChanged: {
        update(); // Force re-paint immediately.
    } break;
    case ItemSceneChange: {
        if (value.window) {
            d->rebindWindow();
        }
    } break;
    default:
        break;
    }
}

void QuickMicaMaterial::classBegin()
{
    QQuickPaintedItem::classBegin();
}

void QuickMicaMaterial::componentComplete()
{
    QQuickPaintedItem::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE
