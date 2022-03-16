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
class QWindow;
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
    Q_PROPERTY(QColor frameBorderActiveColor READ frameBorderActiveColor NOTIFY frameBorderActiveColorChanged FINAL)
    Q_PROPERTY(QColor frameBorderInactiveColor READ frameBorderInactiveColor NOTIFY frameBorderInactiveColorChanged FINAL)
    Q_PROPERTY(bool darkModeEnabled READ darkModeEnabled NOTIFY darkModeEnabledChanged FINAL)
    Q_PROPERTY(QColor systemAccentColor READ systemAccentColor NOTIFY systemAccentColorChanged FINAL)
    Q_PROPERTY(bool titleBarColorVisible READ titleBarColorVisible NOTIFY titleBarColorVisibleChanged FINAL)

public:
    explicit FramelessQuickUtils(QObject *parent = nullptr);
    ~FramelessQuickUtils() override;

    Q_NODISCARD static qreal titleBarHeight();
    Q_NODISCARD static bool frameBorderVisible();
    Q_NODISCARD static qreal frameBorderThickness();
    Q_NODISCARD static QColor frameBorderActiveColor();
    Q_NODISCARD static QColor frameBorderInactiveColor();
    Q_NODISCARD static bool darkModeEnabled();
    Q_NODISCARD static QColor systemAccentColor();
    Q_NODISCARD static bool titleBarColorVisible();

    Q_INVOKABLE static void showMinimized2(QWindow *window);
    Q_INVOKABLE static void showSystemMenu(QWindow *window, const QPointF &pos);
    Q_INVOKABLE static void startSystemMove2(QWindow *window);
    Q_INVOKABLE static void startSystemResize2(QWindow *window, const Qt::Edges edges);

Q_SIGNALS:
    void frameBorderActiveColorChanged();
    void frameBorderInactiveColorChanged();
    void darkModeEnabledChanged();
    void systemAccentColorChanged();
    void titleBarColorVisibleChanged();
};

FRAMELESSHELPER_END_NAMESPACE
