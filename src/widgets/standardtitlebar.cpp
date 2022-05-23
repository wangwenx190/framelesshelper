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

#include "standardtitlebar.h"
#include "standardtitlebar_p.h"
#include "standardsystembutton.h"
#include "standardsystembutton_p.h"
#include <QtCore/qcoreevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>
#include <framelessmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(StyleSheetColorTemplate, "color: %1;")
FRAMELESSHELPER_STRING_CONSTANT2(StyleSheetBackgroundColorTemplate, "background-color: %1;")

StandardTitleBarPrivate::StandardTitleBarPrivate(StandardTitleBar *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

StandardTitleBarPrivate::~StandardTitleBarPrivate() = default;

StandardTitleBarPrivate *StandardTitleBarPrivate::get(StandardTitleBar *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const StandardTitleBarPrivate *StandardTitleBarPrivate::get(const StandardTitleBar *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

Qt::Alignment StandardTitleBarPrivate::titleLabelAlignment() const
{
    return m_labelAlignment;
}

void StandardTitleBarPrivate::setTitleLabelAlignment(const Qt::Alignment value)
{
    if (m_labelAlignment == value) {
        return;
    }
    m_labelAlignment = value;
    bool needsInvalidate = false;
    if (m_labelAlignment & Qt::AlignLeft) {
        m_labelLeftStretch->changeSize(kDefaultTitleBarContentsMargin, 0, QSizePolicy::Fixed);
        m_labelRightStretch->changeSize(0, 0, QSizePolicy::Expanding);
        needsInvalidate = true;
    }
    if (m_labelAlignment & Qt::AlignRight) {
        m_labelLeftStretch->changeSize(0, 0, QSizePolicy::Expanding);
        m_labelRightStretch->changeSize(kDefaultTitleBarContentsMargin, 0, QSizePolicy::Fixed);
        needsInvalidate = true;
    }
    if (m_labelAlignment & Qt::AlignHCenter) {
        m_labelLeftStretch->changeSize(0, 0, QSizePolicy::Expanding);
        m_labelRightStretch->changeSize(0, 0, QSizePolicy::Expanding);
        needsInvalidate = true;
    }
    Q_Q(StandardTitleBar);
    if (needsInvalidate) {
        // Tell the layout manager that we have changed the layout item's size
        // manually to let it refresh the layout immediately.
        q->layout()->invalidate();
    }
    Q_EMIT q->titleLabelAlignmentChanged();
}

bool StandardTitleBarPrivate::isExtended() const
{
    return m_extended;
}

void StandardTitleBarPrivate::setExtended(const bool value)
{
    if (m_extended == value) {
        return;
    }
    m_extended = value;
    Q_Q(StandardTitleBar);
    q->setFixedHeight(m_extended ? kDefaultExtendedTitleBarHeight : kDefaultTitleBarHeight);
    Q_EMIT q->extendedChanged();
}

bool StandardTitleBarPrivate::isUsingAlternativeBackground() const
{
    return m_useAlternativeBackground;
}

void StandardTitleBarPrivate::setUseAlternativeBackground(const bool value)
{
    if (m_useAlternativeBackground == value) {
        return;
    }
    m_useAlternativeBackground = value;
    Q_Q(StandardTitleBar);
    Q_EMIT q->useAlternativeBackgroundChanged();
}

bool StandardTitleBarPrivate::isHideWhenClose() const
{
    return m_hideWhenClose;
}

void StandardTitleBarPrivate::setHideWhenClose(const bool value)
{
    if (m_hideWhenClose == value) {
        return;
    }
    m_hideWhenClose = value;
    Q_Q(StandardTitleBar);
    Q_EMIT q->hideWhenCloseChanged();
}

void StandardTitleBarPrivate::updateMaximizeButton()
{
    const bool max = m_window->isMaximized();
    m_maximizeButton->setButtonType(max ? SystemButtonType::Restore : SystemButtonType::Maximize);
    m_maximizeButton->setToolTip(max ? tr("Restore") : tr("Maximize"));
}

void StandardTitleBarPrivate::updateTitleBarStyleSheet()
{
    const bool active = m_window->isActiveWindow();
    const bool dark = Utils::shouldAppsUseDarkMode();
    const bool colorizedTitleBar = Utils::isTitleBarColorized();
    const QColor titleBarBackgroundColor = [active, colorizedTitleBar, dark]() -> QColor {
        if (active) {
            if (colorizedTitleBar) {
#ifdef Q_OS_WINDOWS
                return Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
                return Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
                return Utils::getControlsAccentColor();
#endif
            } else {
                if (dark) {
                    return kDefaultBlackColor;
                } else {
                    return kDefaultWhiteColor;
                }
            }
        } else {
            if (dark) {
                return kDefaultSystemDarkColor;
            } else {
                return kDefaultWhiteColor;
            }
        }
    }();
    const QColor windowTitleLabelTextColor = (active ? ((dark || colorizedTitleBar) ? kDefaultWhiteColor : kDefaultBlackColor) : kDefaultDarkGrayColor);
    m_windowTitleLabel->setStyleSheet(kStyleSheetColorTemplate.arg(windowTitleLabelTextColor.name()));
    StandardSystemButtonPrivate::get(m_minimizeButton.data())->setInactive(!active);
    StandardSystemButtonPrivate::get(m_maximizeButton.data())->setInactive(!active);
    StandardSystemButtonPrivate::get(m_closeButton.data())->setInactive(!active);
    if (!m_useAlternativeBackground) {
        Q_Q(StandardTitleBar);
        q->setStyleSheet(kStyleSheetBackgroundColorTemplate.arg(titleBarBackgroundColor.name()));
    }
}

void StandardTitleBarPrivate::retranslateUi()
{
    m_minimizeButton->setToolTip(tr("Minimize"));
    m_maximizeButton->setToolTip(m_window->isMaximized() ? tr("Restore") : tr("Maximize"));
    m_closeButton->setToolTip(tr("Close"));
}

bool StandardTitleBarPrivate::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!object->isWidgetType()) {
        return QObject::eventFilter(object, event);
    }
    const auto widget = qobject_cast<QWidget *>(object);
    if (!widget->isWindow() || (widget != m_window)) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::WindowStateChange:
        updateMaximizeButton();
        break;
    case QEvent::ActivationChange:
        updateTitleBarStyleSheet();
        break;
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void StandardTitleBarPrivate::initialize()
{
    Q_Q(StandardTitleBar);
    m_window = (q->nativeParentWidget() ? q->nativeParentWidget() : q->window());
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    q->setFixedHeight(kDefaultTitleBarHeight);
    m_windowTitleLabel.reset(new QLabel(q));
    m_windowTitleLabel->setFrameShape(QFrame::NoFrame);
    QFont windowTitleFont = q->font();
    windowTitleFont.setPointSize(kDefaultTitleBarFontPointSize);
    m_windowTitleLabel->setFont(windowTitleFont);
    m_windowTitleLabel->setText(m_window->windowTitle());
    connect(m_window, &QWidget::windowTitleChanged, m_windowTitleLabel.data(), &QLabel::setText);
    m_minimizeButton.reset(new StandardSystemButton(SystemButtonType::Minimize, q));
    connect(m_minimizeButton.data(), &StandardSystemButton::clicked, m_window, &QWidget::showMinimized);
    m_maximizeButton.reset(new StandardSystemButton(SystemButtonType::Maximize, q));
    updateMaximizeButton();
    connect(m_maximizeButton.data(), &StandardSystemButton::clicked, this, [this](){
        if (m_window->isMaximized()) {
            m_window->showNormal();
        } else {
            m_window->showMaximized();
        }
    });
    m_closeButton.reset(new StandardSystemButton(SystemButtonType::Close, q));
    connect(m_closeButton.data(), &StandardSystemButton::clicked, this, [this](){
        if (m_hideWhenClose) {
            m_window->hide();
        } else {
            m_window->close();
        }
    });
    m_labelLeftStretch = new QSpacerItem(0, 0);
    m_labelRightStretch = new QSpacerItem(0, 0);
    const auto titleLabelLayout = new QHBoxLayout;
    titleLabelLayout->setSpacing(0);
    titleLabelLayout->setContentsMargins(0, 0, 0, 0);
    titleLabelLayout->addSpacerItem(m_labelLeftStretch);
    titleLabelLayout->addWidget(m_windowTitleLabel.data());
    titleLabelLayout->addSpacerItem(m_labelRightStretch);
    // According to the title bar design guidance, the system buttons should always be
    // placed on the top-right corner of the window, so we need the following additional
    // layouts to ensure this.
    const auto systemButtonsInnerLayout = new QHBoxLayout;
    systemButtonsInnerLayout->setSpacing(0);
    systemButtonsInnerLayout->setContentsMargins(0, 0, 0, 0);
    systemButtonsInnerLayout->addWidget(m_minimizeButton.data());
    systemButtonsInnerLayout->addWidget(m_maximizeButton.data());
    systemButtonsInnerLayout->addWidget(m_closeButton.data());
    const auto systemButtonsOuterLayout = new QVBoxLayout;
    systemButtonsOuterLayout->setSpacing(0);
    systemButtonsOuterLayout->setContentsMargins(0, 0, 0, 0);
    systemButtonsOuterLayout->addLayout(systemButtonsInnerLayout);
    systemButtonsOuterLayout->addStretch();
    const auto titleBarLayout = new QHBoxLayout(q);
    titleBarLayout->setSpacing(0);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->addLayout(titleLabelLayout);
    titleBarLayout->addLayout(systemButtonsOuterLayout);
    q->setLayout(titleBarLayout);
    setTitleLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    retranslateUi();
    updateTitleBarStyleSheet();
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged,
                              this, &StandardTitleBarPrivate::updateTitleBarStyleSheet);
    m_window->installEventFilter(this);
}

StandardTitleBar::StandardTitleBar(QWidget *parent)
    : QWidget(parent), d_ptr(new StandardTitleBarPrivate(this))
{
}

StandardTitleBar::~StandardTitleBar() = default;

Qt::Alignment StandardTitleBar::titleLabelAlignment() const
{
    Q_D(const StandardTitleBar);
    return d->titleLabelAlignment();
}

void StandardTitleBar::setTitleLabelAlignment(const Qt::Alignment value)
{
    Q_D(StandardTitleBar);
    d->setTitleLabelAlignment(value);
}

QLabel *StandardTitleBar::titleLabel() const
{
    Q_D(const StandardTitleBar);
    return d->m_windowTitleLabel.data();
}

StandardSystemButton *StandardTitleBar::minimizeButton() const
{
    Q_D(const StandardTitleBar);
    return d->m_minimizeButton.data();
}

StandardSystemButton *StandardTitleBar::maximizeButton() const
{
    Q_D(const StandardTitleBar);
    return d->m_maximizeButton.data();
}

StandardSystemButton *StandardTitleBar::closeButton() const
{
    Q_D(const StandardTitleBar);
    return d->m_closeButton.data();
}

bool StandardTitleBar::isExtended() const
{
    Q_D(const StandardTitleBar);
    return d->isExtended();
}

void StandardTitleBar::setExtended(const bool value)
{
    Q_D(StandardTitleBar);
    d->setExtended(value);
}

bool StandardTitleBar::isUsingAlternativeBackground() const
{
    Q_D(const StandardTitleBar);
    return d->isUsingAlternativeBackground();
}

void StandardTitleBar::setUseAlternativeBackground(const bool value)
{
    Q_D(StandardTitleBar);
    d->setUseAlternativeBackground(value);
}

bool StandardTitleBar::isHideWhenClose() const
{
    Q_D(const StandardTitleBar);
    return d->isHideWhenClose();
}

void StandardTitleBar::setHideWhenClose(const bool value)
{
    Q_D(StandardTitleBar);
    d->setHideWhenClose(value);
}

void StandardTitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // This block of code ensures that our widget can still apply the stylesheet correctly.
    // Enabling the "Qt::WA_StyledBackground" attribute can also achieve the same
    // effect, but since it's documented as only for internal uses, we use the
    // public way to do that instead.
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

FRAMELESSHELPER_END_NAMESPACE
