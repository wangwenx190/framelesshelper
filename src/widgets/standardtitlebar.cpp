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
#include <QtCore/qcoreevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>
#include <framelesswindowsmanager.h>
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

void StandardTitleBarPrivate::updateMaximizeButton()
{
    const bool zoomed = (m_window->isMaximized() || m_window->isFullScreen());
    m_maximizeButton->setToolTip(zoomed ? tr("Restore") : tr("Maximize"));
    m_maximizeButton->setButtonType(zoomed ? SystemButtonType::Restore : SystemButtonType::Maximize);
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
    Q_Q(StandardTitleBar);
    q->setStyleSheet(kStyleSheetBackgroundColorTemplate.arg(titleBarBackgroundColor.name()));
    q->update();
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
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void StandardTitleBarPrivate::initialize()
{
    Q_Q(StandardTitleBar);
    m_window = q->window();
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
    m_minimizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_minimizeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_minimizeButton->setToolTip(tr("Minimize"));
    connect(m_minimizeButton.data(), &StandardSystemButton::clicked, m_window, &QWidget::showMinimized);
    m_maximizeButton.reset(new StandardSystemButton(SystemButtonType::Maximize, q));
    m_maximizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_maximizeButton->setIconSize(kDefaultSystemButtonIconSize);
    updateMaximizeButton();
    connect(m_maximizeButton.data(), &StandardSystemButton::clicked, this, [this](){
        if (m_window->isMaximized() || m_window->isFullScreen()) {
            m_window->showNormal();
        } else {
            m_window->showMaximized();
        }
    });
    m_closeButton.reset(new StandardSystemButton(SystemButtonType::Close, q));
    m_closeButton->setFixedSize(kDefaultSystemButtonSize);
    m_closeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_closeButton->setToolTip(tr("Close"));
    connect(m_closeButton.data(), &StandardSystemButton::clicked, m_window, &QWidget::close);
    const auto titleBarLayout = new QHBoxLayout(q);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addSpacerItem(new QSpacerItem(kDefaultTitleBarTitleLabelMargin, kDefaultTitleBarTitleLabelMargin));
    titleBarLayout->addWidget(m_windowTitleLabel.data());
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(m_minimizeButton.data());
    titleBarLayout->addWidget(m_maximizeButton.data());
    titleBarLayout->addWidget(m_closeButton.data());
    q->setLayout(titleBarLayout);
    updateTitleBarStyleSheet();
    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged,
                                        this, &StandardTitleBarPrivate::updateTitleBarStyleSheet);
    m_window->installEventFilter(this);
}

StandardTitleBar::StandardTitleBar(QWidget *parent)
    : QWidget(parent), d_ptr(new StandardTitleBarPrivate(this))
{
}

StandardTitleBar::~StandardTitleBar() = default;

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

void StandardTitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // This block of code ensures that our widget applies the stylesheet correctly.
    // Enabling the "Qt::WA_StyledBackground" attribute can also achieve the same
    // effect, but since it's documented as only for internal uses, we use the
    // public way to do that instead.
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

FRAMELESSHELPER_END_NAMESPACE
