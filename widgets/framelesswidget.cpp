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

class FramelessWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessWidget)
    Q_DISABLE_COPY_MOVE(FramelessWidgetPrivate)

public:
    explicit FramelessWidgetPrivate(FramelessWidget *q, QObject *parent = nullptr);
    ~FramelessWidgetPrivate() override;

    Q_NODISCARD bool isNormal() const;
    Q_NODISCARD bool isZoomed() const;

    void setTitleBarWidget(QWidget *widget);
    Q_NODISCARD QWidget *titleBarWidget() const;

    void setContentWidget(QWidget *widget);
    Q_NODISCARD QWidget *contentWidget() const;

    void setHitTestVisible(QWidget *widget, const bool visible);

private:
    void setupFramelessHelperOnce();
    void createSystemTitleBar();
    void createUserContentContainer();
    void setupInitialUi();
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldDrawFrameBorder() const;

private Q_SLOTS:
    void updateContentsMargins();
    void updateSystemTitleBarStyleSheet();
    void updateSystemButtonsIcon();

private:
    FramelessWidget *q_ptr = nullptr;
    bool m_framelessHelperInited = false;
    QWidget *m_systemTitleBarWidget = nullptr;
    QLabel *m_systemWindowTitleLabel = nullptr;
    QPushButton *m_systemMinimizeButton = nullptr;
    QPushButton *m_systemMaximizeButton = nullptr;
    QPushButton *m_systemCloseButton = nullptr;
    QWidget *m_userTitleBarWidget = nullptr;
    QWidget *m_userContentWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QWidgetList m_hitTestVisibleWidgets = {};
    QWidget *m_userContentContainerWidget = nullptr;
    QVBoxLayout *m_userContentContainerLayout = nullptr;
};

FramelessWidgetPrivate::FramelessWidgetPrivate(FramelessWidget *q, QObject *parent) : QObject(parent)
{
    Q_ASSERT(q);
    if (q) {
        q_ptr = q;
    }
}

FramelessWidgetPrivate::~FramelessWidgetPrivate() = default;

bool FramelessWidgetPrivate::isNormal() const
{
    Q_Q(const FramelessWidget);
    return (q->windowState() == Qt::WindowNoState);
}

bool FramelessWidgetPrivate::isZoomed() const
{
    Q_Q(const FramelessWidget);
    return (q->isMaximized() || q->isFullScreen());
}

void FramelessWidgetPrivate::setTitleBarWidget(QWidget *widget)
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
    Q_Q(FramelessWidget);
    Q_EMIT q->titleBarWidgetChanged();
}

QWidget *FramelessWidgetPrivate::titleBarWidget() const
{
    return m_userTitleBarWidget;
}

void FramelessWidgetPrivate::setContentWidget(QWidget *widget)
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
    Q_Q(FramelessWidget);
    Q_EMIT q->contentWidgetChanged();
}

QWidget *FramelessWidgetPrivate::contentWidget() const
{
    return m_userContentWidget;
}

void FramelessWidgetPrivate::setHitTestVisible(QWidget *widget, const bool visible)
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

void FramelessWidgetPrivate::setupFramelessHelperOnce()
{
    if (m_framelessHelperInited) {
        return;
    }
    m_framelessHelperInited = true;
    Q_Q(FramelessWidget);
    FramelessWindowsManager::addWindow(q->windowHandle());
    const FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this, q](){
        updateSystemTitleBarStyleSheet();
        updateSystemButtonsIcon();
        Q_EMIT q->systemThemeChanged();
    });
    connect(manager, &FramelessWindowsManager::systemMenuRequested, q, &FramelessWidget::systemMenuRequested);
}

