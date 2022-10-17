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
#include "framelessquickhelper.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

struct QuickHelperData;

class FRAMELESSHELPER_QUICK_API FramelessQuickHelperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessQuickHelper)
    Q_DISABLE_COPY_MOVE(FramelessQuickHelperPrivate)

public:
    explicit FramelessQuickHelperPrivate(FramelessQuickHelper *q);
    ~FramelessQuickHelperPrivate() override;

    Q_NODISCARD static FramelessQuickHelperPrivate *get(FramelessQuickHelper *pub);
    Q_NODISCARD static const FramelessQuickHelperPrivate *get(const FramelessQuickHelper *pub);

    Q_NODISCARD bool isContentExtendedIntoTitleBar() const;
    void extendsContentIntoTitleBar(const bool value);

    Q_NODISCARD QQuickItem *getTitleBarItem() const;
    void setTitleBarItem(QQuickItem *value);

    void attach();
    void detach();
    void setSystemButton(QQuickItem *item, const QuickGlobal::SystemButtonType buttonType);
    void setHitTestVisible(QQuickItem *item, const bool visible = true);
    void setHitTestVisible(const QRect &rect, const bool visible = true);
    void showSystemMenu(const QPoint &pos);
    void windowStartSystemMove2(const QPoint &pos);
    void windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos);

    void moveWindowToDesktopCenter();
    void bringWindowToFront();

    Q_NODISCARD bool isWindowFixedSize() const;
    void setWindowFixedSize(const bool value);

    void emitSignalForAllInstances(const QByteArray &signal);

    Q_NODISCARD bool isBlurBehindWindowEnabled() const;
    void setBlurBehindWindowEnabled(const bool value, const QColor &color);

    void setProperty(const QByteArray &name, const QVariant &value);
    Q_NODISCARD QVariant getProperty(const QByteArray &name, const QVariant &defaultValue = {});

    Q_NODISCARD QuickMicaMaterial *findOrCreateMicaMaterial() const;
    Q_NODISCARD QuickWindowBorder *findOrCreateWindowBorder() const;

    Q_NODISCARD static FramelessQuickHelper *findOrCreateFramelessHelper(QObject *object);

protected:
    Q_NODISCARD bool eventFilter(QObject *object, QEvent *event) override;

private:
    Q_NODISCARD QRect mapItemGeometryToScene(const QQuickItem * const item) const;
    Q_NODISCARD bool isInSystemButtons(const QPoint &pos, QuickGlobal::SystemButtonType *button) const;
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldIgnoreMouseEvents(const QPoint &pos) const;
    void setSystemButtonState(const QuickGlobal::SystemButtonType button, const QuickGlobal::ButtonState state);
    Q_NODISCARD QuickHelperData getWindowData() const;
    Q_NODISCARD QuickHelperData *getWindowDataMutable() const;
    void rebindWindow();

private:
    QPointer<FramelessQuickHelper> q_ptr = nullptr;
    QColor m_savedWindowBackgroundColor = {};
    bool m_blurBehindWindowEnabled = false;
    std::optional<bool> m_extendIntoTitleBar = std::nullopt;
    bool m_destroying = false;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessQuickHelperPrivate))
