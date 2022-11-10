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
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquicktext_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickStandardSystemButton, "wangwenx190.framelesshelper.quick.quickstandardsystembutton")

#ifdef FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcQuickStandardSystemButton)
#  define DEBUG qCDebug(lcQuickStandardSystemButton)
#  define WARNING qCWarning(lcQuickStandardSystemButton)
#  define CRITICAL qCCritical(lcQuickStandardSystemButton)
#endif

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

QColor QuickStandardSystemButton::activeForegroundColor() const
{
    return m_activeForegroundColor;
}

QColor QuickStandardSystemButton::inactiveForegroundColor() const
{
    return m_inactiveForegroundColor;
}

qreal QuickStandardSystemButton::iconSize() const
{
    if (m_contentItem.isNull()) {
        return -1;
    }
    const QFont font = m_contentItem->font();
    const qreal point = font.pointSizeF();
    if (point > 0) {
        return point;
    }
    const int pixel = font.pixelSize();
    if (pixel > 0) {
        return pixel;
    }
    return -1;
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
    updateColor();
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
    updateColor();
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
    updateColor();
    Q_EMIT pressColorChanged();
}

void QuickStandardSystemButton::setActiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_activeForegroundColor == value) {
        return;
    }
    m_activeForegroundColor = value;
    updateColor();
    Q_EMIT activeForegroundColorChanged();
}

void QuickStandardSystemButton::setInactiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_inactiveForegroundColor == value) {
        return;
    }
    m_inactiveForegroundColor = value;
    updateColor();
    Q_EMIT inactiveForegroundColorChanged();
}

void QuickStandardSystemButton::setIconSize(const qreal value)
{
    Q_ASSERT(value > 0);
    if (qFuzzyIsNull(value) || (value < 0)) {
        return;
    }
    if (qFuzzyCompare(iconSize(), value)) {
        return;
    }
    QFont font = m_contentItem->font();
    font.setPointSizeF(value);
    m_contentItem->setFont(font);
    Q_EMIT iconSizeChanged();
}

void QuickStandardSystemButton::updateColor()
{
    const bool hover = isHovered();
    const bool press = isPressed();
    m_contentItem->setColor([this, hover]() -> QColor {
        const bool active = (window() ? window()->isActive() : false);
        if (!hover && !active && m_inactiveForegroundColor.isValid()) {
            return m_inactiveForegroundColor;
        }
        if (m_activeForegroundColor.isValid()) {
            return m_activeForegroundColor;
        }
        return kDefaultBlackColor;
    }());
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

    setAntialiasing(true);
    setSmooth(true);
    setClip(true);

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
    connect(this, &QuickStandardSystemButton::hoveredChanged, this, &QuickStandardSystemButton::updateColor);
    connect(this, &QuickStandardSystemButton::pressedChanged, this, &QuickStandardSystemButton::updateColor);

    updateColor();

    setContentItem(m_contentItem.data());
    setBackground(m_backgroundItem.data());
}

void QuickStandardSystemButton::classBegin()
{
    QQuickButton::classBegin();
}

void QuickStandardSystemButton::componentComplete()
{
    QQuickButton::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
