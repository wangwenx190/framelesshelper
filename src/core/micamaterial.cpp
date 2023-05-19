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

#include "micamaterial.h"
#include "micamaterial_p.h"
#include "framelessmanager.h"
#include "utils.h"
#include "framelessconfig_p.h"
#include <QtCore/qsysinfo.h>
#include <QtCore/qloggingcategory.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#ifndef FRAMELESSHELPER_CORE_NO_PRIVATE
#  include <QtGui/private/qmemrotate_p.h>
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE

FRAMELESSHELPER_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lcMicaMaterial, "wangwenx190.framelesshelper.core.micamaterial")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcMicaMaterial)
#  define DEBUG qCDebug(lcMicaMaterial)
#  define WARNING qCWarning(lcMicaMaterial)
#  define CRITICAL qCCritical(lcMicaMaterial)
#endif

using namespace Global;

[[maybe_unused]] static constexpr const qreal kDefaultTintOpacity = 0.7;
[[maybe_unused]] static constexpr const qreal kDefaultNoiseOpacity = 0.04;
[[maybe_unused]] static constexpr const qreal kDefaultBlurRadius = 128.0;

[[maybe_unused]] static Q_COLOR_CONSTEXPR const QColor kDefaultSystemLightColor2 = {243, 243, 243}; // #F3F3F3

[[maybe_unused]] static Q_COLOR_CONSTEXPR const QColor kDefaultFallbackColorDark = {44, 44, 44}; // #2C2C2C
[[maybe_unused]] static Q_COLOR_CONSTEXPR const QColor kDefaultFallbackColorLight = {249, 249, 249}; // #F9F9F9

#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
FRAMELESSHELPER_STRING_CONSTANT2(NoiseImageFilePath, ":/org.wangwenx190.FramelessHelper/resources/images/noise.png")
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE

struct MicaMaterialData
{
    QPixmap blurredWallpaper = {};
    bool graphicsResourcesReady = false;
};

Q_GLOBAL_STATIC(MicaMaterialData, g_micaMaterialData)

#ifndef FRAMELESSHELPER_CORE_NO_PRIVATE
template<const int shift>
[[nodiscard]] static inline constexpr int qt_static_shift(const int value)
{
    if constexpr (shift == 0) {
        return value;
    } else if constexpr (shift > 0) {
        return (value << (quint32(shift) & 0x1f));
    } else {
        return (value >> (quint32(-shift) & 0x1f));
    }
}

template<const int aprec, const int zprec>
static inline void qt_blurinner(uchar *bptr, int &zR, int &zG, int &zB, int &zA, const int alpha)
{
    const auto pixel = reinterpret_cast<QRgb *>(bptr);

#define Z_MASK (0xff << zprec)
    const int A_zprec = (qt_static_shift<zprec - 24>(*pixel) & Z_MASK);
    const int R_zprec = (qt_static_shift<zprec - 16>(*pixel) & Z_MASK);
    const int G_zprec = (qt_static_shift<zprec - 8>(*pixel)  & Z_MASK);
    const int B_zprec = (qt_static_shift<zprec>(*pixel)      & Z_MASK);
#undef Z_MASK

    const int zR_zprec = (zR >> aprec);
    const int zG_zprec = (zG >> aprec);
    const int zB_zprec = (zB >> aprec);
    const int zA_zprec = (zA >> aprec);

    zR += (alpha * (R_zprec - zR_zprec));
    zG += (alpha * (G_zprec - zG_zprec));
    zB += (alpha * (B_zprec - zB_zprec));
    zA += (alpha * (A_zprec - zA_zprec));

#define ZA_MASK (0xff << (zprec + aprec))
    *pixel = (qt_static_shift<24 - zprec - aprec>(zA & ZA_MASK)
        | qt_static_shift<16 - zprec - aprec>(zR & ZA_MASK)
        | qt_static_shift<8 - zprec - aprec>(zG & ZA_MASK)
        | qt_static_shift<-zprec - aprec>(zB & ZA_MASK));
#undef ZA_MASK
}

static constexpr const int alphaIndex = ((QSysInfo::ByteOrder == QSysInfo::BigEndian) ? 0 : 3);

