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
#include <QtQuick/qquickitem.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessQuickHelperPrivate;

class FRAMELESSHELPER_QUICK_API FramelessQuickHelper : public QQuickItem
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessHelper)
#endif
#ifdef QML_ATTACHED
    QML_ATTACHED(FramelessQuickHelper)
#endif
    Q_DECLARE_PRIVATE(FramelessQuickHelper)
    Q_DISABLE_COPY_MOVE(FramelessQuickHelper)
    Q_PROPERTY(QQuickItem* titleBarItem READ titleBarItem WRITE setTitleBarItem NOTIFY titleBarItemChanged FINAL)
    Q_PROPERTY(bool windowFixedSize READ isWindowFixedSize WRITE setWindowFixedSize NOTIFY windowFixedSizeChanged FINAL)
    Q_PROPERTY(bool blurBehindWindowEnabled READ isBlurBehindWindowEnabled WRITE setBlurBehindWindowEnabled NOTIFY blurBehindWindowEnabledChanged FINAL)

public:
    explicit FramelessQuickHelper(QQuickItem *parent = nullptr);
    ~FramelessQuickHelper() override;

    Q_NODISCARD static FramelessQuickHelper *get(QObject *object);
    Q_NODISCARD static FramelessQuickHelper *qmlAttachedProperties(QObject *parentObject);

    Q_NODISCARD QQuickItem *titleBarItem() const;
    Q_NODISCARD bool isWindowFixedSize() const;
    Q_NODISCARD bool isBlurBehindWindowEnabled() const;

public Q_SLOTS:
    void extendsContentIntoTitleBar();

    void setTitleBarItem(QQuickItem *value);
    void setSystemButton(QQuickItem *item, const QuickGlobal::SystemButtonType buttonType);
    void setHitTestVisible(QQuickItem *item, const bool visible = true);

    void showSystemMenu(const QPoint &pos);
    void windowStartSystemMove2(const QPoint &pos);
    void windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos);

    void moveWindowToDesktopCenter();
    void bringWindowToFront();
    void setWindowFixedSize(const bool value);
    void setBlurBehindWindowEnabled(const bool value);

protected:
    void itemChange(const ItemChange change, const ItemChangeData &value) override;

Q_SIGNALS:
    void titleBarItemChanged();
    void windowFixedSizeChanged();
    void blurBehindWindowEnabledChanged();
    void ready();

private:
    QScopedPointer<FramelessQuickHelperPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE

QML_DECLARE_TYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessQuickHelper))
QML_DECLARE_TYPEINFO(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessQuickHelper), QML_HAS_ATTACHED_PROPERTIES)