void FramelessWidgetPrivate::createSystemTitleBar()
{
    if (m_systemTitleBarWidget) {
        return;
    }
    static constexpr const QSize systemButtonSize = {int(qRound(qreal(kDefaultTitleBarHeight) * 1.5)), kDefaultTitleBarHeight};
    static constexpr const QSize systemButtonIconSize = {16, 16};
    Q_Q(FramelessWidget);
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
    m_systemMinimizeButton->setFixedSize(systemButtonSize);
    m_systemMinimizeButton->setIconSize(systemButtonIconSize);
    m_systemMinimizeButton->setToolTip(tr("Minimize"));
    connect(m_systemMinimizeButton, &QPushButton::clicked, q, &FramelessWidget::showMinimized);
    m_systemMaximizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMaximizeButton->setFixedSize(systemButtonSize);
    m_systemMaximizeButton->setIconSize(systemButtonIconSize);
    m_systemMaximizeButton->setToolTip(tr("Maximize"));
    connect(m_systemMaximizeButton, &QPushButton::clicked, this, [this, q](){
        if (isZoomed()) {
            q->showNormal();
        } else {
            q->showMaximized();
        }
        updateSystemButtonsIcon();
    });
    m_systemCloseButton = new QPushButton(m_systemTitleBarWidget);
    m_systemCloseButton->setFixedSize(systemButtonSize);
    m_systemCloseButton->setIconSize(systemButtonIconSize);
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

void FramelessWidgetPrivate::createUserContentContainer()
{
    Q_Q(FramelessWidget);
    m_userContentContainerWidget = new QWidget(q);
    m_userContentContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userContentContainerLayout = new QVBoxLayout(m_userContentContainerWidget);
    m_userContentContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_userContentContainerLayout->setSpacing(0);
    m_userContentContainerWidget->setLayout(m_userContentContainerLayout);
}

void FramelessWidgetPrivate::setupInitialUi()
{
    createSystemTitleBar();
    createUserContentContainer();
    Q_Q(FramelessWidget);
    m_mainLayout = new QVBoxLayout(q);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_systemTitleBarWidget);
    m_mainLayout->addWidget(m_userContentContainerWidget);
    q->setLayout(m_mainLayout);
    updateSystemTitleBarStyleSheet();
    updateContentsMargins();
}

bool FramelessWidgetPrivate::isInTitleBarDraggableArea(const QPoint &pos) const
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

bool FramelessWidgetPrivate::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater() && isNormal());
#else
    return false;
#endif
}

void FramelessWidgetPrivate::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    Q_Q(FramelessWidget);
    q->setContentsMargins(0, (shouldDrawFrameBorder() ? 1 : 0), 0, 0);
#endif
}

void FramelessWidgetPrivate::updateSystemTitleBarStyleSheet()
{
    Q_Q(FramelessWidget);
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

void FramelessWidgetPrivate::updateSystemButtonsIcon()
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

FramelessWidget::FramelessWidget(QWidget *parent) : QWidget(parent), d_ptr(new FramelessWidgetPrivate(this, this))
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();
    Q_D(FramelessWidget);
    d->setupInitialUi();
}

FramelessWidget::~FramelessWidget() = default;

bool FramelessWidget::isNormal() const
{
    Q_D(const FramelessWidget);
    return d->isNormal();
}

bool FramelessWidget::isZoomed() const
{
    Q_D(const FramelessWidget);
    return d->isZoomed();
}

void FramelessWidget::setTitleBarWidget(QWidget *widget)
{
    Q_D(FramelessWidget);
    d->setTitleBarWidget(widget);
}

QWidget *FramelessWidget::titleBarWidget() const
{
    Q_D(const FramelessWidget);
    return d->titleBarWidget();
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
    Q_D(FramelessWidget);
    d->setupFramelessHelperOnce();
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
    Q_D(FramelessWidget);
    if (d->shouldDrawFrameBorder()) {
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
    Q_D(FramelessWidget);
    if (d->isInTitleBarDraggableArea(event->pos())) {
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
    Q_D(FramelessWidget);
    if (d->isInTitleBarDraggableArea(event->pos())) {
        d->m_systemMaximizeButton->click();
    }
}

FRAMELESSHELPER_END_NAMESPACE
