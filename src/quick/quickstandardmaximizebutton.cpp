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

#include "quickstandardmaximizebutton_p.h"
#include "framelessquickutils.h"
#include <QtGui/qguiapplication.h>
#include <QtGui/qstylehints.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelperquick);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(DarkMaxUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-maximize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightMaxUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-maximize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(DarkRestoreUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-restore.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightRestoreUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-restore.svg")

QuickStandardMaximizeButton::QuickStandardMaximizeButton(QQuickItem *parent) : QQuickButton(parent)
{
    initialize();
}

QuickStandardMaximizeButton::~QuickStandardMaximizeButton() = default;

bool QuickStandardMaximizeButton::isMaximized() const
{
    return m_max;
}

void QuickStandardMaximizeButton::setMaximized(const bool max)
{
    if (m_max == max) {
        return;
    }
    m_max = max;
    Q_EMIT maximizedChanged();
}

void QuickStandardMaximizeButton::updateForeground()
{
    const bool dark = (FramelessQuickUtils::darkModeEnabled() || FramelessQuickUtils::titleBarColorized());
    const auto url = QUrl(dark ? (m_max ? kDarkRestoreUrl : kDarkMaxUrl) : (m_max ? kLightRestoreUrl : kLightMaxUrl));
    initResource();
    m_image->setSource(url);
}

void QuickStandardMaximizeButton::updateBackground()
{
    const SystemButtonType button = (m_max ? SystemButtonType::Restore : SystemButtonType::Maximize);
    const ButtonState state = (isPressed() ? ButtonState::Pressed : ButtonState::Hovered);
    const bool visible = (isHovered() || isPressed());
    m_backgroundItem->setColor(FramelessQuickUtils::getSystemButtonBackgroundColor(button, state));
    m_backgroundItem->setVisible(visible);
}

void QuickStandardMaximizeButton::updateToolTip()
{
    const bool visible = (isHovered() && !isPressed());
    const int delay = QGuiApplication::styleHints()->mousePressAndHoldInterval();
    const QString text = (m_max ? tr("Restore") : tr("Maximize"));
    m_tooltip->setVisible(visible);
    m_tooltip->setDelay(delay);
    m_tooltip->setText(text);
}

void QuickStandardMaximizeButton::initialize()
{
    setImplicitWidth(kDefaultSystemButtonSize.width());
    setImplicitHeight(kDefaultSystemButtonSize.height());

    m_contentItem.reset(new QQuickItem(this));
    m_contentItem->setImplicitWidth(kDefaultSystemButtonIconSize.width());
    m_contentItem->setImplicitHeight(kDefaultSystemButtonIconSize.height());
    m_image.reset(new QQuickImage(m_contentItem.data()));
    const auto imageAnchors = new QQuickAnchors(m_image.data(), m_image.data());
    imageAnchors->setCenterIn(m_contentItem.data());
    const FramelessQuickUtils * const utils = FramelessQuickUtils::instance();
    connect(utils, &FramelessQuickUtils::darkModeEnabledChanged, this, &QuickStandardMaximizeButton::updateForeground);
    connect(utils, &FramelessQuickUtils::titleBarColorizedChanged, this, &QuickStandardMaximizeButton::updateForeground);
    connect(this, &QuickStandardMaximizeButton::maximizedChanged, this, &QuickStandardMaximizeButton::updateForeground);

    m_backgroundItem.reset(new QQuickRectangle(this));
    QQuickPen * const border = m_backgroundItem->border();
    border->setWidth(0.0);
    border->setColor(kDefaultTransparentColor);
    connect(this, &QuickStandardMaximizeButton::hoveredChanged, this, &QuickStandardMaximizeButton::updateBackground);
    connect(this, &QuickStandardMaximizeButton::pressedChanged, this, &QuickStandardMaximizeButton::updateBackground);

    m_tooltip.reset(new QQuickToolTip(this));
    connect(QGuiApplication::styleHints(), &QStyleHints::mousePressAndHoldIntervalChanged, this, [this](int interval){
        Q_UNUSED(interval);
        updateToolTip();
    });
    connect(this, &QuickStandardMaximizeButton::hoveredChanged, this, &QuickStandardMaximizeButton::updateToolTip);
    connect(this, &QuickStandardMaximizeButton::pressedChanged, this, &QuickStandardMaximizeButton::updateToolTip);
    connect(this, &QuickStandardMaximizeButton::maximizedChanged, this, &QuickStandardMaximizeButton::updateToolTip);

    updateBackground();
    updateForeground();
    updateToolTip();

    setContentItem(m_contentItem.data());
    setBackground(m_backgroundItem.data());
}

FRAMELESSHELPER_END_NAMESPACE
