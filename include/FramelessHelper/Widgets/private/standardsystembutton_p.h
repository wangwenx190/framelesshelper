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

#include <QtCore/qobject.h>
#include <QtGui/qpixmap.h>
#include "framelesshelperwidgets_global.h"

QT_BEGIN_NAMESPACE
class QEnterEvent;
class QPaintEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class StandardSystemButton;

class FRAMELESSHELPER_WIDGETS_API StandardSystemButtonPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(StandardSystemButton)
    Q_DISABLE_COPY_MOVE(StandardSystemButtonPrivate)

public:
    explicit StandardSystemButtonPrivate(StandardSystemButton *q);
    ~StandardSystemButtonPrivate() override;

    Q_NODISCARD static StandardSystemButtonPrivate *get(StandardSystemButton *pub);
    Q_NODISCARD static const StandardSystemButtonPrivate *get(const StandardSystemButton *pub);

    void refreshButtonTheme(const bool force);

    Q_NODISCARD Global::SystemButtonType getButtonType() const;
    void setButtonType(const Global::SystemButtonType type);

    void setIcon(const QIcon &value, const bool reverse);
    void setPixmap(const QPixmap &value, const bool reverse);
    void setImage(const QImage &value, const bool reverse);

    Q_NODISCARD QSize getRecommendedButtonSize() const;

    Q_NODISCARD bool isHovered() const;
    Q_NODISCARD bool isPressed() const;
    Q_NODISCARD QColor getHoverColor() const;
    Q_NODISCARD QColor getPressColor() const;

    void setHovered(const bool value);
    void setPressed(const bool value);
    void setHoverColor(const QColor &value);
    void setPressColor(const QColor &value);

    void enterEventHandler(QT_ENTER_EVENT_TYPE *event);
    void leaveEventHandler(QEvent *event);
    void paintEventHandler(QPaintEvent *event);

    void setInactive(const bool value);

private:
    void initialize();
    void checkInactive();

private:
    StandardSystemButton *q_ptr = nullptr;
    Global::SystemTheme m_buttonTheme = Global::SystemTheme::Unknown;
    Global::SystemButtonType m_buttonType = Global::SystemButtonType::Unknown;
    QPixmap m_icon = {};
    QPixmap m_reversedIcon = {};
    QColor m_hoverColor = {};
    QColor m_pressColor = {};
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_forceLightTheme = false;
    bool m_shouldCheck = false;
    bool m_checkFlag = false;
};

FRAMELESSHELPER_END_NAMESPACE
