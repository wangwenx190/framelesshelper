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
#include <FramelessHelper/Core/framelessmanager.h>
#include <FramelessHelper/Core/private/micamaterial_p.h>
#include <QtCore/qloggingcategory.h>
#include <QtGui/qpainter.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgsimpletexturenode.h>
#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
#  include <QtQuick/private/qquickitem_p.h>
#  include <QtQuick/private/qquickrectangle_p.h>
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

class WallpaperImageNode : public QObject, public QSGTransformNode
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WallpaperImageNode)

public:
    explicit WallpaperImageNode(QuickMicaMaterial *item);
    ~WallpaperImageNode() override;

public Q_SLOTS:
    void maybeUpdateWallpaperImageClipRect();
    void maybeGenerateWallpaperImageCache(const bool force = false);

private:
    void initialize();

private:
    QPointer<QuickMicaMaterial> m_item = nullptr;
    QSGSimpleTextureNode *m_node = nullptr;
    std::unique_ptr<QSGTexture> m_texture = nullptr;
    QPointer<MicaMaterial> m_mica{ nullptr };
    QPointer<MicaMaterialPrivate> m_micaPriv{ nullptr };
    QPointer<QQuickWindow> m_window{ nullptr };
};

WallpaperImageNode::WallpaperImageNode(QuickMicaMaterial *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    m_item = item;
    initialize();
}

WallpaperImageNode::~WallpaperImageNode()
{
    QuickMicaMaterialPrivate::get(m_item)->removeNode(this);
}

void WallpaperImageNode::initialize()
{
    m_window = m_item->window();
    m_mica = QuickMicaMaterialPrivate::get(m_item)->m_micaMaterial;
    m_micaPriv = MicaMaterialPrivate::get(m_mica);

    // QtQuick's render engine will free it when appropriate.
    m_node = new QSGSimpleTextureNode;
    m_node->setFiltering(QSGTexture::Linear);

    maybeGenerateWallpaperImageCache();
    maybeUpdateWallpaperImageClipRect();

    appendChildNode(m_node);

    connect(m_window, &QQuickWindow::beforeRendering, this,
        &WallpaperImageNode::maybeUpdateWallpaperImageClipRect, Qt::DirectConnection);

    QuickMicaMaterialPrivate::get(m_item)->appendNode(this);
}

void WallpaperImageNode::maybeGenerateWallpaperImageCache(const bool force)
{
    if (m_texture && !force) {
        return;
    }
    static constexpr const auto originPoint = QPoint{ 0, 0 };
    const QSize imageSize = MicaMaterialPrivate::wallpaperSize();
    auto pixmap = QPixmap(imageSize);
    pixmap.fill(kDefaultTransparentColor);
    QPainter painter(&pixmap);
    // We need the real wallpaper image here, so always use "active" state.
    m_mica->paint(&painter, QRect{ originPoint, imageSize }, true);
    m_texture.reset(m_window->createTextureFromImage(pixmap.toImage()));
    m_node->setTexture(m_texture.get());
}

void WallpaperImageNode::maybeUpdateWallpaperImageClipRect()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const QSizeF itemSize = m_item->size();
#else
    const QSizeF itemSize = {m_item->width(), m_item->height()};
#endif
    m_node->setRect(QRectF(QPointF(0.0, 0.0), itemSize));
    const auto rect = QRectF(m_item->mapToGlobal(QPointF(0.0, 0.0)), itemSize);
    m_node->setSourceRect(m_micaPriv->mapToWallpaper(rect.toRect()));
}

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

    q->setFlag(QuickMicaMaterial::ItemHasContents);
    q->setSmooth(true);
    q->setAntialiasing(true);
    q->setClip(true);

    m_micaMaterial = new MicaMaterial(this);
    connect(m_micaMaterial, &MicaMaterial::tintColorChanged, q, &QuickMicaMaterial::tintColorChanged);
    connect(m_micaMaterial, &MicaMaterial::tintOpacityChanged, q, &QuickMicaMaterial::tintOpacityChanged);
    connect(m_micaMaterial, &MicaMaterial::fallbackColorChanged, q, &QuickMicaMaterial::fallbackColorChanged);
    connect(m_micaMaterial, &MicaMaterial::noiseOpacityChanged, q, &QuickMicaMaterial::noiseOpacityChanged);
    connect(m_micaMaterial, &MicaMaterial::fallbackEnabledChanged, q, &QuickMicaMaterial::fallbackEnabledChanged);
    connect(m_micaMaterial, &MicaMaterial::shouldRedraw, this, &QuickMicaMaterialPrivate::forceRegenerateWallpaperImageCache);

