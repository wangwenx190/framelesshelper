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

#include "framelesswidgetshelper.h"
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <framelesswindowsmanager.h>
#include <utils.h>
#include "framelesswidget.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

static constexpr const char QT_MAINWINDOW_CLASS_NAME[] = "QMainWindow";

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

FramelessWidgetsHelper::FramelessWidgetsHelper(QWidget *q, const WindowLayout wl) : QObject(q)
{
    Q_ASSERT(q);
    if (q) {
        this->q = q;
        m_windowLayout = wl;
        initialize();
    }
}

FramelessWidgetsHelper::~FramelessWidgetsHelper() = default;

bool FramelessWidgetsHelper::isNormal() const
{
    return (q->windowState() == Qt::WindowNoState);
}

bool FramelessWidgetsHelper::isZoomed() const
{
    return (q->isMaximized() || q->isFullScreen());
}

void FramelessWidgetsHelper::setTitleBarWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (m_userTitleBarWidget == widget) {
        return;
    }
    if (m_systemTitleBarWidget && m_systemTitleBarWidget->isVisible()) {
        m_systemTitleBarWidget->hide();
    }
    if (isMainWindow()) {
        m_userTitleBarWidget = widget;
    } else {
        if (m_userTitleBarWidget) {
            m_mainLayout->removeWidget(m_userTitleBarWidget);
            m_userTitleBarWidget = nullptr;
        }
        m_userTitleBarWidget = widget;
        m_mainLayout->insertWidget(0, m_userTitleBarWidget);
    }
    QMetaObject::invokeMethod(q, "titleBarWidgetChanged");
}

QWidget *FramelessWidgetsHelper::titleBarWidget() const
{
    return m_userTitleBarWidget;
}

void FramelessWidgetsHelper::setContentWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (isMainWindow()) {
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
    QMetaObject::invokeMethod(q, "contentWidgetChanged");
}

QWidget *FramelessWidgetsHelper::contentWidget() const
{
    if (isMainWindow()) {
        return nullptr;
    }
    return m_userContentWidget;
}

void FramelessWidgetsHelper::setHitTestVisible(QWidget *widget, const bool visible)
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

void FramelessWidgetsHelper::showEventHandler(QShowEvent *event)
{
    Q_UNUSED(event);
    setupFramelessHelperOnce();
}

void FramelessWidgetsHelper::changeEventHandler(QEvent *event)
{
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

void FramelessWidgetsHelper::paintEventHandler(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!shouldDrawFrameBorder()) {
        return;
    }
    QPainter painter(q);
    painter.save();
    QPen pen = {};
    pen.setColor(Utils::getFrameBorderColor(q->isActiveWindow()));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(0, 0, q->width(), 0);
    painter.restore();
}

void FramelessWidgetsHelper::mousePressEventHandler(QMouseEvent *event)
{
    const Qt::MouseButton button = event->button();
    if ((button != Qt::LeftButton) && (button != Qt::RightButton)) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        if (button == Qt::LeftButton) {
            Utils::startSystemMove(q->windowHandle());
        } else {
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            const QPointF globalPos = event->globalPosition();
#  else
            const QPointF globalPos = event->globalPos();
#  endif
            const QPointF pos = globalPos * q->devicePixelRatioF();
            Utils::showSystemMenu(q->winId(), pos);
#endif
        }
    }
}

void FramelessWidgetsHelper::mouseDoubleClickEventHandler(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        m_systemMaximizeButton->click();
    }
}

void FramelessWidgetsHelper::initialize()
{
    q->setAttribute(Qt::WA_DontCreateNativeAncestors);
    q->createWinId();
    setupInitialUi();
}

void FramelessWidgetsHelper::setupFramelessHelperOnce()
{
    if (m_framelessHelperInited) {
        return;
    }
    m_framelessHelperInited = true;
    FramelessWindowsManager *manager = FramelessWindowsManager::instance();
    manager->addWindow(q->windowHandle());
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this](){
        updateSystemTitleBarStyleSheet();
        updateSystemButtonsIcon();
        QMetaObject::invokeMethod(q, "systemThemeChanged");
    });
    connect(manager, &FramelessWindowsManager::systemMenuRequested, this, [this](const QPointF &pos){
        QMetaObject::invokeMethod(q, "systemMenuRequested", Q_ARG(QPointF, pos));
    });
}

