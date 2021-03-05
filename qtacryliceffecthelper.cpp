/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "qtacryliceffecthelper.h"
#include "utilities.h"
#include <QtGui/qpainter.h>
#include <QtCore/qdebug.h>
#include <QtGui/qwindow.h>
#include <QtCore/qcoreapplication.h>

QtAcrylicEffectHelper::QtAcrylicEffectHelper()
{
    Q_INIT_RESOURCE(qtacrylichelper);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#ifdef Q_OS_MACOS
    if (Utilities::isAcrylicEffectSupported()) {
        m_tintOpacity = 0.6;
    }
#endif
    m_frameColor = Utilities::getNativeWindowFrameColor(true);
}

void QtAcrylicEffectHelper::install(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (m_window != window) {
        m_window = const_cast<QWindow *>(window);
    }
}

void QtAcrylicEffectHelper::uninstall()
{
    if (m_window) {
        m_window = nullptr;
    }
}

void QtAcrylicEffectHelper::clearWallpaper()
{
    if (!m_bluredWallpaper.isNull()) {
        m_bluredWallpaper = {};
    }
}

QBrush QtAcrylicEffectHelper::getAcrylicBrush() const
{
    return m_acrylicBrush;
}

QColor QtAcrylicEffectHelper::getTintColor() const
{
    return m_tintColor;
}

qreal QtAcrylicEffectHelper::getTintOpacity() const
{
    return m_tintOpacity;
}

qreal QtAcrylicEffectHelper::getNoiseOpacity() const
{
    return m_noiseOpacity;
}

QPixmap QtAcrylicEffectHelper::getBluredWallpaper() const
{
    return m_bluredWallpaper;
}

QColor QtAcrylicEffectHelper::getFrameColor() const
{
    return m_frameColor;
}

void QtAcrylicEffectHelper::setTintColor(const QColor &value)
{
    m_tintColor = value;
}

void QtAcrylicEffectHelper::setTintOpacity(qreal value)
{
    m_tintOpacity = value;
}

void QtAcrylicEffectHelper::setNoiseOpacity(qreal value)
{
    m_noiseOpacity = value;
}

void QtAcrylicEffectHelper::setFrameColor(const QColor &value)
{
    m_frameColor = value;
}

QtAcrylicEffectHelper::~QtAcrylicEffectHelper() = default;

void QtAcrylicEffectHelper::paintWindowBackground(QPainter *painter, const QRegion &clip)
{
    Q_ASSERT(painter);
    Q_ASSERT(!clip.isEmpty());
    if (!painter || clip.isEmpty() || !m_window) {
        return;
    }
    painter->save();
    painter->setClipRegion(clip);
    paintBackground(painter, clip.boundingRect());
}

void QtAcrylicEffectHelper::paintWindowBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid() || !m_window) {
        return;
    }
    painter->save();
    painter->setClipRegion({rect});
    paintBackground(painter, rect);
}

void QtAcrylicEffectHelper::paintBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid() || !m_window) {
        return;
    }
    if (Utilities::isAcrylicEffectSupported()) {
        const QPainter::CompositionMode mode = painter->compositionMode();
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(rect, Qt::white);
        painter->setCompositionMode(mode);
    } else {
        // Emulate blur behind window by blurring the desktop wallpaper.
        updateBehindWindowBackground();
        painter->drawPixmap(QPoint{0, 0}, m_bluredWallpaper, rect);
    }
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(1);
    painter->fillRect(rect, m_acrylicBrush);
    painter->restore();
}

void QtAcrylicEffectHelper::paintWindowFrame(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    if (!painter || !m_window) {
        return;
    }
    if (m_window->windowState() != Qt::WindowNoState) {
        // We shouldn't draw the window frame when it's minimized/maximized/fullscreen.
        return;
    }
    const int width = rect.isValid() ? rect.width() : m_window->width();
    const int height = rect.isValid() ? rect.height() : m_window->height();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QList<QLine> lines = {
#else
    const QVector<QLine> lines = {
#endif
        {0, 0, width, 0},
        {width - 1, 0, width - 1, height},
        {width, height - 1, 0, height - 1},
        {0, height, 0, 0}
    };
    const bool active = m_window->isActive();
    const QColor color = (active && m_frameColor.isValid() && (m_frameColor != Qt::transparent)) ? m_frameColor : Utilities::getNativeWindowFrameColor(active);
    painter->save();
    painter->setPen({color, 1});
    painter->drawLines(lines);
    painter->restore();
}

void QtAcrylicEffectHelper::updateAcrylicBrush(const QColor &alternativeTintColor)
{
    const auto getAppropriateTintColor = [&alternativeTintColor, this]() -> QColor {
        if (alternativeTintColor.isValid() && (alternativeTintColor != Qt::transparent)) {
            return alternativeTintColor;
        }
        if (m_tintColor.isValid() && (m_tintColor != Qt::transparent)) {
            return m_tintColor;
        }
        return Qt::white;
    };
    static const QImage noiseTexture(QStringLiteral(":/QtAcrylicHelper/Noise.png"));
    QImage acrylicTexture({64, 64}, QImage::Format_ARGB32_Premultiplied);
    QColor fillColor = Qt::transparent;
#ifdef Q_OS_WINDOWS
    if (!Utilities::isMSWin10AcrylicEffectAvailable()) {
        // Add a soft light layer for the background.
        fillColor = Qt::white;
        fillColor.setAlpha(150);
    }
#endif
    acrylicTexture.fill(fillColor);
    QPainter painter(&acrylicTexture);
    painter.setOpacity(m_tintOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, getAppropriateTintColor());
    painter.setOpacity(m_noiseOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, noiseTexture);
    m_acrylicBrush = acrylicTexture;
}

void QtAcrylicEffectHelper::updateBehindWindowBackground()
{
    if (!m_bluredWallpaper.isNull()) {
        return;
    }
    const QSize size = Utilities::getScreenAvailableGeometry().size();
    m_bluredWallpaper = QPixmap(size);
    m_bluredWallpaper.fill(Qt::transparent);
    QImage image = Utilities::getDesktopWallpaperImage();
    // On some platforms we may not be able to get the desktop wallpaper, such as Linux and WebAssembly.
    if (image.isNull()) {
        return;
    }
    const Utilities::DesktopWallpaperAspectStyle aspectStyle = Utilities::getDesktopWallpaperAspectStyle();
    QImage buffer(size, QImage::Format_ARGB32_Premultiplied);
#ifdef Q_OS_WINDOWS
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::Central) {
        buffer.fill(Qt::black);
    }
#endif
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatio ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatio ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioByExpanding) {
        Qt::AspectRatioMode mode = Qt::KeepAspectRatioByExpanding;
        if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatio) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatio) {
            mode = Qt::KeepAspectRatio;
        }
        QSize newSize = image.size();
        newSize.scale(size, mode);
        image = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::Tiled) {
        QPainter painterBuffer(&buffer);
        painterBuffer.fillRect(QRect{{0, 0}, size}, image);
    } else {
        QPainter painterBuffer(&buffer);
        const QRect rect = Utilities::alignedRect(Qt::LeftToRight, Qt::AlignCenter, image.size(), {{0, 0}, size});
        painterBuffer.drawImage(rect.topLeft(), image);
    }
    QPainter painter(&m_bluredWallpaper);
#if 1
    Utilities::blurImage(&painter, buffer, 128, false, false);
#else
    painter.drawImage(QPoint{0, 0}, buffer);
#endif
}
