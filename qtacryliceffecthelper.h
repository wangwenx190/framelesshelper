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

#pragma once

#include "framelesshelper_global.h"
#include <QtGui/qbrush.h>

class FRAMELESSHELPER_EXPORT QtAcrylicEffectHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QtAcrylicEffectHelper)

public:
    explicit QtAcrylicEffectHelper(QObject *parent = nullptr);
    ~QtAcrylicEffectHelper() override;

    QBrush getAcrylicBrush() const;
    QColor getTintColor() const;
    qreal getTintOpacity() const;
    qreal getNoiseOpacity() const;
    QPixmap getBluredWallpaper() const;
    QColor getFrameColor() const;
    qreal getFrameThickness() const;

public Q_SLOTS:
    void install(const QWindow *window);
    void uninstall();

    void clearWallpaper();

    void showWarning() const;

    void setTintColor(const QColor &value);
    void setTintOpacity(const qreal value);
    void setNoiseOpacity(const qreal value);
    void setFrameColor(const QColor &value);
    void setFrameThickness(const qreal value);

    void paintWindowBackground(QPainter *painter, const QRegion &clip);
    void paintWindowBackground(QPainter *painter, const QRect &rect);
    void paintWindowFrame(QPainter *painter, const QRect &rect = {});
    void updateAcrylicBrush(const QColor &alternativeTintColor = {});

private:
    void paintBackground(QPainter *painter, const QRect &rect);
    void updateBehindWindowBackground();
    bool checkWindow() const;

Q_SIGNALS:
    void needsRepaint();

private:
    QWindow *m_window = nullptr;
    QBrush m_acrylicBrush = {};
    QColor m_tintColor = {};
    qreal m_tintOpacity = 0.7;
    qreal m_noiseOpacity = 0.04;
    QPixmap m_bluredWallpaper = {};
    QColor m_frameColor = {};
    qreal m_frameThickness = 1.0;
};
