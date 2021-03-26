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

#include "utilities.h"
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qscreen.h>
#include <QtGui/qpainter.h>
#include <QtGui/private/qmemrotate_p.h>
#include <QtCore/qdebug.h>

/*
 * Copied from https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/effects/qpixmapfilter.cpp
 * With minor modifications, most of them are format changes.
 * They are exported functions of Qt, we can make use of them directly, but they are in the QtWidgets
 * module, I don't want our library have such a dependency.
 */

#ifndef AVG
#define AVG(a,b)  ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )
#endif

#ifndef AVG16
#define AVG16(a,b)  ( ((((a)^(b)) & 0xf7deUL) >> 1) + ((a)&(b)) )
#endif

template<const int shift>
static inline int qt_static_shift(const int value)
{
    if (shift == 0) {
        return value;
    } else if (shift > 0) {
        return value << (uint(shift) & 0x1f);
    } else {
        return value >> (uint(-shift) & 0x1f);
    }
}

template<const int aprec, const int zprec>
static inline void qt_blurinner(uchar *bptr, int &zR, int &zG, int &zB, int &zA, const int alpha)
{
    QRgb *pixel = reinterpret_cast<QRgb *>(bptr);
#define Z_MASK (0xff << zprec)
    const int A_zprec = qt_static_shift<zprec - 24>(*pixel) & Z_MASK;
    const int R_zprec = qt_static_shift<zprec - 16>(*pixel) & Z_MASK;
    const int G_zprec = qt_static_shift<zprec - 8>(*pixel)  & Z_MASK;
    const int B_zprec = qt_static_shift<zprec>(*pixel)      & Z_MASK;
#undef Z_MASK
    const int zR_zprec = zR >> aprec;
    const int zG_zprec = zG >> aprec;
    const int zB_zprec = zB >> aprec;
    const int zA_zprec = zA >> aprec;
    zR += alpha * (R_zprec - zR_zprec);
    zG += alpha * (G_zprec - zG_zprec);
    zB += alpha * (B_zprec - zB_zprec);
    zA += alpha * (A_zprec - zA_zprec);
#define ZA_MASK (0xff << (zprec + aprec))
    *pixel =
        qt_static_shift<24 - zprec - aprec>(zA & ZA_MASK)
        | qt_static_shift<16 - zprec - aprec>(zR & ZA_MASK)
        | qt_static_shift<8 - zprec - aprec>(zG & ZA_MASK)
        | qt_static_shift<-zprec - aprec>(zB & ZA_MASK);
#undef ZA_MASK
}

static const int alphaIndex = ((QSysInfo::ByteOrder == QSysInfo::BigEndian) ? 0 : 3);

template<const int aprec, const int zprec>
static inline void qt_blurinner_alphaOnly(uchar *bptr, int &z, const int alpha)
{
    const int A_zprec = int(*(bptr)) << zprec;
    const int z_zprec = z >> aprec;
    z += alpha * (A_zprec - z_zprec);
    *(bptr) = z >> (zprec + aprec);
}

template<const int aprec, const int zprec, const bool alphaOnly>
static inline void qt_blurrow(QImage &im, const int line, const int alpha)
{
    uchar *bptr = im.scanLine(line);
    int zR = 0, zG = 0, zB = 0, zA = 0;
    if (alphaOnly && (im.format() != QImage::Format_Indexed8)) {
        bptr += alphaIndex;
    }
    const int stride = im.depth() >> 3;
    const int im_width = im.width();
    for (int index = 0; index < im_width; ++index) {
        if (alphaOnly) {
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        } else {
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
        }
        bptr += stride;
    }
    bptr -= stride;
    for (int index = im_width - 2; index >= 0; --index) {
        bptr -= stride;
        if (alphaOnly) {
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        } else {
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
        }
    }
}

