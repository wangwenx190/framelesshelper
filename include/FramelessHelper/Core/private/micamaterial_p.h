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

#pragma once

#include <FramelessHelper/Core/framelesshelpercore_global.h>
#include <QtGui/qbrush.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class MicaMaterial;

struct Transform
{
    qreal Horizontal = 0;
    qreal Vertical = 0;
};

class FRAMELESSHELPER_CORE_API MicaMaterialPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MicaMaterialPrivate)
    Q_DECLARE_PUBLIC(MicaMaterial)

public:
    explicit MicaMaterialPrivate(MicaMaterial *q);
    ~MicaMaterialPrivate() override;

    Q_NODISCARD static MicaMaterialPrivate *get(MicaMaterial *q);
    Q_NODISCARD static const MicaMaterialPrivate *get(const MicaMaterial *q);

    Q_NODISCARD static QColor systemFallbackColor();

    Q_NODISCARD static QSize monitorSize();
    Q_NODISCARD static QSize wallpaperSize();

    Q_NODISCARD QPoint mapToWallpaper(const QPoint &pos) const;
    Q_NODISCARD QSize mapToWallpaper(const QSize &size) const;
    Q_NODISCARD QRect mapToWallpaper(const QRect &rect) const;

public Q_SLOTS:
    void maybeGenerateBlurredWallpaper(const bool force = false);
    void updateMaterialBrush();
    void paint(QPainter *painter, const QRect &rect, const bool active = true);
    void forceRebuildWallpaper();

private:
    void initialize();
    void prepareGraphicsResources();

private:
    MicaMaterial *q_ptr = nullptr;
    QColor tintColor = {};
    qreal tintOpacity = 0.0;
    QColor fallbackColor = {};
    qreal noiseOpacity = 0.0;
    bool fallbackEnabled = true;
    QBrush micaBrush = {};
    bool initialized = false;
    Transform transform = {};
};

FRAMELESSHELPER_END_NAMESPACE
