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

QtAcrylicEffectHelper::QtAcrylicEffectHelper(QObject *parent) : QObject(parent)
{
    Q_INIT_RESOURCE(qtacrylichelper);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#ifdef Q_OS_MACOS
    if (Utilities::shouldUseTraditionalBlur()) {
        m_tintOpacity = 0.6;
    }
#endif
#ifdef Q_OS_WINDOWS
    m_frameColor = Utilities::getNativeWindowFrameColor(true);
#else
    m_frameColor = Qt::black;
#endif
}

QtAcrylicEffectHelper::~QtAcrylicEffectHelper() = default;

void QtAcrylicEffectHelper::install(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (m_window != window) {
        m_window = const_cast<QWindow *>(window);
        connect(m_window, &QWindow::xChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        connect(m_window, &QWindow::yChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        connect(m_window, &QWindow::activeChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        // What's the difference between "visibility" and "window state"?
        //connect(m_window, &QWindow::visibilityChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        connect(m_window, &QWindow::windowStateChanged, this, &QtAcrylicEffectHelper::needsRepaint);
#ifdef Q_OS_WINDOWS
        //QtAcrylicWinEventFilter::setup();
#endif
    }
}

void QtAcrylicEffectHelper::uninstall()
{
    if (m_window) {
#ifdef Q_OS_WINDOWS
        //QtAcrylicWinEventFilter::unsetup();
#endif
        disconnect(m_window, &QWindow::xChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        disconnect(m_window, &QWindow::yChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        disconnect(m_window, &QWindow::activeChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        //disconnect(m_window, &QWindow::visibilityChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        disconnect(m_window, &QWindow::windowStateChanged, this, &QtAcrylicEffectHelper::needsRepaint);
        m_window = nullptr;
    }
}

void QtAcrylicEffectHelper::clearWallpaper()
{
    if (!m_bluredWallpaper.isNull()) {
        m_bluredWallpaper = {};
    }
}

void QtAcrylicEffectHelper::showWarning() const
{
    qDebug() << "The Acrylic blur effect has been enabled. Rendering acrylic material surfaces is highly GPU-intensive, which can slow down the application, increase the power consumption on the devices on which the application is running.";
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

qreal QtAcrylicEffectHelper::getFrameThickness() const
{
    return m_frameThickness;
}

void QtAcrylicEffectHelper::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << value << "is not a valid color.";
        return;
    }
    if (m_tintColor != value) {
        m_tintColor = value;
    }
}

void QtAcrylicEffectHelper::setTintOpacity(const qreal value)
{
    if (m_tintOpacity != value) {
        m_tintOpacity = value;
    }
}

void QtAcrylicEffectHelper::setNoiseOpacity(const qreal value)
{
    if (m_noiseOpacity != value) {
        m_noiseOpacity = value;
    }
}

void QtAcrylicEffectHelper::setFrameColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << value << "is not a valid color.";
        return;
    }
    if (m_frameColor != value) {
        m_frameColor = value;
    }
}

void QtAcrylicEffectHelper::setFrameThickness(const qreal value)
{
    if (m_frameThickness != value) {
        m_frameThickness = value;
    }
}

void QtAcrylicEffectHelper::paintWindowBackground(QPainter *painter, const QRegion &clip)
{
    Q_ASSERT(painter);
    Q_ASSERT(!clip.isEmpty());
    if (!painter || clip.isEmpty()) {
        return;
    }
    if (!checkWindow()) {
        return;
    }
    painter->save();
    painter->setClipRegion(clip);
    paintBackground(painter, clip.boundingRect());
    painter->restore();
}

void QtAcrylicEffectHelper::paintWindowBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid()) {
        return;
    }
    if (!checkWindow()) {
        return;
    }
    painter->save();
    painter->setClipRegion({rect});
    paintBackground(painter, rect);
    painter->restore();
}

void QtAcrylicEffectHelper::paintBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid()) {
        return;
    }
    if (!checkWindow()) {
        return;
    }
    // TODO: should we limit it to Win32 only? Or should we do something about the
    // acrylic brush instead?
    if (Utilities::disableExtraProcessingForBlur()) {
        return;
    }
    if (Utilities::shouldUseTraditionalBlur()) {
        const QPainter::CompositionMode mode = painter->compositionMode();
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(rect, Qt::white);
        painter->setCompositionMode(mode);
    } else {
        // Emulate blur behind window by blurring the desktop wallpaper.
        updateBehindWindowBackground();
        painter->drawPixmap(QPoint{0, 0}, m_bluredWallpaper, QRect{m_window->mapToGlobal(QPoint{0, 0}), rect.size()});
    }
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(1);
    painter->fillRect(rect, m_acrylicBrush);
}

void QtAcrylicEffectHelper::paintWindowFrame(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    if (!painter) {
        return;
    }
    if (!checkWindow()) {
        return;
    }
    if (m_window->windowState() != Qt::WindowNoState) {
        // We shouldn't draw the window frame when it's minimized/maximized/fullscreen.
        return;
    }
    const int width = rect.isValid() ? rect.width() : m_window->width();
    const int height = rect.isValid() ? rect.height() : m_window->height();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QList<QLineF> lines = {
#else
    const QVector<QLineF> lines = {
#endif
        {0, 0, static_cast<qreal>(width), 0},
        {width - m_frameThickness, 0, width - m_frameThickness, static_cast<qreal>(height)},
        {static_cast<qreal>(width), height - m_frameThickness, 0, height - m_frameThickness},
        {0, static_cast<qreal>(height), 0, 0}
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
    if (!Utilities::isOfficialMSWin10AcrylicBlurAvailable()) {
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
    if (!checkWindow()) {
        return;
    }
    if (!m_bluredWallpaper.isNull()) {
        return;
    }
    const QSize size = Utilities::getScreenGeometry(m_window).size();
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
    if ((aspectStyle == Utilities::DesktopWallpaperAspectStyle::Central) ||
            (aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit)) {
        buffer.fill(Utilities::getDesktopBackgroundColor());
    }
#endif
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioByExpanding) {
        Qt::AspectRatioMode mode;
        if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit) {
            mode = Qt::KeepAspectRatio;
        } else {
            mode = Qt::KeepAspectRatioByExpanding;
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

bool QtAcrylicEffectHelper::checkWindow() const
{
    if (m_window) {
        return true;
    }
    qWarning() << "m_window is null, forgot to call \"QtAcrylicEffectHelper::install()\"?";
    return false;
}
