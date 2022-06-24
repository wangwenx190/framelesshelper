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

#include "standardsystembutton.h"
#include "standardsystembutton_p.h"
#include <QtGui/qpainter.h>
#include <QtWidgets/qtooltip.h>
#include <framelessmanager.h>
#include <private/framelessmanager_p.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

static constexpr const QRect g_buttonRect = {QPoint(0, 0), kDefaultSystemButtonSize};

StandardSystemButtonPrivate::StandardSystemButtonPrivate(StandardSystemButton *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

StandardSystemButtonPrivate::~StandardSystemButtonPrivate() = default;

StandardSystemButtonPrivate *StandardSystemButtonPrivate::get(StandardSystemButton *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const StandardSystemButtonPrivate *StandardSystemButtonPrivate::get(const StandardSystemButton *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

QString StandardSystemButtonPrivate::getIconCode() const
{
    return m_iconCode;
}

void StandardSystemButtonPrivate::setIconCode(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (m_iconCode == value) {
        return;
    }
    m_iconCode = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->iconCodeChanged();
}

SystemButtonType StandardSystemButtonPrivate::getButtonType() const
{
    return m_buttonType;
}

void StandardSystemButtonPrivate::setButtonType(const SystemButtonType type)
{
    Q_ASSERT(type != SystemButtonType::Unknown);
    if (type == SystemButtonType::Unknown) {
        return;
    }
    if (m_buttonType == type) {
        return;
    }
    m_buttonType = type;
    setIconCode(Utils::getSystemButtonIconCode(m_buttonType));
    updateBackgroundColor();
    Q_Q(StandardSystemButton);
    q->update();
}

QSize StandardSystemButtonPrivate::getRecommendedButtonSize() const
{
    return kDefaultSystemButtonSize;
}

bool StandardSystemButtonPrivate::isHovered() const
{
    return m_hovered;
}

bool StandardSystemButtonPrivate::isPressed() const
{
    return m_pressed;
}

QColor StandardSystemButtonPrivate::getHoverColor() const
{
    return m_hoverColor;
}

QColor StandardSystemButtonPrivate::getPressColor() const
{
    return m_pressColor;
}

QColor StandardSystemButtonPrivate::getIconColor() const
{
    return m_iconColor;
}

void StandardSystemButtonPrivate::setHovered(const bool value)
{
    if (m_hovered == value) {
        return;
    }
    m_hovered = value;
    Q_Q(StandardSystemButton);
    q->update();
    if (m_hovered) {
        const QString toolTip = q->toolTip();
        if (!toolTip.isEmpty() && !QToolTip::isVisible()) {
            const int yPos = [q]() -> int {
                static const int h = kDefaultSystemButtonSize.height();
                if (const QWidget * const window = q->window()) {
                    if (Utils::windowStatesToWindowState(window->windowState()) == Qt::WindowMaximized) {
                        return qRound(h * 0.5);
                    }
                }
                return -qRound(h * 1.3);
            }();
            QToolTip::showText(q->mapToGlobal(QPoint(-2, yPos)), toolTip, q, q->geometry());
        }
    } else {
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    }
    Q_EMIT q->hoveredChanged();
}

void StandardSystemButtonPrivate::setPressed(const bool value)
{
    if (m_pressed == value) {
        return;
    }
    m_pressed = value;
    Q_Q(StandardSystemButton);
    q->setDown(m_pressed);
    q->update();
    Q_EMIT q->pressedChanged();
    if (m_pressed) {
        Q_EMIT q->pressed();
    } else {
        Q_EMIT q->released();
    }
}

void StandardSystemButtonPrivate::setHoverColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_hoverColor == value) {
        return;
    }
    m_hoverColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->hoverColorChanged();
}

void StandardSystemButtonPrivate::setPressColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_pressColor == value) {
        return;
    }
    m_pressColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->pressColorChanged();
}

void StandardSystemButtonPrivate::setIconColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_iconColor == value) {
        return;
    }
    m_iconColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->iconColorChanged();
}

void StandardSystemButtonPrivate::updateBackgroundColor()
{
    if (m_buttonType == SystemButtonType::Unknown) {
        return;
    }
    setHoverColor(Utils::calculateSystemButtonBackgroundColor(m_buttonType, ButtonState::Hovered));
    setPressColor(Utils::calculateSystemButtonBackgroundColor(m_buttonType, ButtonState::Pressed));
}

void StandardSystemButtonPrivate::enterEventHandler(QT_ENTER_EVENT_TYPE *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    setHovered(true);
}

void StandardSystemButtonPrivate::leaveEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    setHovered(false);
}

