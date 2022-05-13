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
#include <QtCore/qvariant.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qtooltip.h>
#include <framelessmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

static constexpr const QRect g_buttonRect = {QPoint(0, 0), kDefaultSystemButtonSize};
static constexpr const auto g_buttonIconX = static_cast<int>(qRound(qreal(kDefaultSystemButtonSize.width() - kDefaultSystemButtonIconSize.width()) / 2.0));
static constexpr const auto g_buttonIconY = static_cast<int>(qRound(qreal(kDefaultSystemButtonSize.height() - kDefaultSystemButtonIconSize.height()) / 2.0));

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

void StandardSystemButtonPrivate::refreshButtonTheme(const bool force)
{
    if (m_buttonType == SystemButtonType::Unknown) {
        return;
    }
    const SystemTheme systemTheme = [this]() -> SystemTheme {
        if (m_forceLightTheme) {
            return SystemTheme::Light;
        }
#ifdef Q_OS_WINDOWS
        if (Utils::isTitleBarColorized()) {
            return SystemTheme::Dark;
        }
#endif
        return Utils::getSystemTheme();
    }();
    if ((m_buttonTheme == systemTheme) && !force) {
        return;
    }
    m_buttonTheme = systemTheme;
    const SystemTheme reversedTheme = [this]() -> SystemTheme {
        if (m_buttonTheme == SystemTheme::Light) {
            return SystemTheme::Dark;
        }
        return SystemTheme::Light;
    }();
    // QPixmap doesn't support SVG images. Please refer to:
    // https://doc.qt.io/qt-6/qpixmap.html#reading-and-writing-image-files
    setImage(qvariant_cast<QImage>(Utils::getSystemButtonIconResource(m_buttonType, m_buttonTheme, ResourceType::Image)), false);
    setImage(qvariant_cast<QImage>(Utils::getSystemButtonIconResource(m_buttonType, reversedTheme, ResourceType::Image)), true);
    setHoverColor(Utils::calculateSystemButtonBackgroundColor(m_buttonType, ButtonState::Hovered));
    setPressColor(Utils::calculateSystemButtonBackgroundColor(m_buttonType, ButtonState::Pressed));
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
    refreshButtonTheme(true);
}

void StandardSystemButtonPrivate::setIcon(const QIcon &value, const bool reverse)
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return;
    }
    const QPixmap pixmap = value.pixmap(kDefaultSystemButtonIconSize);
    if (reverse) {
        m_reversedIcon = pixmap;
    } else {
        m_icon = pixmap;
    }
    Q_Q(StandardSystemButton);
    q->update();
}

void StandardSystemButtonPrivate::setPixmap(const QPixmap &value, const bool reverse)
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return;
    }
    const QPixmap pixmap = ((value.size() == kDefaultSystemButtonIconSize) ? value :
                  value.scaled(kDefaultSystemButtonIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    if (reverse) {
        m_reversedIcon = pixmap;
    } else {
        m_icon = pixmap;
    }
    Q_Q(StandardSystemButton);
    q->update();
}

void StandardSystemButtonPrivate::setImage(const QImage &value, const bool reverse)
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return;
    }
    const QImage image = ((value.size() == kDefaultSystemButtonIconSize) ? value :
                        value.scaled(kDefaultSystemButtonIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    const QPixmap pixmap = QPixmap::fromImage(image);
    if (reverse) {
        m_reversedIcon = pixmap;
    } else {
        m_icon = pixmap;
    }
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
                const auto h = qreal(q->height());
                if (const QWidget * const window = q->window()) {
                    if (Utils::windowStatesToWindowState(window->windowState()) == Qt::WindowMaximized) {
                        return int(qRound(h * 0.5));
                    }
                }
                return -int(qRound(h * 1.3));
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
    // Enabling "QPainter::SmoothPixmapTransform" will cause the painted image
    // looks blurry, strange.
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QColor color = {};
    // The pressed state has higher priority than the hovered state.
    if (m_pressed && m_pressColor.isValid()) {
        color = m_pressColor;
    } else if (m_hovered && m_hoverColor.isValid()) {
        color = m_hoverColor;
    }
    if (color.isValid()) {
        painter.fillRect(g_buttonRect, color);
    }
    if (!m_icon.isNull()) {
        painter.drawPixmap(g_buttonIconX, g_buttonIconY, [this]() -> QPixmap {
            if (m_reversedIcon.isNull()) {
                return m_icon;
            }
            if (m_hovered && (((m_buttonType == SystemButtonType::Close)
                  && (m_buttonTheme == SystemTheme::Light)) || m_forceLightTheme)) {
                return m_reversedIcon;
            }
            return m_icon;
        }());
    }
    painter.restore();
}

void StandardSystemButtonPrivate::setInactive(const bool value)
{
    const bool force = (value && Utils::isTitleBarColorized() && !Utils::shouldAppsUseDarkMode());
    if (m_forceLightTheme == force) {
        return;
    }
    m_forceLightTheme = force;
    m_shouldCheck = m_forceLightTheme;
    refreshButtonTheme(true);
}

void StandardSystemButtonPrivate::checkInactive()
{
    if (!m_shouldCheck) {
        return;
    }
    m_forceLightTheme = m_checkFlag;
    m_checkFlag = !m_checkFlag;
    refreshButtonTheme(true);
}

void StandardSystemButtonPrivate::initialize()
{
    Q_Q(StandardSystemButton);
    q->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    q->setFixedSize(kDefaultSystemButtonSize);
    q->setIconSize(kDefaultSystemButtonIconSize);
    connect(q, &StandardSystemButton::pressed, this, [this](){ setPressed(true); });
    connect(q, &StandardSystemButton::released, this, [this](){ setPressed(false); });
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged,
            this, [this](){ refreshButtonTheme(false); });
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

void StandardSystemButton::setIcon(const QIcon &icon)
{
    QAbstractButton::setIcon(icon);
    Q_D(StandardSystemButton);
    d->setIcon(icon, false);
}

SystemButtonType StandardSystemButton::buttonType()
{
    Q_D(const StandardSystemButton);
    return d->getButtonType();
}

void StandardSystemButton::setButtonType(const SystemButtonType value)
{
    Q_D(StandardSystemButton);
    d->setButtonType(value);
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

void StandardSystemButton::setPressColor(const QColor &value)
{
    Q_D(StandardSystemButton);
    d->setPressColor(value);
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
