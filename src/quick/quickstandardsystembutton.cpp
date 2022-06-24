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
#include <private/framelessmanager_p.h>
#include <utils.h>
#include <QtQuick/qquickwindow.h>
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
    updateForeground();
}

void QuickStandardSystemButton::updateForeground()
{
    if (m_buttonType == QuickGlobal::SystemButtonType::Unknown) {
        return;
    }
    const QString iconCode = Utils::getSystemButtonIconCode(
        FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, m_buttonType));
    if (m_contentItem->text() != iconCode) {
        m_contentItem->setText(iconCode);
    }
    const QColor iconColor = [this]() -> QColor {
        const bool active = [this]() -> bool {
            const QQuickWindow * const w = window();
            return (w ? w->isActive() : false);
        }();
        if (!active) {
            return kDefaultDarkGrayColor;
        }
        if (Utils::isTitleBarColorized()) {
            return kDefaultWhiteColor;
        }
        return kDefaultBlackColor;
    }();
    if (m_contentItem->color() != iconColor) {
        m_contentItem->setColor(iconColor);
    }
}

void QuickStandardSystemButton::updateBackground()
{
    const bool hover = isHovered();
    const bool press = isPressed();
    m_backgroundItem->setColor(Utils::calculateSystemButtonBackgroundColor(
           FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, m_buttonType),
              (press ? ButtonState::Pressed : ButtonState::Hovered)));
    m_backgroundItem->setVisible(hover || press);
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(this))->setVisible(hover);
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
    connect(this, &QuickStandardSystemButton::hoveredChanged, this, &QuickStandardSystemButton::updateForeground);
    connect(this, &QuickStandardSystemButton::pressedChanged, this, &QuickStandardSystemButton::updateBackground);

    updateBackground();
    updateForeground();

    setContentItem(m_contentItem.data());
    setBackground(m_backgroundItem.data());
}

void QuickStandardSystemButton::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickButton::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        if (m_windowActiveConnection) {
            disconnect(m_windowActiveConnection);
            m_windowActiveConnection = {};
        }
        m_windowActiveConnection = connect(value.window, &QQuickWindow::activeChanged,
             this, &QuickStandardSystemButton::updateForeground);
    }
}

FRAMELESSHELPER_END_NAMESPACE
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