template<const int aprec, const int zprec>
static inline void qt_blurinner_alphaOnly(uchar *bptr, int &z, const int alpha)
{
    const int A_zprec = (int(*(bptr)) << zprec);
    const int z_zprec = (z >> aprec);
    z += (alpha * (A_zprec - z_zprec));
    *(bptr) = (z >> (zprec + aprec));
}

template<const int aprec, const int zprec, const bool alphaOnly>
static inline void qt_blurrow(QImage &im, const int line, const int alpha)
{
    uchar *bptr = im.scanLine(line);

    int zR = 0, zG = 0, zB = 0, zA = 0;

#ifdef Q_CC_MSVC
#  pragma warning(push)
#  pragma warning(disable:4127) // false alarm.
#endif // Q_CC_MSVC
    if (alphaOnly && (im.format() != QImage::Format_Indexed8)) {
        bptr += alphaIndex;
    }
#ifdef Q_CC_MSVC
#  pragma warning(pop)
#endif // Q_CC_MSVC

    const int stride = (im.depth() >> 3);
    const int im_width = im.width();
    for (int index = 0; index != im_width; ++index) {
        if (alphaOnly) {
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        } else {
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
        }
        bptr += stride;
    }

    bptr -= stride;

    for (int index = (im_width - 2); index >= 0; --index) {
        bptr -= stride;
        if (alphaOnly) {
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        } else {
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
        }
    }
}

/*
*  expblur(QImage &img, int radius)
*
*  Based on exponential blur algorithm by Jani Huhtanen
*
*  In-place blur of image 'img' with kernel
*  of approximate radius 'radius'.
*
*  Blurs with two sided exponential impulse
*  response.
*
*  aprec = precision of alpha parameter
*  in fixed-point format 0.aprec
*
*  zprec = precision of state parameters
*  zR,zG,zB and zA in fp format 8.zprec
*/
template<const int aprec, const int zprec, const bool alphaOnly>
static inline void expblur(QImage &img, qreal radius, const bool improvedQuality = false, const int transposed = 0)
{
    Q_ASSERT((img.format() == QImage::Format_ARGB32_Premultiplied)
             || (img.format() == QImage::Format_RGB32)
             || (img.format() == QImage::Format_Indexed8)
             || (img.format() == QImage::Format_Grayscale8));
    if ((img.format() != QImage::Format_ARGB32_Premultiplied)
        && (img.format() != QImage::Format_RGB32)
        && (img.format() != QImage::Format_Indexed8)
        && (img.format() != QImage::Format_Grayscale8)) {
        return;
    }

    // halve the radius if we're using two passes
    if (improvedQuality) {
        radius *= 0.5;
    }

    // choose the alpha such that pixels at radius distance from a fully
    // saturated pixel will have an alpha component of no greater than
    // the cutOffIntensity
    static constexpr const qreal cutOffIntensity = 2.0;
    const int alpha = ((radius <= qreal(1e-5)) ? ((1 << aprec) - 1) :
        std::round((1 << aprec) * (1 - qPow(cutOffIntensity / qreal(255), qreal(1) / radius))));

    int img_height = img.height();
    for (int row = 0; row != img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i) {
            qt_blurrow<aprec, zprec, alphaOnly>(img, row, alpha);
        }
    }

    QImage temp(img.height(), img.width(), img.format());
    temp.setDevicePixelRatio(img.devicePixelRatio());

    if (transposed >= 0) {
        if (img.depth() == 8) {
            qt_memrotate270(reinterpret_cast<const quint8 *>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint8 *>(temp.bits()),
                            temp.bytesPerLine());
        } else {
            qt_memrotate270(reinterpret_cast<const quint32 *>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint32 *>(temp.bits()),
                            temp.bytesPerLine());
        }
    } else {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8 *>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint8 *>(temp.bits()),
                           temp.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32 *>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint32 *>(temp.bits()),
                           temp.bytesPerLine());
        }
    }

    img_height = temp.height();
    for (int row = 0; row != img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i) {
            qt_blurrow<aprec, zprec, alphaOnly>(temp, row, alpha);
        }
    }

    if (transposed == 0) {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8 *>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint8 *>(img.bits()),
                           img.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32 *>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint32 *>(img.bits()),
                           img.bytesPerLine());
        }
    } else {
        img = temp;
    }
}

