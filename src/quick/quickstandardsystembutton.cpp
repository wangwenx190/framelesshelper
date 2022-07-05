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

#include "quickstandardsystembutton_p.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <framelessmanager_p.h>
#include <utils.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquicktext_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

QuickStandardSystemButton::QuickStandardSystemButton(QQuickItem *parent) : QQuickButton(parent)
{
    initialize();
}

QuickStandardSystemButton::QuickStandardSystemButton(const QuickGlobal::SystemButtonType type, QQuickItem *parent) : QuickStandardSystemButton(parent)
{
    setButtonType(type);
}

QuickStandardSystemButton::~QuickStandardSystemButton() = default;

QuickGlobal::SystemButtonType QuickStandardSystemButton::buttonType() const
{
    return m_buttonType;
}

QString QuickStandardSystemButton::code() const
{
    return m_code;
}

QColor QuickStandardSystemButton::color() const
{
    return m_color;
}

QColor QuickStandardSystemButton::normalColor() const
{
    return m_normalColor;
}

QColor QuickStandardSystemButton::hoverColor() const
{
    return m_hoverColor;
}

QColor QuickStandardSystemButton::pressColor() const
{
    return m_pressColor;
}

void QuickStandardSystemButton::setButtonType(const QuickGlobal::SystemButtonType type)
{
    Q_ASSERT(type != QuickGlobal::SystemButtonType::Unknown);
    if (type == QuickGlobal::SystemButtonType::Unknown) {
        return;
    }
    if (m_buttonType == type) {
        return;
    }
    m_buttonType = type;
    setCode(Utils::getSystemButtonIconCode(
        FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, m_buttonType)));
    Q_EMIT buttonTypeChanged();
}

void QuickStandardSystemButton::setCode(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (m_code == value) {
        return;
    }
    m_code = value;
    m_contentItem->setText(m_code);
    Q_EMIT codeChanged();
}

void QuickStandardSystemButton::setColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_color == value) {
        return;
    }
    m_color = value;
    updateForeground();
    Q_EMIT colorChanged();
}

void QuickStandardSystemButton::setNormalColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_normalColor == value) {
        return;
    }
    m_normalColor = value;
    updateBackground();
    Q_EMIT normalColorChanged();
}

void QuickStandardSystemButton::setHoverColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_hoverColor == value) {
        return;
    }
    m_hoverColor = value;
    updateBackground();
    Q_EMIT hoverColorChanged();
}

void QuickStandardSystemButton::setPressColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_pressColor == value) {
        return;
    }
    m_pressColor = value;
    updateBackground();
    Q_EMIT pressColorChanged();
}

void QuickStandardSystemButton::updateForeground()
{
    m_contentItem->setColor(m_color.isValid() ? m_color : kDefaultBlackColor);
}

void QuickStandardSystemButton::updateBackground()
{
    const bool hover = isHovered();
    const bool press = isPressed();
    m_backgroundItem->setColor([this, hover, press]() -> QColor {
        if (press && m_pressColor.isValid()) {
            return m_pressColor;
        }
        if (hover && m_hoverColor.isValid()) {
            return m_hoverColor;
        }
        if (m_normalColor.isValid()) {
            return m_normalColor;
        }
        return kDefaultTransparentColor;
    }());
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(this))->setVisible(hover || press);
}

void QuickStandardSystemButton::initialize()
{
    FramelessManagerPrivate::initializeIconFont();

    setImplicitWidth(kDefaultSystemButtonSize.width());
    setImplicitHeight(kDefaultSystemButtonSize.height());

    m_contentItem.reset(new QQuickText(this));
    m_contentItem->setFont(FramelessManagerPrivate::getIconFont());
    m_contentItem->setHAlign(QQuickText::AlignHCenter);
    m_contentItem->setVAlign(QQuickText::AlignVCenter);
    QQuickItemPrivate::get(m_contentItem.data())->anchors()->setFill(this);

    m_backgroundItem.reset(new QQuickRectangle(this));
    QQuickPen * const border = m_backgroundItem->border();
    border->setWidth(0.0);
    border->setColor(kDefaultTransparentColor);
    connect(this, &QuickStandardSystemButton::hoveredChanged, this, &QuickStandardSystemButton::updateBackground);
    connect(this, &QuickStandardSystemButton::pressedChanged, this, &QuickStandardSystemButton::updateBackground);

    updateBackground();
    updateForeground();

    setContentItem(m_contentItem.data());
    setBackground(m_backgroundItem.data());
}

FRAMELESSHELPER_END_NAMESPACE
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