#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
    m_fallbackColorItem = new QQuickRectangle(q);
    QQuickItemPrivate::get(m_fallbackColorItem)->anchors()->setFill(q);
    QQuickPen * const border = m_fallbackColorItem->border();
    border->setColor(kDefaultTransparentColor);
    border->setWidth(0);
    updateFallbackColor();
    m_fallbackColorItem->setVisible(false);
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged, this, &QuickMicaMaterialPrivate::updateFallbackColor);
#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE
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
    m_rootWindowXChangedConnection = connect(window, &QQuickWindow::xChanged, q, [q](){ q->update(); });
    m_rootWindowYChangedConnection = connect(window, &QQuickWindow::yChanged, q, [q](){ q->update(); });
#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
    if (m_rootWindowActiveChangedConnection) {
        disconnect(m_rootWindowActiveChangedConnection);
        m_rootWindowActiveChangedConnection = {};
    }
    m_rootWindowActiveChangedConnection = connect(window, &QQuickWindow::activeChanged, q, [this, window](){
        if (m_micaMaterial->isFallbackEnabled()) {
            m_fallbackColorItem->setVisible(!window->isActive());
        } else {
            m_fallbackColorItem->setVisible(false);
        }
    });
#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE
}

void QuickMicaMaterialPrivate::forceRegenerateWallpaperImageCache()
{
    if (m_nodes.isEmpty()) {
        return;
    }
    for (auto &&node : std::as_const(m_nodes)) {
        if (node) {
            node->maybeGenerateWallpaperImageCache(true);
        }
    }
}

void QuickMicaMaterialPrivate::appendNode(WallpaperImageNode *node)
{
    Q_ASSERT(node);
    if (!node) {
        return;
    }
    if (m_nodes.contains(node)) {
        return;
    }
    m_nodes.append(node);
}

void QuickMicaMaterialPrivate::removeNode(WallpaperImageNode *node)
{
    Q_ASSERT(node);
    if (!node) {
        return;
    }
    if (!m_nodes.contains(node)) {
        return;
    }
    m_nodes.removeAll(node);
}

void QuickMicaMaterialPrivate::updateFallbackColor()
{
#ifndef FRAMELESSHELPER_QUICK_NO_PRIVATE
    if (!m_fallbackColorItem || !m_micaMaterial) {
        return;
    }
    const QColor color = m_micaMaterial->fallbackColor();
    if (color.isValid()) {
        m_fallbackColorItem->setColor(color);
        return;
    }
    m_fallbackColorItem->setColor(MicaMaterialPrivate::systemFallbackColor());
#endif // FRAMELESSHELPER_QUICK_NO_PRIVATE
}

QuickMicaMaterial::QuickMicaMaterial(QQuickItem *parent)
    : QQuickItem(parent), d_ptr(new QuickMicaMaterialPrivate(this))
{
}

QuickMicaMaterial::~QuickMicaMaterial() = default;

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
    QQuickItem::itemChange(change, value);
    Q_D(QuickMicaMaterial);
    switch (change) {
    case ItemDevicePixelRatioHasChanged: {
        d->forceRegenerateWallpaperImageCache();
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

QSGNode *QuickMicaMaterial::updatePaintNode(QSGNode *old, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    auto node = static_cast<WallpaperImageNode *>(old);
    if (!node) {
        node = new WallpaperImageNode(this);
    }
    return node;
}

void QuickMicaMaterial::classBegin()
{
    QQuickItem::classBegin();
}

void QuickMicaMaterial::componentComplete()
{
    QQuickItem::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE

#include "quickmicamaterial.moc"
