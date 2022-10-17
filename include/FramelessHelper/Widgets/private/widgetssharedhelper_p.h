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

#include "framelesshelperwidgets_global.h"
#include <QtGui/qscreen.h>

QT_BEGIN_NAMESPACE
class QPaintEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class MicaMaterial;
class WindowBorderPainter;

class FRAMELESSHELPER_WIDGETS_API WidgetsSharedHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WidgetsSharedHelper)
    Q_PROPERTY(bool micaEnabled READ isMicaEnabled WRITE setMicaEnabled NOTIFY micaEnabledChanged FINAL)

public:
    explicit WidgetsSharedHelper(QObject *parent = nullptr);
    ~WidgetsSharedHelper() override;

    void setup(QWidget *widget);

    Q_NODISCARD bool isMicaEnabled() const;
    void setMicaEnabled(const bool value);

    Q_NODISCARD MicaMaterial *rawMicaMaterial() const;
    Q_NODISCARD WindowBorderPainter *rawWindowBorder() const;

protected:
    Q_NODISCARD bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void updateContentsMargins();
    void handleScreenChanged(QScreen *screen);

private:
    void changeEventHandler(QEvent *event);
    void paintEventHandler(QPaintEvent *event);

Q_SIGNALS:
    void micaEnabledChanged();

private:
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    // Due to a Qt bug, we can't initialize the QPointer objects with nullptr.
    // The bug was fixed in Qt 5.15.
    QPointer<QWidget> m_targetWidget;
    QPointer<QScreen> m_screen;
#else
    QPointer<QWidget> m_targetWidget = nullptr;
    QPointer<QScreen> m_screen = nullptr;
#endif
    bool m_micaEnabled = false;
    QScopedPointer<MicaMaterial> m_micaMaterial;
    QMetaObject::Connection m_micaRedrawConnection = {};
    qreal m_screenDpr = 0.0;
    QMetaObject::Connection m_screenDpiChangeConnection = {};
    QScopedPointer<WindowBorderPainter> m_borderPainter;
    QMetaObject::Connection m_borderRepaintConnection = {};
    QMetaObject::Connection m_screenChangeConnection = {};
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(WidgetsSharedHelper))
