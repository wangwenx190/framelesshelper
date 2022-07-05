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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtQuickTemplates2/private/qquickbutton_p.h>

QT_BEGIN_NAMESPACE
class QQuickText;
class QQuickRectangle;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_QUICK_API QuickStandardSystemButton : public QQuickButton
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(StandardSystemButton)
#endif
    Q_DISABLE_COPY_MOVE(QuickStandardSystemButton)
    Q_PROPERTY(QuickGlobal::SystemButtonType buttonType READ buttonType WRITE setButtonType NOTIFY buttonTypeChanged FINAL)
    Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged FINAL)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged FINAL)
    Q_PROPERTY(QColor pressColor READ pressColor WRITE setPressColor NOTIFY pressColorChanged FINAL)
    Q_PROPERTY(QColor normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)

public:
    explicit QuickStandardSystemButton(QQuickItem *parent = nullptr);
    explicit QuickStandardSystemButton(const QuickGlobal::SystemButtonType type, QQuickItem *parent = nullptr);
    ~QuickStandardSystemButton() override;

    Q_NODISCARD QuickGlobal::SystemButtonType buttonType() const;
    Q_NODISCARD QString code() const;
    Q_NODISCARD QColor color() const;
    Q_NODISCARD QColor normalColor() const;
    Q_NODISCARD QColor hoverColor() const;
    Q_NODISCARD QColor pressColor() const;

public Q_SLOTS:
    void updateForeground();
    void updateBackground();
    void setButtonType(const QuickGlobal::SystemButtonType type);
    void setCode(const QString &value);
    void setColor(const QColor &value);
    void setNormalColor(const QColor &value);
    void setHoverColor(const QColor &value);
    void setPressColor(const QColor &value);

private:
    void initialize();

Q_SIGNALS:
    void buttonTypeChanged();
    void codeChanged();
    void colorChanged();
    void normalColorChanged();
    void hoverColorChanged();
    void pressColorChanged();

private:
    QScopedPointer<QQuickText> m_contentItem;
    QScopedPointer<QQuickRectangle> m_backgroundItem;
    QuickGlobal::SystemButtonType m_buttonType = QuickGlobal::SystemButtonType::Unknown;
    QString m_code = {};
    QColor m_color = {};
    QColor m_normalColor = {};
    QColor m_hoverColor = {};
    QColor m_pressColor = {};
};

FRAMELESSHELPER_END_NAMESPACE

QML_DECLARE_TYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(QuickStandardSystemButton))
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