#define AVG(a,b)  ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )
#define AVG16(a,b)  ( ((((a)^(b)) & 0xf7deUL) >> 1) + ((a)&(b)) )

[[nodiscard]] static inline QImage qt_halfScaled(const QImage &source)
{
    if ((source.width() < 2) || (source.height() < 2)) {
        return {};
    }

    QImage srcImage = source;

    if ((source.format() == QImage::Format_Indexed8)
        || (source.format() == QImage::Format_Grayscale8)) {
        // assumes grayscale
        QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
        dest.setDevicePixelRatio(source.devicePixelRatio());

        auto src = reinterpret_cast<const uchar *>(const_cast<const QImage &>(srcImage).bits());
        const qsizetype sx = srcImage.bytesPerLine();
        const qsizetype sx2 = (sx << 1);

        auto dst = reinterpret_cast<uchar *>(dest.bits());
        const qsizetype dx = dest.bytesPerLine();
        const int ww = dest.width();
        const int hh = dest.height();

        for (int y = hh; y; --y, dst += dx, src += sx2) {
            const uchar *p1 = src;
            const uchar *p2 = (src + sx);
            uchar *q = dst;
            for (int x = ww; x; --x, ++q, p1 += 2, p2 += 2) {
                *q = (((int(p1[0]) + int(p1[1]) + int(p2[0]) + int(p2[1])) + 2) >> 2);
            }
        }

        return dest;
    }
    if (source.format() == QImage::Format_ARGB8565_Premultiplied) {
        QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
        dest.setDevicePixelRatio(source.devicePixelRatio());

        auto src = reinterpret_cast<const uchar *>(const_cast<const QImage &>(srcImage).bits());
        const qsizetype sx = srcImage.bytesPerLine();
        const qsizetype sx2 = (sx << 1);

        auto dst = reinterpret_cast<uchar *>(dest.bits());
        const qsizetype dx = dest.bytesPerLine();
        const int ww = dest.width();
        const int hh = dest.height();

        for (int y = hh; y; --y, dst += dx, src += sx2) {
            const uchar *p1 = src;
            const uchar *p2 = (src + sx);
            uchar *q = dst;
            for (int x = ww; x; --x, q += 3, p1 += 6, p2 += 6) {
                // alpha
                q[0] = AVG(AVG(p1[0], p1[3]), AVG(p2[0], p2[3]));
                // rgb
                const quint16 p16_1 = ((p1[2] << 8) | p1[1]);
                const quint16 p16_2 = ((p1[5] << 8) | p1[4]);
                const quint16 p16_3 = ((p2[2] << 8) | p2[1]);
                const quint16 p16_4 = ((p2[5] << 8) | p2[4]);
                const quint16 result = AVG16(AVG16(p16_1, p16_2), AVG16(p16_3, p16_4));
                q[1] = (result & 0xff);
                q[2] = (result >> 8);
            }
        }

        return dest;
    }
    if ((source.format() != QImage::Format_ARGB32_Premultiplied)
        && (source.format() != QImage::Format_RGB32)) {
        srcImage = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
    dest.setDevicePixelRatio(source.devicePixelRatio());

    auto src = reinterpret_cast<const quint32 *>(const_cast<const QImage &>(srcImage).bits());
    const qsizetype sx = (srcImage.bytesPerLine() >> 2);
    const qsizetype sx2 = (sx << 1);

    auto dst = reinterpret_cast<quint32 *>(dest.bits());
    const qsizetype dx = (dest.bytesPerLine() >> 2);
    const int ww = dest.width();
    const int hh = dest.height();

    for (int y = hh; y; --y, dst += dx, src += sx2) {
        const quint32 *p1 = src;
        const quint32 *p2 = (src + sx);
        quint32 *q = dst;
        for (int x = ww; x; --x, q++, p1 += 2, p2 += 2) {
            *q = AVG(AVG(p1[0], p1[1]), AVG(p2[0], p2[1]));
        }
    }

    return dest;
}

[[maybe_unused]] static inline void qt_blurImage(QPainter *p, QImage &blurImage,
    qreal radius, const bool quality, const bool alphaOnly, const int transposed = 0)
{
    if ((blurImage.format() != QImage::Format_ARGB32_Premultiplied)
        && (blurImage.format() != QImage::Format_RGB32)) {
        blurImage = blurImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    qreal scale = 1.0;
    if ((radius >= 4) && (blurImage.width() >= 2) && (blurImage.height() >= 2)) {
        blurImage = qt_halfScaled(blurImage);
        scale = 2.0;
        radius *= 0.5;
    }

    if (alphaOnly) {
        expblur<12, 10, true>(blurImage, radius, quality, transposed);
    } else {
        expblur<12, 10, false>(blurImage, radius, quality, transposed);
    }

    if (p) {
        p->save();
        p->setRenderHints(QPainter::Antialiasing |
            QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        p->scale(scale, scale);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        const QSize imageSize = blurImage.deviceIndependentSize().toSize();
#else
        const QSize imageSize = QSizeF(QSizeF(blurImage.size()) / blurImage.devicePixelRatio()).toSize();
#endif
        p->drawImage(QRect(QPoint(0, 0), imageSize), blurImage);
        p->restore();
    }
}

[[maybe_unused]] static inline void qt_blurImage(QImage &blurImage,
    const qreal radius, const bool quality, const int transposed = 0)
{
    if ((blurImage.format() == QImage::Format_Indexed8)
        || (blurImage.format() == QImage::Format_Grayscale8)) {
        expblur<12, 10, true>(blurImage, radius, quality, transposed);
    } else {
        expblur<12, 10, false>(blurImage, radius, quality, transposed);
    }
}
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE

/*!
    Transforms an \a alignment of Qt::AlignLeft or Qt::AlignRight
    without Qt::AlignAbsolute into Qt::AlignLeft or Qt::AlignRight with
    Qt::AlignAbsolute according to the layout \a direction. The other
    alignment flags are left untouched.

    If no horizontal alignment was specified, the function returns the
    default alignment for the given layout \a direction.

    \sa QWidget::layoutDirection
*/
[[nodiscard]] static inline Qt::Alignment visualAlignment
    (const Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask)) {
        alignment |= Qt::AlignLeft;
    }
    if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft) {
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        }
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

/*!
    Returns a new rectangle of the specified \a size that is aligned to the given
    \a rectangle according to the specified \a alignment and \a direction.
 */
[[nodiscard]] static inline QRect alignedRect(const Qt::LayoutDirection direction,
    Qt::Alignment alignment, const QSize &size, const QRect &rectangle)
{
    alignment = visualAlignment(direction, alignment);
    int x = rectangle.x();
    int y = rectangle.y();
    int w = size.width();
    int h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter) {
        y += ((rectangle.size().height() / 2) - (h / 2));
    } else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom) {
        y += (rectangle.size().height() - h);
    }
    if ((alignment & Qt::AlignRight) == Qt::AlignRight) {
        x += (rectangle.size().width() - w);
    } else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter) {
        x += ((rectangle.size().width() / 2) - (w / 2));
    }
    return {x, y, w, h};
}

MicaMaterialPrivate::MicaMaterialPrivate(MicaMaterial *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

MicaMaterialPrivate::~MicaMaterialPrivate() = default;

MicaMaterialPrivate *MicaMaterialPrivate::get(MicaMaterial *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

const MicaMaterialPrivate *MicaMaterialPrivate::get(const MicaMaterial *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

void MicaMaterialPrivate::maybeGenerateBlurredWallpaper(const bool force)
{
    if (!g_micaMaterialData()->blurredWallpaper.isNull() && !force) {
        return;
    }
    const QSize size = QGuiApplication::primaryScreen()->virtualSize();
    g_micaMaterialData()->blurredWallpaper = QPixmap(size);
    g_micaMaterialData()->blurredWallpaper.fill(kDefaultTransparentColor);
    const QString wallpaperFilePath = Utils::getWallpaperFilePath();
    if (wallpaperFilePath.isEmpty()) {
        WARNING << "Failed to retrieve the wallpaper file path.";
        return;
    }
    QImage image(wallpaperFilePath);
    if (image.isNull()) {
        WARNING << "QImage doesn't support this kind of file:" << wallpaperFilePath;
        return;
    }
    WallpaperAspectStyle aspectStyle = Utils::getWallpaperAspectStyle();
    QImage buffer(size, QImage::Format_ARGB32_Premultiplied);
#ifdef Q_OS_WINDOWS
    if (aspectStyle == WallpaperAspectStyle::Center) {
        buffer.fill(kDefaultBlackColor);
    }
#endif
    if ((aspectStyle == WallpaperAspectStyle::Stretch)
        || (aspectStyle == WallpaperAspectStyle::Fit)
        || (aspectStyle == WallpaperAspectStyle::Fill)) {
        Qt::AspectRatioMode mode = Qt::KeepAspectRatioByExpanding;
        if (aspectStyle == WallpaperAspectStyle::Stretch) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == WallpaperAspectStyle::Fit) {
            mode = Qt::KeepAspectRatio;
        }
        QSize newSize = image.size();
        newSize.scale(size, mode);
        image = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    static constexpr const QPoint desktopOriginPoint = {0, 0};
    const QRect desktopRect = {desktopOriginPoint, size};
    if (aspectStyle == WallpaperAspectStyle::Tile) {
        QPainter bufferPainter(&buffer);
        bufferPainter.setRenderHints(QPainter::Antialiasing |
            QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        bufferPainter.fillRect(desktopRect, QBrush(image));
    } else {
        QPainter bufferPainter(&buffer);
        bufferPainter.setRenderHints(QPainter::Antialiasing |
            QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        const QRect rect = alignedRect(Qt::LeftToRight, Qt::AlignCenter, image.size(), desktopRect);
        bufferPainter.drawImage(rect.topLeft(), image);
    }
    QPainter painter(&g_micaMaterialData()->blurredWallpaper);
    painter.setRenderHints(QPainter::Antialiasing |
        QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    painter.drawImage(desktopOriginPoint, buffer);
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    qt_blurImage(&painter, buffer, kDefaultBlurRadius, true, false);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
    if (initialized) {
        Q_Q(MicaMaterial);
        Q_EMIT q->shouldRedraw();
    }
}

void MicaMaterialPrivate::updateMaterialBrush()
{
#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    framelesshelpercore_initResource();
    static const QImage noiseTexture = QImage(kNoiseImageFilePath);
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    QImage micaTexture = QImage(QSize(64, 64), QImage::Format_ARGB32_Premultiplied);
    QColor fillColor = ((FramelessManager::instance()->systemTheme() == SystemTheme::Dark) ? kDefaultSystemDarkColor : kDefaultSystemLightColor2);
    fillColor.setAlphaF(0.9f);
    micaTexture.fill(fillColor);
    QPainter painter(&micaTexture);
    painter.setRenderHints(QPainter::Antialiasing |
        QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setOpacity(tintOpacity);
    const QRect rect = {QPoint(0, 0), micaTexture.size()};
    painter.fillRect(rect, tintColor);
    painter.setOpacity(noiseOpacity);
#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    painter.fillRect(rect, QBrush(noiseTexture));
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    micaBrush = QBrush(micaTexture);
    if (initialized) {
        Q_Q(MicaMaterial);
        Q_EMIT q->shouldRedraw();
    }
}

void MicaMaterialPrivate::paint(QPainter *painter, const QSize &size, const QPoint &pos, const bool active)
{
    Q_ASSERT(painter);
    if (!painter) {
        return;
    }
    prepareGraphicsResources();
    static constexpr const QPoint originPoint = {0, 0};
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing |
        QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    if (active) {
        painter->drawPixmap(originPoint, g_micaMaterialData()->blurredWallpaper, QRect(pos, size));
    }
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(qreal(1));
    painter->fillRect(QRect{originPoint, size}, [this, active]() -> QBrush {
        if (!fallbackEnabled || active) {
            return micaBrush;
        }
        if (fallbackColor.isValid()) {
            return fallbackColor;
        }
        return systemFallbackColor();
    }());
    painter->restore();
}

void MicaMaterialPrivate::initialize()
{
    tintColor = kDefaultTransparentColor;
    tintOpacity = kDefaultTintOpacity;
    // Leave fallbackColor invalid, we need to use this state to judge
    // whether we should use the system color instead.
    noiseOpacity = kDefaultNoiseOpacity;

    updateMaterialBrush();

    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged,
        this, &MicaMaterialPrivate::updateMaterialBrush);
    connect(FramelessManager::instance(), &FramelessManager::wallpaperChanged,
        this, [this](){
            maybeGenerateBlurredWallpaper(true);
        });

    if (FramelessConfig::instance()->isSet(Option::DisableLazyInitializationForMicaMaterial)) {
        prepareGraphicsResources();
    }

    initialized = true;
}

void MicaMaterialPrivate::prepareGraphicsResources()
{
    if (g_micaMaterialData()->graphicsResourcesReady) {
        return;
    }
    g_micaMaterialData()->graphicsResourcesReady = true;
    maybeGenerateBlurredWallpaper();
}

QColor MicaMaterialPrivate::systemFallbackColor()
{
    return ((FramelessManager::instance()->systemTheme() == SystemTheme::Dark) ? kDefaultFallbackColorDark : kDefaultFallbackColorLight);
}

MicaMaterial::MicaMaterial(QObject *parent)
    : QObject(parent), d_ptr(new MicaMaterialPrivate(this))
{
}

MicaMaterial::~MicaMaterial() = default;

QColor MicaMaterial::tintColor() const
{
    Q_D(const MicaMaterial);
    return d->tintColor;
}

void MicaMaterial::setTintColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(MicaMaterial);
    if (d->tintColor == value) {
        return;
    }
    d->prepareGraphicsResources();
    d->tintColor = value;
    d->updateMaterialBrush();
    Q_EMIT tintColorChanged();
}

qreal MicaMaterial::tintOpacity() const
{
    Q_D(const MicaMaterial);
    return d->tintOpacity;
}

void MicaMaterial::setTintOpacity(const qreal value)
{
    Q_D(MicaMaterial);
    if (qFuzzyCompare(d->tintOpacity, value)) {
        return;
    }
    d->prepareGraphicsResources();
    d->tintOpacity = value;
    d->updateMaterialBrush();
    Q_EMIT tintOpacityChanged();
}

QColor MicaMaterial::fallbackColor() const
{
    Q_D(const MicaMaterial);
    return d->fallbackColor;
}

void MicaMaterial::setFallbackColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(MicaMaterial);
    if (d->fallbackColor == value) {
        return;
    }
    d->prepareGraphicsResources();
    d->fallbackColor = value;
    d->updateMaterialBrush();
    Q_EMIT fallbackColorChanged();
}

qreal MicaMaterial::noiseOpacity() const
{
    Q_D(const MicaMaterial);
    return d->noiseOpacity;
}

void MicaMaterial::setNoiseOpacity(const qreal value)
{
    Q_D(MicaMaterial);
    if (qFuzzyCompare(d->noiseOpacity, value)) {
        return;
    }
    d->prepareGraphicsResources();
    d->noiseOpacity = value;
    d->updateMaterialBrush();
    Q_EMIT noiseOpacityChanged();
}

bool MicaMaterial::isFallbackEnabled() const
{
    Q_D(const MicaMaterial);
    return d->fallbackEnabled;
}

void MicaMaterial::setFallbackEnabled(const bool value)
{
    Q_D(MicaMaterial);
    if (d->fallbackEnabled == value) {
        return;
    }
    d->prepareGraphicsResources();
    d->fallbackEnabled = value;
    d->updateMaterialBrush();
    Q_EMIT fallbackEnabledChanged();
}

void MicaMaterial::paint(QPainter *painter, const QSize &size, const QPoint &pos, const bool active)
{
    Q_D(MicaMaterial);
    d->paint(painter, size, pos, active);
}

FRAMELESSHELPER_END_NAMESPACE
