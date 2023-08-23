/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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
#include <FramelessHelper/Core/utils.h>
#include <FramelessHelper/Core/private/framelessmanager_p.h>
#include <QtCore/qloggingcategory.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtooltip.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcStandardSystemButton, "wangwenx190.framelesshelper.widgets.standardsystembutton")

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

QString StandardSystemButtonPrivate::getGlyph() const
{
    return m_glyph;
}

void StandardSystemButtonPrivate::setGlyph(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (m_glyph == value) {
        return;
    }
    m_glyph = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->glyphChanged();
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
    setGlyph(Utils::getSystemButtonGlyph(m_buttonType));
    Q_Q(StandardSystemButton);
    q->update();
}

QSize StandardSystemButtonPrivate::getRecommendedButtonSize() const
{
    return kDefaultSystemButtonSize;
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

int StandardSystemButtonPrivate::glyphSize() const
{
    return m_glyphSize.value_or(FramelessManagerPrivate::getIconFont().pointSize());
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

void StandardSystemButtonPrivate::setGlyphSize(const int value)
{
    Q_ASSERT(value > 0);
    if (value <= 0) {
        return;
    }
    if (glyphSize() == value) {
        return;
    }
    m_glyphSize = value;
    Q_Q(StandardSystemButton);
    q->update();
    Q_EMIT q->glyphSizeChanged();
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
    const auto backgroundColor = [this, q]() -> QColor {
        // The pressed state has higher priority than the hovered state.
        if (q->isDown() && m_pressColor.isValid()) {
            return m_pressColor;
        }
        if (q->underMouse() && m_hoverColor.isValid()) {
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
    if (!m_glyph.isEmpty()) {
        painter.setPen([this, q]() -> QColor {
            if (!q->underMouse() && !m_active && m_inactiveForegroundColor.isValid()) {
                return m_inactiveForegroundColor;
            }
            if (m_activeForegroundColor.isValid()) {
                return m_activeForegroundColor;
            }
            return kDefaultBlackColor;
        }());
        painter.setFont([this]() -> QFont {
            QFont font = FramelessManagerPrivate::getIconFont();
            if (m_glyphSize.has_value()) {
                font.setPointSize(m_glyphSize.value());
            }
            return font;
        }());
        painter.drawText(buttonRect, Qt::AlignCenter, m_glyph);
    }
    painter.restore();
    event->accept();
}

void StandardSystemButtonPrivate::initialize()
{
    FramelessManagerPrivate::initializeIconFont();
    Q_Q(StandardSystemButton);
    q->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    q->setFixedSize(getRecommendedButtonSize());
    q->setIconSize(kDefaultSystemButtonIconSize);
    q->setMouseTracking(true);
    q->setAttribute(Qt::WA_Hover);
}

StandardSystemButton::StandardSystemButton(QWidget *parent)
    : QPushButton(parent), d_ptr(new StandardSystemButtonPrivate(this))
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

QString StandardSystemButton::glyph() const
{
    Q_D(const StandardSystemButton);
    return d->getGlyph();
}

void StandardSystemButton::setButtonType(const SystemButtonType value)
{
    Q_D(StandardSystemButton);
    d->setButtonType(value);
}

void StandardSystemButton::setGlyph(const QString &glyph)
{
    Q_D(StandardSystemButton);
    d->setGlyph(glyph);
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

int StandardSystemButton::glyphSize() const
{
    Q_D(const StandardSystemButton);
    return d->glyphSize();
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

void StandardSystemButton::setGlyphSize(const int value)
{
    Q_D(StandardSystemButton);
    d->setGlyphSize(value);
}

void StandardSystemButton::paintEvent(QPaintEvent *event)
{
    Q_D(StandardSystemButton);
    d->paintEventHandler(event);
}

FRAMELESSHELPER_END_NAMESPACE