/*
 *  expblur(QImage &img, const qreal radius)
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
static inline void expblur(QImage &img, const qreal radius, const bool improvedQuality = false, const int transposed = 0)
{
    qreal _radius = radius;
    // halve the radius if we're using two passes
    if (improvedQuality) {
        _radius *= 0.5;
    }
    Q_ASSERT((img.format() == QImage::Format_ARGB32_Premultiplied)
             || (img.format() == QImage::Format_RGB32)
             || (img.format() == QImage::Format_Indexed8)
             || (img.format() == QImage::Format_Grayscale8));
    // choose the alpha such that pixels at radius distance from a fully
    // saturated pixel will have an alpha component of no greater than
    // the cutOffIntensity
    const qreal cutOffIntensity = 2;
    const int alpha = _radius <= qreal(1e-5)
                    ? ((1 << aprec)-1)
                    : qRound((1<<aprec)*(1 - qPow(cutOffIntensity * (1 / qreal(255)), 1 / _radius)));
    int img_height = img.height();
    for (int row = 0; row < img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i) {
            qt_blurrow<aprec, zprec, alphaOnly>(img, row, alpha);
        }
    }
    QImage temp(img.height(), img.width(), img.format());
    temp.setDevicePixelRatio(img.devicePixelRatio());
    if (transposed >= 0) {
        if (img.depth() == 8) {
            qt_memrotate270(reinterpret_cast<const quint8*>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint8*>(temp.bits()),
                            temp.bytesPerLine());
        } else {
            qt_memrotate270(reinterpret_cast<const quint32*>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint32*>(temp.bits()),
                            temp.bytesPerLine());
        }
    } else {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8*>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint8*>(temp.bits()),
                           temp.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32*>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint32*>(temp.bits()),
                           temp.bytesPerLine());
        }
    }
    img_height = temp.height();
    for (int row = 0; row < img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i) {
            qt_blurrow<aprec, zprec, alphaOnly>(temp, row, alpha);
        }
    }
    if (transposed == 0) {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8*>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint8*>(img.bits()),
                           img.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32*>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint32*>(img.bits()),
                           img.bytesPerLine());
        }
    } else {
        img = temp;
    }
}

static inline QImage qt_halfScaled(const QImage &source)
{
    if (source.width() < 2 || source.height() < 2) {
        return {};
    }
    QImage srcImage = source;
    if (source.format() == QImage::Format_Indexed8 || source.format() == QImage::Format_Grayscale8) {
        // assumes grayscale
        QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
        dest.setDevicePixelRatio(source.devicePixelRatio());
        const uchar *src = reinterpret_cast<const uchar*>(const_cast<const QImage &>(srcImage).bits());
        qsizetype sx = srcImage.bytesPerLine();
        qsizetype sx2 = sx << 1;
        uchar *dst = reinterpret_cast<uchar*>(dest.bits());
        qsizetype dx = dest.bytesPerLine();
        int ww = dest.width();
        int hh = dest.height();
        for (int y = hh; y; --y, dst += dx, src += sx2) {
            const uchar *p1 = src;
            const uchar *p2 = src + sx;
            uchar *q = dst;
            for (int x = ww; x; --x, ++q, p1 += 2, p2 += 2) {
                *q = ((int(p1[0]) + int(p1[1]) + int(p2[0]) + int(p2[1])) + 2) >> 2;
            }
        }
        return dest;
    } else if (source.format() == QImage::Format_ARGB8565_Premultiplied) {
        QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
        dest.setDevicePixelRatio(source.devicePixelRatio());
        const uchar *src = reinterpret_cast<const uchar*>(const_cast<const QImage &>(srcImage).bits());
        qsizetype sx = srcImage.bytesPerLine();
        qsizetype sx2 = sx << 1;
        uchar *dst = reinterpret_cast<uchar*>(dest.bits());
        qsizetype dx = dest.bytesPerLine();
        int ww = dest.width();
        int hh = dest.height();
        for (int y = hh; y; --y, dst += dx, src += sx2) {
            const uchar *p1 = src;
            const uchar *p2 = src + sx;
            uchar *q = dst;
            for (int x = ww; x; --x, q += 3, p1 += 6, p2 += 6) {
                // alpha
                q[0] = AVG(AVG(p1[0], p1[3]), AVG(p2[0], p2[3]));
                // rgb
                const quint16 p16_1 = (p1[2] << 8) | p1[1];
                const quint16 p16_2 = (p1[5] << 8) | p1[4];
                const quint16 p16_3 = (p2[2] << 8) | p2[1];
                const quint16 p16_4 = (p2[5] << 8) | p2[4];
                const quint16 result = AVG16(AVG16(p16_1, p16_2), AVG16(p16_3, p16_4));
                q[1] = result & 0xff;
                q[2] = result >> 8;
            }
        }
        return dest;
    } else if ((source.format() != QImage::Format_ARGB32_Premultiplied) && (source.format() != QImage::Format_RGB32)) {
        srcImage = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    QImage dest(source.width() / 2, source.height() / 2, srcImage.format());
    dest.setDevicePixelRatio(source.devicePixelRatio());
    const quint32 *src = reinterpret_cast<const quint32*>(const_cast<const QImage &>(srcImage).bits());
    qsizetype sx = srcImage.bytesPerLine() >> 2;
    qsizetype sx2 = sx << 1;
    quint32 *dst = reinterpret_cast<quint32*>(dest.bits());
    qsizetype dx = dest.bytesPerLine() >> 2;
    int ww = dest.width();
    int hh = dest.height();
    for (int y = hh; y; --y, dst += dx, src += sx2) {
        const quint32 *p1 = src;
        const quint32 *p2 = src + sx;
        quint32 *q = dst;
        for (int x = ww; x; --x, q++, p1 += 2, p2 += 2) {
            *q = AVG(AVG(p1[0], p1[1]), AVG(p2[0], p2[1]));
        }
    }
    return dest;
}

void Utilities::blurImage(QPainter *painter, QImage &blurImage, const qreal radius, const bool quality, const bool alphaOnly, const int transposed)
{
    if ((blurImage.format() != QImage::Format_ARGB32_Premultiplied) && (blurImage.format() != QImage::Format_RGB32)) {
        blurImage = blurImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    qreal _radius = radius;
    qreal scale = 1;
    if ((_radius >= 4) && (blurImage.width() >= 2) && (blurImage.height() >= 2)) {
        blurImage = qt_halfScaled(blurImage);
        scale = 2;
        _radius *= 0.5;
    }
    if (alphaOnly) {
        expblur<12, 10, true>(blurImage, _radius, quality, transposed);
    } else {
        expblur<12, 10, false>(blurImage, _radius, quality, transposed);
    }
    if (painter) {
        painter->scale(scale, scale);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(QRect{QPoint{0, 0}, blurImage.size() / blurImage.devicePixelRatio()}, blurImage);
    }
}

void Utilities::blurImage(QImage &blurImage, const qreal radius, const bool quality, const int transposed)
{
    if ((blurImage.format() == QImage::Format_Indexed8) || (blurImage.format() == QImage::Format_Grayscale8)) {
        expblur<12, 10, true>(blurImage, radius, quality, transposed);
    } else {
        expblur<12, 10, false>(blurImage, radius, quality, transposed);
    }
}

///////////////////////////////////////////////////

/*
 * Copied from https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/styles/qstyle.cpp
 * With minor modifications, most of them are format changes.
 * They are exported functions of Qt, we can make use of them directly, but they are in the QtWidgets
 * module, I don't want our library have such a dependency.
 */

