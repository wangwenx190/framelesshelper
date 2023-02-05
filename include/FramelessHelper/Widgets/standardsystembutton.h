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
#include <QtWidgets/qabstractbutton.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class StandardSystemButtonPrivate;

class FRAMELESSHELPER_WIDGETS_API StandardSystemButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StandardSystemButton)
    Q_DISABLE_COPY_MOVE(StandardSystemButton)
    Q_PROPERTY(Global::SystemButtonType buttonType READ buttonType WRITE setButtonType NOTIFY buttonTypeChanged FINAL)
    Q_PROPERTY(QString glyph READ glyph WRITE setGlyph NOTIFY glyphChanged FINAL)
    Q_PROPERTY(bool hovered READ isHovered WRITE setHovered NOTIFY hoveredChanged FINAL)
    Q_PROPERTY(bool pressed READ isPressed WRITE setPressed NOTIFY pressedChanged FINAL)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged FINAL)
    Q_PROPERTY(QColor pressColor READ pressColor WRITE setPressColor NOTIFY pressColorChanged FINAL)
    Q_PROPERTY(QColor normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged FINAL)
    Q_PROPERTY(QColor activeForegroundColor READ activeForegroundColor WRITE setActiveForegroundColor NOTIFY activeForegroundColorChanged FINAL)
    Q_PROPERTY(QColor inactiveForegroundColor READ inactiveForegroundColor WRITE setInactiveForegroundColor NOTIFY inactiveForegroundColorChanged FINAL)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(int iconSize2 READ iconSize2 WRITE setIconSize2 NOTIFY iconSize2Changed FINAL)

public:
    explicit StandardSystemButton(QWidget *parent = nullptr);
    explicit StandardSystemButton(const Global::SystemButtonType type, QWidget *parent = nullptr);
    ~StandardSystemButton() override;

    Q_NODISCARD QSize sizeHint() const override;
    Q_NODISCARD Global::SystemButtonType buttonType();
    Q_NODISCARD QString glyph() const;
    Q_NODISCARD bool isHovered() const;
    Q_NODISCARD bool isPressed() const;
    Q_NODISCARD QColor hoverColor() const;
    Q_NODISCARD QColor pressColor() const;
    Q_NODISCARD QColor normalColor() const;
    Q_NODISCARD QColor activeForegroundColor() const;
    Q_NODISCARD QColor inactiveForegroundColor() const;
    Q_NODISCARD bool isActive() const;
    Q_NODISCARD int iconSize2() const;

public Q_SLOTS:
    void setButtonType(const Global::SystemButtonType value);
    void setGlyph(const QString &glyph);
    void setHovered(const bool value);
    void setPressed(const bool value);
    void setHoverColor(const QColor &value);
    void setPressColor(const QColor &value);
    void setNormalColor(const QColor &value);
    void setActiveForegroundColor(const QColor &value);
    void setInactiveForegroundColor(const QColor &value);
    void setActive(const bool value);
    void setIconSize2(const int value);

protected:
    void enterEvent(QT_ENTER_EVENT_TYPE *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void buttonTypeChanged();
    void glyphChanged();
    void hoveredChanged();
    void pressedChanged();
    void hoverColorChanged();
    void pressColorChanged();
    void normalColorChanged();
    void activeForegroundColorChanged();
    void inactiveForegroundColorChanged();
    void activeChanged();
    void iconSize2Changed();

private:
    QScopedPointer<StandardSystemButtonPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE
