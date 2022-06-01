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
#include <QtCore/qobject.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

struct WidgetsHelperData;
class FramelessWidgetsHelper;

class FRAMELESSHELPER_WIDGETS_API FramelessWidgetsHelperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessWidgetsHelper)
    Q_DISABLE_COPY_MOVE(FramelessWidgetsHelperPrivate)

public:
    explicit FramelessWidgetsHelperPrivate(FramelessWidgetsHelper *q);
    ~FramelessWidgetsHelperPrivate() override;

    Q_NODISCARD static FramelessWidgetsHelperPrivate *get(FramelessWidgetsHelper *pub);
    Q_NODISCARD static const FramelessWidgetsHelperPrivate *get(const FramelessWidgetsHelper *pub);

    Q_NODISCARD QWidget *getTitleBarWidget() const;
    void setTitleBarWidget(QWidget *widget);

    void attachToWindow();
    void setSystemButton(QWidget *widget, const Global::SystemButtonType buttonType);
    void setHitTestVisible(QWidget *widget, const bool visible = true);
    void showSystemMenu(const QPoint &pos);
    void windowStartSystemMove2(const QPoint &pos);
    void windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos);

    void moveWindowToDesktopCenter();
    void bringWindowToFront();

    Q_NODISCARD bool isWindowFixedSize() const;
    void setWindowFixedSize(const bool value);

    void emitSignalForAllInstances(const QByteArray &signal);

private:
    Q_NODISCARD QRect mapWidgetGeometryToScene(const QWidget * const widget) const;
    Q_NODISCARD bool isInSystemButtons(const QPoint &pos, Global::SystemButtonType *button) const;
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldIgnoreMouseEvents(const QPoint &pos) const;
    void setSystemButtonState(const Global::SystemButtonType button, const Global::ButtonState state);
    Q_NODISCARD QWidget *getWindow() const;
    Q_NODISCARD WidgetsHelperData getWindowData() const;
    Q_NODISCARD WidgetsHelperData *getWindowDataMutable() const;

private:
    FramelessWidgetsHelper *q_ptr = nullptr;
};

FRAMELESSHELPER_END_NAMESPACE
