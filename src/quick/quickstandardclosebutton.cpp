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

#include "quickstandardclosebutton_p.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <framelesswindowsmanager.h>
#include <utils.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickanchors_p.h>

static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelperquick);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(DarkUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-close.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-close.svg")

QuickStandardCloseButton::QuickStandardCloseButton(QQuickItem *parent) : QQuickButton(parent)
{
    initialize();
}

QuickStandardCloseButton::~QuickStandardCloseButton() = default;

void QuickStandardCloseButton::updateForeground()
{
    const bool dark = (Utils::shouldAppsUseDarkMode() || Utils::isTitleBarColorized());
    const auto url = QUrl((dark || isHovered() || isPressed()) ? kDarkUrl : kLightUrl);
    initResource();
    m_image->setSource(url);
}

void QuickStandardCloseButton::updateBackground()
{
    static constexpr const auto button = SystemButtonType::Close;
    const ButtonState state = (isPressed() ? ButtonState::Pressed : ButtonState::Hovered);
    const bool visible = (isHovered() || isPressed());
    m_backgroundItem->setColor(Utils::calculateSystemButtonBackgroundColor(button, state));
    m_backgroundItem->setVisible(visible);
}

void QuickStandardCloseButton::updateToolTip()
{
    m_tooltip->setVisible(isHovered() && !isPressed());
}

void QuickStandardCloseButton::initialize()
{
    setImplicitWidth(kDefaultSystemButtonSize.width());
    setImplicitHeight(kDefaultSystemButtonSize.height());

    m_contentItem.reset(new QQuickItem(this));
    m_contentItem->setImplicitWidth(kDefaultSystemButtonIconSize.width());
    m_contentItem->setImplicitHeight(kDefaultSystemButtonIconSize.height());
    m_image.reset(new QQuickImage(m_contentItem.data()));
    const auto imageAnchors = new QQuickAnchors(m_image.data(), m_image.data());
    imageAnchors->setCenterIn(m_contentItem.data());
    connect(this, &QuickStandardCloseButton::hoveredChanged, this, &QuickStandardCloseButton::updateForeground);
    connect(this, &QuickStandardCloseButton::pressedChanged, this, &QuickStandardCloseButton::updateForeground);
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, &QuickStandardCloseButton::updateForeground);

    m_backgroundItem.reset(new QQuickRectangle(this));
    QQuickPen * const border = m_backgroundItem->border();
    border->setWidth(0.0);
    border->setColor(kDefaultTransparentColor);
    connect(this, &QuickStandardCloseButton::hoveredChanged, this, &QuickStandardCloseButton::updateBackground);
    connect(this, &QuickStandardCloseButton::pressedChanged, this, &QuickStandardCloseButton::updateBackground);

    m_tooltip = qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(this));
    m_tooltip->setText(tr("Close"));
    m_tooltip->setDelay(0);
    connect(this, &QuickStandardCloseButton::hoveredChanged, this, &QuickStandardCloseButton::updateToolTip);
    connect(this, &QuickStandardCloseButton::pressedChanged, this, &QuickStandardCloseButton::updateToolTip);

    updateBackground();
    updateForeground();
    updateToolTip();

    setContentItem(m_contentItem.data());
    setBackground(m_backgroundItem.data());
}

FRAMELESSHELPER_END_NAMESPACE
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
