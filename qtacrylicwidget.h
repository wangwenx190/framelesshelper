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
#include <QtWidgets/qwidget.h>
#include "qtacryliceffecthelper.h"

class FRAMELESSHELPER_EXPORT QtAcrylicWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QtAcrylicWidget)
    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor NOTIFY tintColorChanged)
    Q_PROPERTY(qreal tintOpacity READ tintOpacity WRITE setTintOpacity NOTIFY tintOpacityChanged)
    Q_PROPERTY(qreal noiseOpacity READ noiseOpacity WRITE setNoiseOpacity NOTIFY noiseOpacityChanged)
    Q_PROPERTY(bool frameVisible READ frameVisible WRITE setFrameVisible NOTIFY frameVisibleChanged)
    Q_PROPERTY(QColor frameColor READ frameColor WRITE setFrameColor NOTIFY frameColorChanged)
    Q_PROPERTY(qreal frameThickness READ frameThickness WRITE setFrameThickness NOTIFY frameThicknessChanged)
    Q_PROPERTY(bool acrylicEnabled READ acrylicEnabled WRITE setAcrylicEnabled NOTIFY acrylicEnabledChanged)

public:
    explicit QtAcrylicWidget(QWidget *parent = nullptr);
    ~QtAcrylicWidget() override;

    QColor tintColor() const;
    void setTintColor(const QColor &value);

    qreal tintOpacity() const;
    void setTintOpacity(const qreal value);

    qreal noiseOpacity() const;
    void setNoiseOpacity(const qreal value);

    bool frameVisible() const;
    void setFrameVisible(const bool value);

    QColor frameColor() const;
    void setFrameColor(const QColor &value);

    qreal frameThickness() const;
    void setFrameThickness(const qreal value);

    bool acrylicEnabled() const;
    void setAcrylicEnabled(const bool value);

Q_SIGNALS:
    void tintColorChanged();
    void tintOpacityChanged();
    void noiseOpacityChanged();
    void frameVisibleChanged();
    void frameColorChanged();
    void frameThicknessChanged();
    void acrylicEnabledChanged();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QtAcrylicEffectHelper m_acrylicHelper;
    bool m_frameVisible = true;
    bool m_acrylicEnabled = false;
    bool m_inited = false;
};