void StandardSystemButtonPrivate::paintEventHandler(QPaintEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    Q_Q(StandardSystemButton);
    QPainter painter(q);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                           | QPainter::SmoothPixmapTransform);
    const QColor backgroundColor = [this]() -> QColor {
        // The pressed state has higher priority than the hovered state.
        if (m_pressed && m_pressColor.isValid()) {
            return m_pressColor;
        } else if (m_hovered && m_hoverColor.isValid()) {
            return m_hoverColor;
        }
        return {};
    }();
    if (backgroundColor.isValid()) {
        painter.fillRect(g_buttonRect, backgroundColor);
    }
    if (!m_iconCode.isEmpty()) {
        const QColor iconColor = [this, q]() -> QColor {
            if (m_iconColor.isValid()) {
                return m_iconColor;
            }
            const bool active = [q]() -> bool {
                const QWidget * const window = q->window();
                return (window ? window->isActiveWindow() : false);
            }();
            if (!active) {
                return kDefaultDarkGrayColor;
            }
            if (Utils::isTitleBarColorized()) {
                return kDefaultWhiteColor;
            }
            return kDefaultBlackColor;
        }();
        painter.setPen(iconColor);
        painter.setFont(FramelessManagerPrivate::getIconFont());
        painter.drawText(g_buttonRect, Qt::AlignCenter, m_iconCode);
    }
    painter.restore();
}

void StandardSystemButtonPrivate::initialize()
{
    FramelessManagerPrivate::initializeIconFont();
    Q_Q(StandardSystemButton);
    q->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    q->setFixedSize(kDefaultSystemButtonSize);
    q->setIconSize(kDefaultSystemButtonIconSize);
    connect(q, &StandardSystemButton::pressed, this, [this](){ setPressed(true); });
    connect(q, &StandardSystemButton::released, this, [this](){ setPressed(false); });
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged,
                this, [this](){ updateBackgroundColor(); });
}

StandardSystemButton::StandardSystemButton(QWidget *parent) : QAbstractButton(parent), d_ptr(new StandardSystemButtonPrivate(this))
{
}

StandardSystemButton::StandardSystemButton(const SystemButtonType type, QWidget *parent) : StandardSystemButton(parent)
{
    setButtonType(type);
}

StandardSystemButton::~StandardSystemButton() = default;

QSize StandardSystemButton::sizeHint() const
{
    Q_D(const StandardSystemButton);
    return d->getRecommendedButtonSize();
}

SystemButtonType StandardSystemButton::buttonType()
{
    Q_D(const StandardSystemButton);
    return d->getButtonType();
}

QString StandardSystemButton::iconCode() const
{
    Q_D(const StandardSystemButton);
    return d->getIconCode();
}

void StandardSystemButton::setButtonType(const SystemButtonType value)
{
    Q_D(StandardSystemButton);
    d->setButtonType(value);
}

void StandardSystemButton::setIconCode(const QString &code)
{
    Q_D(StandardSystemButton);
    d->setIconCode(code);
}

bool StandardSystemButton::isHovered() const
{
    Q_D(const StandardSystemButton);
    return d->isHovered();
}

void StandardSystemButton::setHovered(const bool value)
{
    Q_D(StandardSystemButton);
    d->setHovered(value);
}

bool StandardSystemButton::isPressed() const
{
    Q_D(const StandardSystemButton);
    return d->isPressed();
}

void StandardSystemButton::setPressed(const bool value)
{
    Q_D(StandardSystemButton);
    d->setPressed(value);
}

QColor StandardSystemButton::hoverColor() const
{
    Q_D(const StandardSystemButton);
    return d->getHoverColor();
}

void StandardSystemButton::setHoverColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setHoverColor(value);
}

QColor StandardSystemButton::pressColor() const
{
    Q_D(const StandardSystemButton);
    return d->getPressColor();
}

QColor StandardSystemButton::iconColor() const
{
    Q_D(const StandardSystemButton);
    return d->getIconColor();
}

void StandardSystemButton::setPressColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setPressColor(value);
}

void StandardSystemButton::setIconColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setIconColor(value);
}

void StandardSystemButton::enterEvent(QT_ENTER_EVENT_TYPE *event)
{
    QAbstractButton::enterEvent(event);
    Q_D(StandardSystemButton);
    d->enterEventHandler(event);
}

void StandardSystemButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    Q_D(StandardSystemButton);
    d->leaveEventHandler(event);
}

void StandardSystemButton::paintEvent(QPaintEvent *event)
{
    Q_D(StandardSystemButton);
    d->paintEventHandler(event);
}

FRAMELESSHELPER_END_NAMESPACE
