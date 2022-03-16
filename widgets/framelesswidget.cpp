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

#include "framelesswidget.h"
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <framelesswindowsmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

static const QString kSystemButtonStyleSheet = QStringLiteral(R"(
QPushButton {
    border-style: none;
    background-color: transparent;
}

QPushButton:hover {
    background-color: #cccccc;
}

QPushButton:pressed {
    background-color: #b3b3b3;
}
)");

FramelessWidget::FramelessWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();
    setupInitialUi();
}

FramelessWidget::~FramelessWidget() = default;

bool FramelessWidget::isNormal() const
{
    return (windowState() == Qt::WindowNoState);
}

bool FramelessWidget::isZoomed() const
{
    return (isMaximized() || isFullScreen());
}

void FramelessWidget::setTitleBarWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (m_userTitleBarWidget == widget) {
        return;
    }
    if (m_systemTitleBarWidget) {
        m_systemTitleBarWidget->hide();
    }
    if (m_userTitleBarWidget) {
        m_mainLayout->removeWidget(m_userTitleBarWidget);
        m_userTitleBarWidget = nullptr;
    }
    m_userTitleBarWidget = widget;
    m_mainLayout->insertWidget(0, m_userTitleBarWidget);
    Q_EMIT titleBarWidgetChanged();
}

QWidget *FramelessWidget::titleBarWidget() const
{
    return m_userTitleBarWidget;
}

void FramelessWidget::setContentWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (m_userContentWidget == widget) {
        return;
    }
    if (m_userContentWidget) {
        m_userContentContainerLayout->removeWidget(m_userContentWidget);
        m_userContentWidget = nullptr;
    }
    m_userContentWidget = widget;
    m_userContentContainerLayout->addWidget(m_userContentWidget);
    Q_EMIT contentWidgetChanged();
}

QWidget *FramelessWidget::contentWidget() const
{
    return m_userContentWidget;
}

void FramelessWidget::setHitTestVisible(QWidget *widget, const bool visible)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    const bool exists = m_hitTestVisibleWidgets.contains(widget);
    if (visible && !exists) {
        m_hitTestVisibleWidgets.append(widget);
    }
    if (!visible && exists) {
        m_hitTestVisibleWidgets.removeAll(widget);
    }
}

void FramelessWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setupFramelessHelperOnce();
}

void FramelessWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    bool shouldUpdate = false;
    if (event->type() == QEvent::WindowStateChange) {
        if (isZoomed()) {
            m_systemMaximizeButton->setToolTip(tr("Restore"));
        } else {
            m_systemMaximizeButton->setToolTip(tr("Maximize"));
        }
        updateContentsMargins();
        updateSystemButtonsIcon();
        shouldUpdate = true;
    } else if (event->type() == QEvent::ActivationChange) {
        shouldUpdate = true;
    }
    if (shouldUpdate) {
        updateSystemTitleBarStyleSheet();
    }
}

void FramelessWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (shouldDrawFrameBorder()) {
        QPainter painter(this);
        painter.save();
        QPen pen = {};
        pen.setColor(Utils::getFrameBorderColor(isActiveWindow()));
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(0, 0, width(), 0);
        painter.restore();
    }
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    const Qt::MouseButton button = event->button();
    if ((button != Qt::LeftButton) && (button != Qt::RightButton)) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        if (button == Qt::LeftButton) {
            Utils::startSystemMove(windowHandle());
        } else {
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            const QPointF globalPos = event->globalPosition();
#  else
            const QPointF globalPos = event->globalPos();
#  endif
            const QPointF pos = globalPos * devicePixelRatioF();
            Utils::showSystemMenu(winId(), pos);
#endif
        }
    }
}

void FramelessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        m_systemMaximizeButton->click();
    }
}

void FramelessWidget::setupFramelessHelperOnce()
{
    if (m_framelessHelperInited) {
        return;
    }
    m_framelessHelperInited = true;
    FramelessWindowsManager::addWindow(windowHandle());
    const FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this](){
        updateSystemTitleBarStyleSheet();
        updateSystemButtonsIcon();
        Q_EMIT systemThemeChanged();
    });
    connect(manager, &FramelessWindowsManager::systemMenuRequested, this, &FramelessWidget::systemMenuRequested);
}

