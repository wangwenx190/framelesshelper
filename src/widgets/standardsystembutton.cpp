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
#include <framelesswindowsmanager.h>
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
    const SystemTheme systemTheme = []() -> SystemTheme {
        if (Utils::isTitleBarColorized()) {
            return SystemTheme::Dark;
        }
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
    setHoverColor(Utils::calculateSystemButtonBackgroundColor(type, ButtonState::Hovered));
    setPressColor(Utils::calculateSystemButtonBackgroundColor(type, ButtonState::Pressed));
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

bool StandardSystemButtonPrivate::isHover() const
{
    return m_hovered;
}

QColor StandardSystemButtonPrivate::getHoverColor() const
{
    return m_hoverColor;
}

QColor StandardSystemButtonPrivate::getPressColor() const
{
    return m_pressColor;
}

void StandardSystemButtonPrivate::setHover(const bool value)
{
    if (m_hovered == value) {
        return;
    }
    m_hovered = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->hoverChanged();
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
    setHover(true);
}

void StandardSystemButtonPrivate::leaveEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    setHover(false);
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
        painter.drawPixmap(g_buttonIconX,
                           g_buttonIconY,
                           ((m_buttonType == SystemButtonType::Close)
                            && (m_buttonTheme == SystemTheme::Light) && m_hovered
                            && !m_reversedIcon.isNull())
                               ? m_reversedIcon
                               : m_icon);
    }
    painter.restore();
}

void StandardSystemButtonPrivate::initialize()
{
    Q_Q(StandardSystemButton);
    q->setFixedSize(kDefaultSystemButtonSize);
    q->setIconSize(kDefaultSystemButtonIconSize);
    connect(q, &StandardSystemButton::pressed, this, [this, q](){
        if (m_pressed) {
            return;
        }
        m_pressed = true;
        q->update();
    });
    connect(q, &StandardSystemButton::released, this, [this, q](){
        if (!m_pressed) {
            return;
        }
        m_pressed = false;
        q->update();
    });
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged,
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

void StandardSystemButton::setButtonType(const Global::SystemButtonType value)
{
    Q_D(StandardSystemButton);
    d->setButtonType(value);
}

bool StandardSystemButton::isHover() const
{
    Q_D(const StandardSystemButton);
    return d->isHover();
}

void StandardSystemButton::setHover(const bool value)
{
    Q_D(StandardSystemButton);
    d->setHover(value);
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