static inline Qt::Alignment visualAlignment(const Qt::LayoutDirection direction, const Qt::Alignment alignment)
{
    return QGuiApplicationPrivate::visualAlignment(direction, alignment);
}

QRect Utilities::alignedRect(const Qt::LayoutDirection direction, const Qt::Alignment alignment, const QSize &size, const QRect &rectangle)
{
    const Qt::Alignment align = visualAlignment(direction, alignment);
    int x = rectangle.x();
    int y = rectangle.y();
    const int w = size.width();
    const int h = size.height();
    if ((align & Qt::AlignVCenter) == Qt::AlignVCenter) {
        y += rectangle.size().height() / 2 - h / 2;
    } else if ((align & Qt::AlignBottom) == Qt::AlignBottom) {
        y += rectangle.size().height() - h;
    }
    if ((align & Qt::AlignRight) == Qt::AlignRight) {
        x += rectangle.size().width() - w;
    } else if ((align & Qt::AlignHCenter) == Qt::AlignHCenter) {
        x += rectangle.size().width() / 2 - w / 2;
    }
    return {x, y, w, h};
}

///////////////////////////////////////////////////

QWindow *Utilities::findWindow(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == winId) {
                return window;
            }
        }
    }
    return nullptr;
}

QRect Utilities::getScreenAvailableGeometry(const QWindow *window)
{
    if (window) {
        const QScreen *screen = window->screen();
        if (screen) {
            return screen->availableGeometry();
        }
    }
    return QGuiApplication::primaryScreen()->availableGeometry();
}

