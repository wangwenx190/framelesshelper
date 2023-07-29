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

#include <FramelessHelper/Widgets/framelesshelperwidgets_global.h>
#include <optional>

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

    Q_NODISCARD QString getGlyph() const;
    void setGlyph(const QString &value);

    Q_NODISCARD Global::SystemButtonType getButtonType() const;
    void setButtonType(const Global::SystemButtonType type);

    Q_NODISCARD QSize getRecommendedButtonSize() const;

    Q_NODISCARD bool isHovered() const;
    Q_NODISCARD bool isPressed() const;
    Q_NODISCARD QColor getHoverColor() const;
    Q_NODISCARD QColor getPressColor() const;
    Q_NODISCARD QColor getNormalColor() const;
    Q_NODISCARD QColor getActiveForegroundColor() const;
    Q_NODISCARD QColor getInactiveForegroundColor() const;
    Q_NODISCARD bool isActive() const;
    Q_NODISCARD int iconSize2() const;

    void setHovered(const bool value);
    void setPressed(const bool value);
    void setHoverColor(const QColor &value);
    void setPressColor(const QColor &value);
    void setNormalColor(const QColor &value);
    void setActiveForegroundColor(const QColor &value);
    void setInactiveForegroundColor(const QColor &value);
    void setActive(const bool value);
    void setIconSize2(const int value);

    void enterEventHandler(QT_ENTER_EVENT_TYPE *event);
    void leaveEventHandler(QEvent *event);
    void paintEventHandler(QPaintEvent *event);

private:
    void initialize();

private:
    StandardSystemButton *q_ptr = nullptr;
    Global::SystemButtonType m_buttonType = Global::SystemButtonType::Unknown;
    QString m_glyph = {};
    QColor m_hoverColor = {};
    QColor m_pressColor = {};
    QColor m_normalColor = {};
    QColor m_activeForegroundColor = {};
    QColor m_inactiveForegroundColor = {};
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_active = false;
    std::optional<int> m_iconSize2 = std::nullopt;
};

FRAMELESSHELPER_END_NAMESPACE