void FramelessWidget::createSystemTitleBar()
{
    if (m_systemTitleBarWidget) {
        return;
    }
    static constexpr const QSize systemButtonSize = {int(qRound(qreal(kDefaultTitleBarHeight) * 1.5)), kDefaultTitleBarHeight};
    static constexpr const QSize systemButtonIconSize = {16, 16};
    m_systemTitleBarWidget = new QWidget(this);
    m_systemTitleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_systemTitleBarWidget->setFixedHeight(kDefaultTitleBarHeight);
    m_systemWindowTitleLabel = new QLabel(m_systemTitleBarWidget);
    m_systemWindowTitleLabel->setFrameShape(QFrame::NoFrame);
    QFont windowTitleFont = font();
    windowTitleFont.setPointSize(11);
    m_systemWindowTitleLabel->setFont(windowTitleFont);
    m_systemWindowTitleLabel->setText(windowTitle());
    connect(this, &FramelessWidget::windowTitleChanged, m_systemWindowTitleLabel, &QLabel::setText);
    m_systemMinimizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMinimizeButton->setFixedSize(systemButtonSize);
    m_systemMinimizeButton->setIconSize(systemButtonIconSize);
    m_systemMinimizeButton->setToolTip(tr("Minimize"));
    connect(m_systemMinimizeButton, &QPushButton::clicked, this, &FramelessWidget::showMinimized);
    m_systemMaximizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMaximizeButton->setFixedSize(systemButtonSize);
    m_systemMaximizeButton->setIconSize(systemButtonIconSize);
    m_systemMaximizeButton->setToolTip(tr("Maximize"));
    connect(m_systemMaximizeButton, &QPushButton::clicked, this, [this](){
        if (isZoomed()) {
            showNormal();
        } else {
            showMaximized();
        }
        updateSystemButtonsIcon();
    });
    m_systemCloseButton = new QPushButton(m_systemTitleBarWidget);
    m_systemCloseButton->setFixedSize(systemButtonSize);
    m_systemCloseButton->setIconSize(systemButtonIconSize);
    m_systemCloseButton->setToolTip(tr("Close"));
    connect(m_systemCloseButton, &QPushButton::clicked, this, &FramelessWidget::close);
    updateSystemButtonsIcon();
    const auto systemTitleBarLayout = new QHBoxLayout(m_systemTitleBarWidget);
    systemTitleBarLayout->setContentsMargins(0, 0, 0, 0);
    systemTitleBarLayout->setSpacing(0);
    systemTitleBarLayout->addSpacerItem(new QSpacerItem(10, 10));
    systemTitleBarLayout->addWidget(m_systemWindowTitleLabel);
    systemTitleBarLayout->addStretch();
    systemTitleBarLayout->addWidget(m_systemMinimizeButton);
    systemTitleBarLayout->addWidget(m_systemMaximizeButton);
    systemTitleBarLayout->addWidget(m_systemCloseButton);
    m_systemTitleBarWidget->setLayout(systemTitleBarLayout);
}

void FramelessWidget::createUserContentContainer()
{
    m_userContentContainerWidget = new QWidget(this);
    m_userContentContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userContentContainerLayout = new QVBoxLayout(m_userContentContainerWidget);
    m_userContentContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_userContentContainerLayout->setSpacing(0);
    m_userContentContainerWidget->setLayout(m_userContentContainerLayout);
}

void FramelessWidget::setupInitialUi()
{
    createSystemTitleBar();
    createUserContentContainer();
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_systemTitleBarWidget);
    m_mainLayout->addWidget(m_userContentContainerWidget);
    setLayout(m_mainLayout);
    updateSystemTitleBarStyleSheet();
    updateContentsMargins();
}

bool FramelessWidget::isInTitleBarDraggableArea(const QPoint &pos) const
{
    const QRegion draggableRegion = [this]() -> QRegion {
        if (m_userTitleBarWidget) {
            QRegion region = {QRect(QPoint(0, 0), m_userTitleBarWidget->size())};
            if (!m_hitTestVisibleWidgets.isEmpty()) {
                for (auto &&widget : qAsConst(m_hitTestVisibleWidgets)) {
                    region -= widget->geometry();
                }
            }
            return region;
        } else {
            QRegion region = {QRect(QPoint(0, 0), m_systemTitleBarWidget->size())};
            region -= m_systemMinimizeButton->geometry();
            region -= m_systemMaximizeButton->geometry();
            region -= m_systemCloseButton->geometry();
            return region;
        }
    }();
    return draggableRegion.contains(pos);
}

bool FramelessWidget::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater() && isNormal());
#else
    return false;
#endif
}

void FramelessWidget::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    setContentsMargins(0, (shouldDrawFrameBorder() ? 1 : 0), 0, 0);
#endif
}

void FramelessWidget::updateSystemTitleBarStyleSheet()
{
    const bool active = isActiveWindow();
    const bool dark = Utils::shouldAppsUseDarkMode();
    const bool colorizedTitleBar = Utils::isTitleBarColorized();
    const QColor systemTitleBarWidgetBackgroundColor = [active, colorizedTitleBar, dark]() -> QColor {
        if (active) {
            if (colorizedTitleBar) {
                return Utils::getDwmColorizationColor();
            } else {
                if (dark) {
                    return QColor(Qt::black);
                } else {
                    return QColor(Qt::white);
                }
            }
        } else {
            if (dark) {
                return kDefaultSystemDarkColor;
            } else {
                return QColor(Qt::white);
            }
        }
    }();
    const QColor systemWindowTitleLabelTextColor = (active ? ((dark || colorizedTitleBar) ? Qt::white : Qt::black) : Qt::darkGray);
    m_systemWindowTitleLabel->setStyleSheet(QStringLiteral("color: %1;").arg(systemWindowTitleLabelTextColor.name()));
    m_systemMinimizeButton->setStyleSheet(kSystemButtonStyleSheet);
    m_systemMaximizeButton->setStyleSheet(kSystemButtonStyleSheet);
    m_systemCloseButton->setStyleSheet(kSystemButtonStyleSheet);
    m_systemTitleBarWidget->setStyleSheet(QStringLiteral("background-color: %1;").arg(systemTitleBarWidgetBackgroundColor.name()));
    update();
}

void FramelessWidget::updateSystemButtonsIcon()
{
    const SystemTheme theme = ((Utils::shouldAppsUseDarkMode() || Utils::isTitleBarColorized()) ? SystemTheme::Dark : SystemTheme::Light);
    m_systemMinimizeButton->setIcon(Utils::getSystemButtonIcon(SystemButtonType::Minimize, theme));
    if (isZoomed()) {
        m_systemMaximizeButton->setIcon(Utils::getSystemButtonIcon(SystemButtonType::Restore, theme));
    } else {
        m_systemMaximizeButton->setIcon(Utils::getSystemButtonIcon(SystemButtonType::Maximize, theme));
    }
    m_systemCloseButton->setIcon(Utils::getSystemButtonIcon(SystemButtonType::Close, theme));
}

FRAMELESSHELPER_END_NAMESPACE
