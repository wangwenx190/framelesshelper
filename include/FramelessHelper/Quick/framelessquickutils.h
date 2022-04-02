/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
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

#include "framelesshelperquick_global.h"
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE
class QQuickWindow;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_QUICK_API FramelessQuickUtils : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessQuickUtils)
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessUtils)
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_PROPERTY(qreal titleBarHeight READ titleBarHeight CONSTANT FINAL)
    Q_PROPERTY(bool frameBorderVisible READ frameBorderVisible CONSTANT FINAL)
    Q_PROPERTY(qreal frameBorderThickness READ frameBorderThickness CONSTANT FINAL)
    Q_PROPERTY(bool darkModeEnabled READ darkModeEnabled NOTIFY darkModeEnabledChanged FINAL)
    Q_PROPERTY(QColor systemAccentColor READ systemAccentColor NOTIFY systemAccentColorChanged FINAL)
    Q_PROPERTY(bool titleBarColorized READ titleBarColorized NOTIFY titleBarColorizedChanged FINAL)
    Q_PROPERTY(QColor defaultSystemLightColor READ defaultSystemLightColor CONSTANT FINAL)
    Q_PROPERTY(QColor defaultSystemDarkColor READ defaultSystemDarkColor CONSTANT FINAL)
    Q_PROPERTY(QSizeF defaultSystemButtonSize READ defaultSystemButtonSize CONSTANT FINAL)
    Q_PROPERTY(QSizeF defaultSystemButtonIconSize READ defaultSystemButtonIconSize CONSTANT FINAL)
    Q_PROPERTY(QColor defaultSystemButtonHoverColor READ defaultSystemButtonHoverColor CONSTANT FINAL)
    Q_PROPERTY(QColor defaultSystemButtonPressColor READ defaultSystemButtonPressColor CONSTANT FINAL)
    Q_PROPERTY(QColor defaultSystemCloseButtonHoverColor READ defaultSystemCloseButtonHoverColor CONSTANT FINAL)
    Q_PROPERTY(QColor defaultSystemCloseButtonPressColor READ defaultSystemCloseButtonPressColor CONSTANT FINAL)

public:
    explicit FramelessQuickUtils(QObject *parent = nullptr);
    ~FramelessQuickUtils() override;

    Q_NODISCARD static qreal titleBarHeight();
    Q_NODISCARD static bool frameBorderVisible();
    Q_NODISCARD static qreal frameBorderThickness();
    Q_NODISCARD static bool darkModeEnabled();
    Q_NODISCARD static QColor systemAccentColor();
    Q_NODISCARD static bool titleBarColorized();
    Q_NODISCARD static QColor defaultSystemLightColor();
    Q_NODISCARD static QColor defaultSystemDarkColor();
    Q_NODISCARD static QSizeF defaultSystemButtonSize();
    Q_NODISCARD static QSizeF defaultSystemButtonIconSize();
    Q_NODISCARD static QColor defaultSystemButtonHoverColor();
    Q_NODISCARD static QColor defaultSystemButtonPressColor();
    Q_NODISCARD static QColor defaultSystemCloseButtonHoverColor();
    Q_NODISCARD static QColor defaultSystemCloseButtonPressColor();

Q_SIGNALS:
    void darkModeEnabledChanged();
    void systemAccentColorChanged();
    void titleBarColorizedChanged();
};

FRAMELESSHELPER_END_NAMESPACE
