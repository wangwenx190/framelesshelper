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
#include <QtGui/qevent.h>
#include <QtWidgets/qtooltip.h>
#include <framelessmanager_p.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcStandardSystemButton, "wangwenx190.framelesshelper.widgets.standardsystembutton")

#ifdef FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcStandardSystemButton)
#  define DEBUG qCDebug(lcStandardSystemButton)
#  define WARNING qCWarning(lcStandardSystemButton)
#  define CRITICAL qCCritical(lcStandardSystemButton)
#endif

using namespace Global;

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

QString StandardSystemButtonPrivate::getCode() const
{
    return m_code;
}

void StandardSystemButtonPrivate::setCode(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (m_code == value) {
        return;
    }
    m_code = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->codeChanged();
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
    setCode(Utils::getSystemButtonIconCode(m_buttonType));
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

QColor StandardSystemButtonPrivate::getNormalColor() const
{
    return m_normalColor;
}

QColor StandardSystemButtonPrivate::getActiveForegroundColor() const
{
    return m_activeForegroundColor;
}

QColor StandardSystemButtonPrivate::getInactiveForegroundColor() const
{
    return m_inactiveForegroundColor;
}

bool StandardSystemButtonPrivate::isActive() const
{
    return m_active;
}

int StandardSystemButtonPrivate::iconSize2() const
{
    return m_iconSize2.value_or(FramelessManagerPrivate::getIconFont().pointSize());
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
            const auto yPos = [q]() -> int {
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

void StandardSystemButtonPrivate::setNormalColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_normalColor == value) {
        return;
    }
    m_normalColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->normalColorChanged();
}

void StandardSystemButtonPrivate::setActiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_activeForegroundColor == value) {
        return;
    }
    m_activeForegroundColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->activeForegroundColorChanged();
}

void StandardSystemButtonPrivate::setInactiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_inactiveForegroundColor == value) {
        return;
    }
    m_inactiveForegroundColor = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->inactiveForegroundColorChanged();
}

void StandardSystemButtonPrivate::setActive(const bool value)
{
    if (m_active == value) {
        return;
    }
    m_active = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->activeChanged();
}

void StandardSystemButtonPrivate::setIconSize2(const int value)
{
    Q_ASSERT(value > 0);
    if (value <= 0) {
        return;
    }
    if (iconSize2() == value) {
        return;
    }
    m_iconSize2 = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->iconSize2Changed();
}

void StandardSystemButtonPrivate::enterEventHandler(QT_ENTER_EVENT_TYPE *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    setHovered(true);
    event->accept();
}

void StandardSystemButtonPrivate::leaveEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    setHovered(false);
    event->accept();
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
    const auto backgroundColor = [this]() -> QColor {
        // The pressed state has higher priority than the hovered state.
        if (m_pressed && m_pressColor.isValid()) {
            return m_pressColor;
        }
        if (m_hovered && m_hoverColor.isValid()) {
            return m_hoverColor;
        }
        if (m_normalColor.isValid()) {
            return m_normalColor;
        }
        return {};
    }();
    const QRect buttonRect = {QPoint(0, 0), q->size()};
    if (backgroundColor.isValid()) {
        painter.fillRect(buttonRect, backgroundColor);
    }
    if (!m_code.isEmpty()) {
        painter.setPen([this]() -> QColor {
            if (!m_hovered && !m_active && m_inactiveForegroundColor.isValid()) {
                return m_inactiveForegroundColor;
            }
            if (m_activeForegroundColor.isValid()) {
                return m_activeForegroundColor;
            }
            return kDefaultBlackColor;
        }());
        painter.setFont([this]() -> QFont {
            QFont font = FramelessManagerPrivate::getIconFont();
            if (m_iconSize2.has_value()) {
                font.setPointSize(m_iconSize2.value());
            }
            return font;
        }());
        painter.drawText(buttonRect, Qt::AlignCenter, m_code);
    }
    painter.restore();
    event->accept();
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
}

StandardSystemButton::StandardSystemButton(QWidget *parent)
    : QAbstractButton(parent), d_ptr(new StandardSystemButtonPrivate(this))
{
}

StandardSystemButton::StandardSystemButton(const SystemButtonType type, QWidget *parent)
    : StandardSystemButton(parent)
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

QString StandardSystemButton::code() const
{
    Q_D(const StandardSystemButton);
    return d->getCode();
}

void StandardSystemButton::setButtonType(const SystemButtonType value)
{
    Q_D(StandardSystemButton);
    d->setButtonType(value);
}

void StandardSystemButton::setCode(const QString &code)
{
    Q_D(StandardSystemButton);
    d->setCode(code);
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

QColor StandardSystemButton::normalColor() const
{
    Q_D(const StandardSystemButton);
    return d->getNormalColor();
}

QColor StandardSystemButton::activeForegroundColor() const
{
    Q_D(const StandardSystemButton);
    return d->getActiveForegroundColor();
}

QColor StandardSystemButton::inactiveForegroundColor() const
{
    Q_D(const StandardSystemButton);
    return d->getInactiveForegroundColor();
}

bool StandardSystemButton::isActive() const
{
    Q_D(const StandardSystemButton);
    return d->isActive();
}

int StandardSystemButton::iconSize2() const
{
    Q_D(const StandardSystemButton);
    return d->iconSize2();
}

void StandardSystemButton::setPressColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setPressColor(value);
}

void StandardSystemButton::setNormalColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setNormalColor(value);
}

void StandardSystemButton::setActiveForegroundColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setActiveForegroundColor(value);
}

void StandardSystemButton::setInactiveForegroundColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setInactiveForegroundColor(value);
}

void StandardSystemButton::setActive(const bool value)
{
    Q_D(StandardSystemButton);
    d->setActive(value);
}

void StandardSystemButton::setIconSize2(const int value)
{
    Q_D(StandardSystemButton);
    d->setIconSize2(value);
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
