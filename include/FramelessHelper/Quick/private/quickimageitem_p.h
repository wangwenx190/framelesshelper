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

#include <FramelessHelper/Quick/framelesshelperquick_global.h>
#include <QtCore/qvariant.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class QuickImageItem;

class FRAMELESSHELPER_QUICK_API QuickImageItemPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QuickImageItemPrivate)
    Q_DECLARE_PUBLIC(QuickImageItem)

public:
    explicit QuickImageItemPrivate(QuickImageItem *q);
    ~QuickImageItemPrivate() override;

    Q_NODISCARD static QuickImageItemPrivate *get(QuickImageItem *q);
    Q_NODISCARD static const QuickImageItemPrivate *get(const QuickImageItem *q);

    void paint(QPainter *painter) const;

    Q_NODISCARD QVariant source() const;
    void setSource(const QVariant &value);

private:
    void initialize();
    void fromUrl(const QUrl &value, QPainter *painter) const;
    void fromString(const QString &value, QPainter *painter) const;
    void fromImage(const QImage &value, QPainter *painter) const;
    void fromPixmap(const QPixmap &value, QPainter *painter) const;
    void fromIcon(const QIcon &value, QPainter *painter) const;
    Q_NODISCARD QRectF paintArea() const;

private:
    QuickImageItem *q_ptr = nullptr;
    QVariant m_source = {};
};

FRAMELESSHELPER_END_NAMESPACE