bool Utilities::shouldUseWallpaperBlur()
{
    return !shouldUseTraditionalBlur();
}

bool Utilities::disableExtraProcessingForBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_disableExtraProcess);
}

bool Utilities::forceEnableTraditionalBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_forceEnableTraditionalBlur_flag);
}

bool Utilities::forceDisableWallpaperBlur()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_acrylic_forceDisableWallpaperBlur_flag);
}

bool Utilities::shouldUseNativeTitleBar()
{
    return qEnvironmentVariableIsSet(_flh_global::_flh_useNativeTitleBar_flag);
}

bool Utilities::isWindowFixedSize(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
#ifdef Q_OS_WINDOWS
    if (window->flags().testFlag(Qt::MSWindowsFixedSizeDialogHint)) {
        return true;
    }
#endif
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    return false;
}

bool Utilities::isMouseInSpecificObjects(const QPointF &mousePos, const QObjectList &objects, const qreal dpr)
{
    if (mousePos.isNull()) {
        qWarning() << "Mouse position point is null.";
        return false;
    }
    if (objects.isEmpty()) {
        qWarning() << "Object list is empty.";
        return false;
    }
    for (auto &&object : qAsConst(objects)) {
        if (!object) {
            qWarning() << "Object pointer is null.";
            continue;
        }
        if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
            qWarning() << object << "is not a QWidget or QQuickItem!";
            continue;
        }
        if (!object->property("visible").toBool()) {
            qDebug() << "Skipping invisible object" << object;
            continue;
        }
        const auto mapOriginPointToWindow = [](const QObject *obj) -> QPointF {
            Q_ASSERT(obj);
            if (!obj) {
                return {};
            }
            QPointF point = {obj->property("x").toReal(), obj->property("y").toReal()};
            for (QObject *parent = obj->parent(); parent; parent = parent->parent()) {
                point += {parent->property("x").toReal(), parent->property("y").toReal()};
                if (parent->isWindowType()) {
                    break;
                }
            }
            return point;
        };
        const QPointF originPoint = mapOriginPointToWindow(object);
        const qreal width = object->property("width").toReal();
        const qreal height = object->property("height").toReal();
        const QRectF rect = {originPoint.x() * dpr, originPoint.y() * dpr, width * dpr, height * dpr};
        if (rect.contains(mousePos)) {
            return true;
        }
    }
    return false;
}

QRect Utilities::getScreenAvailableGeometry(const QPoint &pos)
{
    if (!pos.isNull()) {
        const QScreen *screen = QGuiApplication::screenAt(pos);
        if (screen) {
            return screen->availableGeometry();
        }
    }
    return QGuiApplication::primaryScreen()->availableGeometry();
}

QRect Utilities::getScreenGeometry(const QWindow *window)
{
    if (window) {
        const QScreen *screen = window->screen();
        if (screen) {
            return screen->geometry();
        }
    }
    return QGuiApplication::primaryScreen()->geometry();
}

QRect Utilities::getScreenGeometry(const QPoint &pos)
{
    if (!pos.isNull()) {
        const QScreen *screen = QGuiApplication::screenAt(pos);
        if (screen) {
            return screen->geometry();
        }
    }
    return QGuiApplication::primaryScreen()->geometry();
}