void FramelessWidgetsHelper::createSystemTitleBar()
{
    if (isCustomWindow()) {
        return;
    }
    m_systemTitleBarWidget = new QWidget(q);
    m_systemTitleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_systemTitleBarWidget->setFixedHeight(kDefaultTitleBarHeight);
    m_systemWindowTitleLabel = new QLabel(m_systemTitleBarWidget);
    m_systemWindowTitleLabel->setFrameShape(QFrame::NoFrame);
    QFont windowTitleFont = q->font();
    windowTitleFont.setPointSize(11);
    m_systemWindowTitleLabel->setFont(windowTitleFont);
    m_systemWindowTitleLabel->setText(q->windowTitle());
    connect(q, &FramelessWidget::windowTitleChanged, m_systemWindowTitleLabel, &QLabel::setText);
    m_systemMinimizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMinimizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemMinimizeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemMinimizeButton->setToolTip(tr("Minimize"));
    connect(m_systemMinimizeButton, &QPushButton::clicked, q, &FramelessWidget::showMinimized);
    m_systemMaximizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMaximizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemMaximizeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemMaximizeButton->setToolTip(tr("Maximize"));
    connect(m_systemMaximizeButton, &QPushButton::clicked, this, [this](){
        if (isZoomed()) {
            q->showNormal();
        } else {
            q->showMaximized();
        }
        updateSystemButtonsIcon();
    });
    m_systemCloseButton = new QPushButton(m_systemTitleBarWidget);
    m_systemCloseButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemCloseButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemCloseButton->setToolTip(tr("Close"));
    connect(m_systemCloseButton, &QPushButton::clicked, q, &FramelessWidget::close);
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

void FramelessWidgetsHelper::createUserContentContainer()
{
    if (isCustomWindow() || isMainWindow()) {
        return;
    }
    m_userContentContainerWidget = new QWidget(q);
    m_userContentContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userContentContainerLayout = new QVBoxLayout(m_userContentContainerWidget);
    m_userContentContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_userContentContainerLayout->setSpacing(0);
    m_userContentContainerWidget->setLayout(m_userContentContainerLayout);
}

void FramelessWidgetsHelper::setupInitialUi()
{
    if (isStandardWindow()) {
        createSystemTitleBar();
        if (isMainWindow()) {
            // ### TODO
        } else {
            createUserContentContainer();
            m_mainLayout = new QVBoxLayout(q);
            m_mainLayout->setContentsMargins(0, 0, 0, 0);
            m_mainLayout->setSpacing(0);
            m_mainLayout->addWidget(m_systemTitleBarWidget);
            m_mainLayout->addWidget(m_userContentContainerWidget);
            q->setLayout(m_mainLayout);
        }
        updateSystemTitleBarStyleSheet();
    }
    updateContentsMargins();
}

bool FramelessWidgetsHelper::isInTitleBarDraggableArea(const QPoint &pos) const
{
    const QRegion draggableRegion = [this]() -> QRegion {
        if (m_userTitleBarWidget) {
            QRegion region = {QRect(QPoint(0, 0), m_userTitleBarWidget->size())};
            if (!m_hitTestVisibleWidgets.isEmpty()) {
                for (auto &&widget : qAsConst(m_hitTestVisibleWidgets)) {
                    Q_ASSERT(widget);
                    if (widget) {
                        region -= widget->geometry();
                    }
                }
            }
            return region;
        }
        if (isStandardWindow()) {
            QRegion region = {QRect(QPoint(0, 0), m_systemTitleBarWidget->size())};
            region -= m_systemMinimizeButton->geometry();
            region -= m_systemMaximizeButton->geometry();
            region -= m_systemCloseButton->geometry();
            return region;
        }
        return {};
    }();
    return draggableRegion.contains(pos);
}

bool FramelessWidgetsHelper::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater() && isNormal());
#else
    return false;
#endif
}

bool FramelessWidgetsHelper::isMainWindow() const
{
    if (!q) {
        return false;
    }
    return q->inherits(QT_MAINWINDOW_CLASS_NAME);
}

bool FramelessWidgetsHelper::isStandardWindow() const
{
    return (m_windowLayout == WindowLayout::Standard);
}

bool FramelessWidgetsHelper::isCustomWindow()
{
    return (m_windowLayout == WindowLayout::Custom);
}

void FramelessWidgetsHelper::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    q->setContentsMargins(0, (shouldDrawFrameBorder() ? 1 : 0), 0, 0);
#endif
}

void FramelessWidgetsHelper::updateSystemTitleBarStyleSheet()
{
    if (isCustomWindow()) {
        return;
    }
    const bool active = q->isActiveWindow();
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
    q->update();
}

void FramelessWidgetsHelper::updateSystemButtonsIcon()
{
    if (isCustomWindow()) {
        return;
    }
    const SystemTheme theme = ((Utils::shouldAppsUseDarkMode() || Utils::isTitleBarColorized()) ? SystemTheme::Dark : SystemTheme::Light);
    m_systemMinimizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Minimize, theme, ResourceType::Icon)));
    if (isZoomed()) {
        m_systemMaximizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Restore, theme, ResourceType::Icon)));
    } else {
        m_systemMaximizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Maximize, theme, ResourceType::Icon)));
    }
    m_systemCloseButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Close, theme, ResourceType::Icon)));
}

FRAMELESSHELPER_END_NAMESPACE
