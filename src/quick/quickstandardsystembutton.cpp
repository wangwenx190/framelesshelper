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
#include <framelessmanager.h>
#include <utils.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelperquick);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(DarkMinimizeUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-minimize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightMinimizeUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-minimize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(DarkMaximizeUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-maximize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightMaximizeUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-maximize.svg")
FRAMELESSHELPER_STRING_CONSTANT2(DarkRestoreUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-restore.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightRestoreUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-restore.svg")
FRAMELESSHELPER_STRING_CONSTANT2(DarkCloseUrl, "qrc:///org.wangwenx190.FramelessHelper/images/dark/chrome-close.svg")
FRAMELESSHELPER_STRING_CONSTANT2(LightCloseUrl, "qrc:///org.wangwenx190.FramelessHelper/images/light/chrome-close.svg")

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
    const QUrl url = [this]() -> QUrl {
        const bool dark = ((Utils::shouldAppsUseDarkMode() || Utils::isTitleBarColorized() || ((m_buttonType == QuickGlobal::SystemButtonType::Close) && isHovered())) && !m_forceLightTheme);
        switch (m_buttonType) {
        case QuickGlobal::SystemButtonType::Minimize:
            return QUrl(dark ? kDarkMinimizeUrl : kLightMinimizeUrl);
        case QuickGlobal::SystemButtonType::Maximize:
            return QUrl(dark ? kDarkMaximizeUrl : kLightMaximizeUrl);
        case QuickGlobal::SystemButtonType::Restore:
            return QUrl(dark ? kDarkRestoreUrl : kLightRestoreUrl);
        case QuickGlobal::SystemButtonType::Close:
            return QUrl(dark ? kDarkCloseUrl : kLightCloseUrl);
        case QuickGlobal::SystemButtonType::WindowIcon:
            Q_FALLTHROUGH();
        case QuickGlobal::SystemButtonType::Help:
            Q_FALLTHROUGH();
        case QuickGlobal::SystemButtonType::Unknown:
            Q_ASSERT(false);
            return {};
        }
        Q_UNREACHABLE();
        return {};
    }();
    if (m_contentItem->source() == url) {
        return;
    }
    initResource();
    m_contentItem->setSource(url);
}

void QuickStandardSystemButton::updateBackground()
{
    const bool hover = isHovered();
    const bool press = isPressed();
    m_backgroundItem->setColor(Utils::calculateSystemButtonBackgroundColor(
           FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, m_buttonType),
              (press ? ButtonState::Pressed : ButtonState::Hovered)));
    m_backgroundItem->setVisible(hover || press);
    checkInactive();
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(this))->setVisible(hover);
}

void QuickStandardSystemButton::setInactive(const bool value)
{
    const bool force = (value && Utils::isTitleBarColorized() && !Utils::shouldAppsUseDarkMode());
    if (m_forceLightTheme == force) {
        return;
    }
    m_forceLightTheme = force;
    m_shouldCheck = m_forceLightTheme;
    updateForeground();
}

void QuickStandardSystemButton::checkInactive()
{
    if (!m_shouldCheck) {
        return;
    }
    m_forceLightTheme = m_checkFlag;
    m_checkFlag = !m_checkFlag;
    updateForeground();
}

void QuickStandardSystemButton::initialize()
{
    setImplicitWidth(kDefaultSystemButtonSize.width());
    setImplicitHeight(kDefaultSystemButtonSize.height());

    m_contentItem.reset(new QQuickImage(this));
    m_contentItem->setFillMode(QQuickImage::Pad); // Don't apply any transformation to the image.
    m_contentItem->setSmooth(true); // Renders better when scaling up.
    m_contentItem->setMipmap(true); // Renders better when scaling down.
    m_contentItem->setWidth(kDefaultSystemButtonIconSize.width());
    m_contentItem->setHeight(kDefaultSystemButtonIconSize.height());
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged, this, &QuickStandardSystemButton::updateForeground);

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

FRAMELESSHELPER_END_NAMESPACE
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
