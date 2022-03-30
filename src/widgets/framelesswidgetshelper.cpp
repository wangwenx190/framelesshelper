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
#include <QtCore/qdebug.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <framelesswindowsmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

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

FramelessWidgetsHelper::FramelessWidgetsHelper(QWidget *q, const UserSettings &settings) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    this->q = q;
    m_settings = settings;
    initialize();
}

FramelessWidgetsHelper::~FramelessWidgetsHelper() = default;

bool FramelessWidgetsHelper::isNormal() const
{
    return (m_params.getWindowState() == Qt::WindowNoState);
}

bool FramelessWidgetsHelper::isZoomed() const
{
    return (q->isMaximized() || ((m_settings.options & Option::DontTreatFullScreenAsZoomed) ? false : q->isFullScreen()));
}

bool FramelessWidgetsHelper::isFixedSize() const
{
    if (m_settings.options & Option::DisableResizing) {
        return true;
    }
    if (q->windowFlags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = q->minimumSize();
    const QSize maxSize = q->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    if (q->sizePolicy() == QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)) {
        return true;
    }
    return false;
}

void FramelessWidgetsHelper::setFixedSize(const bool value, const bool force)
{
    if ((isFixedSize() == value) && !force) {
        return;
    }
    if (value) {
        q->setFixedSize(q->size());
        q->setWindowFlags(q->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    } else {
        q->setWindowFlags(q->windowFlags() & ~Qt::MSWindowsFixedSizeDialogHint);
        q->setMinimumSize(kInvalidWindowSize);
        q->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(m_params.windowId, !value);
#endif
    QMetaObject::invokeMethod(q, "fixedSizeChanged");
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
    if (m_settings.options & Option::UseStandardWindowLayout) {
        if (m_systemTitleBarWidget && m_systemTitleBarWidget->isVisible()) {
            m_mainLayout->removeWidget(m_systemTitleBarWidget);
            m_systemTitleBarWidget->hide();
        }
        if (m_userTitleBarWidget) {
            m_mainLayout->removeWidget(m_userTitleBarWidget);
            m_userTitleBarWidget = nullptr;
        }
        m_userTitleBarWidget = widget;
        m_mainLayout->insertWidget(0, m_userTitleBarWidget);
    } else {
        m_userTitleBarWidget = widget;
    }
    QMetaObject::invokeMethod(q, "titleBarWidgetChanged");
}

QWidget *FramelessWidgetsHelper::getTitleBarWidget() const
{
    return m_userTitleBarWidget;
}

void FramelessWidgetsHelper::setContentWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (!(m_settings.options & Option::UseStandardWindowLayout)) {
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

QWidget *FramelessWidgetsHelper::getContentWidget() const
{
    return m_userContentWidget;
}

void FramelessWidgetsHelper::setHitTestVisible(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    static constexpr const bool visible = true;
    const bool exists = m_hitTestVisibleWidgets.contains(widget);
    if (visible && !exists) {
        m_hitTestVisibleWidgets.append(widget);
    }
    if constexpr (!visible && exists) {
        m_hitTestVisibleWidgets.removeAll(widget);
    }
}

void FramelessWidgetsHelper::showEventHandler(QShowEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (m_windowExposed) {
        return;
    }
    m_windowExposed = true;
    if (m_settings.options & Option::DontMoveWindowToDesktopCenter) {
        if (!m_settings.startupPosition.isNull()) {
            m_params.setWindowPosition(m_settings.startupPosition);
        }
        if (!m_settings.startupSize.isEmpty()) {
            m_params.setWindowSize(m_settings.startupSize);
        }
        if (m_settings.startupState != Qt::WindowNoState) {
            m_params.setWindowState(m_settings.startupState);
        }
    } else {
        moveToDesktopCenter();
    }
}

void FramelessWidgetsHelper::changeEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    const QEvent::Type type = event->type();
    if ((type != QEvent::WindowStateChange) && (type != QEvent::ActivationChange)) {
        return;
    }
    const bool standardLayout = (m_settings.options & Option::UseStandardWindowLayout);
    if (type == QEvent::WindowStateChange) {
        if (standardLayout) {
            if (isZoomed()) {
                m_systemMaximizeButton->setToolTip(tr("Restore"));
            } else {
                m_systemMaximizeButton->setToolTip(tr("Maximize"));
            }
            updateSystemButtonsIcon();
        }
        updateContentsMargins();
    }
    if (standardLayout) {
        updateSystemTitleBarStyleSheet();
    }
    q->update();
}

void FramelessWidgetsHelper::paintEventHandler(QPaintEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (!shouldDrawFrameBorder()) {
        return;
    }
    QPainter painter(q);
    painter.save();
    QPen pen = {};
    pen.setColor(Utils::getFrameBorderColor(q->isActiveWindow()));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(0, 0, q->width()-1, 0);
    painter.restore();
}

void FramelessWidgetsHelper::mouseMoveEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (m_settings.options & Option::DisableDragging) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = event->scenePosition().toPoint();
#else
    const QPoint scenePos = event->windowPos().toPoint();
#endif
    if (shouldIgnoreMouseEvents(scenePos)) {
        return;
    }
    if (!isInTitleBarDraggableArea(scenePos)) {
        return;
    }
    startSystemMove2();
}

void FramelessWidgetsHelper::mouseReleaseEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (m_settings.options & Option::DisableSystemMenu) {
        return;
    }
    if (event->button() != Qt::RightButton) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = event->scenePosition().toPoint();
#else
    const QPoint scenePos = event->windowPos().toPoint();
#endif
    if (shouldIgnoreMouseEvents(scenePos)) {
        return;
    }
    if (!isInTitleBarDraggableArea(scenePos)) {
        return;
    }
    showSystemMenu(scenePos);
}

void FramelessWidgetsHelper::mouseDoubleClickEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if ((m_settings.options & Option::NoDoubleClickMaximizeToggle) || isFixedSize()) {
        return;
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = event->scenePosition().toPoint();
#else
    const QPoint scenePos = event->windowPos().toPoint();
#endif
    if (shouldIgnoreMouseEvents(scenePos)) {
        return;
    }
    if (!isInTitleBarDraggableArea(scenePos)) {
        return;
    }
    toggleMaximize();
}

void FramelessWidgetsHelper::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    // Without this flag, Qt will always create an invisible native parent window
    // for any native widgets which will intercept some win32 messages and confuse
    // our own native event filter, so to prevent some weired bugs from happening,
    // just disable this feature.
    q->setAttribute(Qt::WA_DontCreateNativeAncestors);
    // Force the widget become a native window now so that we can deal with its
    // win32 events as soon as possible.
    q->setAttribute(Qt::WA_NativeWindow);
    const WId windowId = q->winId();
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    m_window = q->windowHandle();
    Q_ASSERT(m_window);
    if (!m_window) {
        return;
    }
    m_params.windowId = windowId;
    m_params.getWindowFlags = [this]() -> Qt::WindowFlags { return q->windowFlags(); };
    m_params.setWindowFlags = [this](const Qt::WindowFlags flags) -> void { q->setWindowFlags(flags); };
    m_params.getWindowSize = [this]() -> QSize { return q->size(); };
    m_params.setWindowSize = [this](const QSize &size) -> void { q->resize(size); };
    m_params.getWindowPosition = [this]() -> QPoint { return q->pos(); };
    m_params.setWindowPosition = [this](const QPoint &pos) -> void { q->move(pos); };
    m_params.getWindowScreen = [this]() -> QScreen * {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return q->screen();
#else
        return m_window->screen();
#endif
    };
    m_params.isWindowFixedSize = [this]() -> bool { return isFixedSize(); };
    m_params.setWindowFixedSize = [this](const bool value) -> void { setFixedSize(value); };
    m_params.getWindowState = [this]() -> Qt::WindowState { return Utils::windowStatesToWindowState(q->windowState()); };
    m_params.setWindowState = [this](const Qt::WindowState state) -> void { q->setWindowState(state); };
    m_params.getWindowHandle = [this]() -> QWindow * { return m_window; };
    m_params.windowToScreen = [this](const QPoint &pos) -> QPoint { return q->mapToGlobal(pos); };
    m_params.screenToWindow = [this](const QPoint &pos) -> QPoint { return q->mapFromGlobal(pos); };
    m_params.isInsideSystemButtons = [this](const QPoint &pos, SystemButtonType *button) -> bool { return isInSystemButtons(pos, button); };
    m_params.isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
    m_params.getWindowDevicePixelRatio = [this]() -> qreal { return q->devicePixelRatioF(); };
    if (m_settings.options & Option::UseStandardWindowLayout) {
        if (q->inherits(QT_MAINWINDOW_CLASS_NAME)) {
            m_settings.options &= ~Options(Option::UseStandardWindowLayout);
            qWarning() << "\"Option::UseStandardWindowLayout\" is not compatible with QMainWindow and it's subclasses."
                          " Enabling this option will mess up with your main window's layout.";
        }
    }
    if (m_settings.options & Option::DisableResizing) {
        setFixedSize(true, true);
    }
    if (m_settings.options & Option::BeCompatibleWithQtFramelessWindowHint) {
        Utils::tryToBeCompatibleWithQtFramelessWindowHint(windowId, m_params.getWindowFlags,
                                                          m_params.setWindowFlags, true);
    }
    FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    manager->addWindow(m_settings, m_params);
    q->installEventFilter(this);
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this](){
        if (m_settings.options & Option::UseStandardWindowLayout) {
            updateSystemTitleBarStyleSheet();
            updateSystemButtonsIcon();
            q->update();
        }
        QMetaObject::invokeMethod(q, "systemThemeChanged");
    });
    connect(m_window, &QWindow::visibilityChanged, this, [this](){
        QMetaObject::invokeMethod(q, "hiddenChanged");
        QMetaObject::invokeMethod(q, "normalChanged");
        QMetaObject::invokeMethod(q, "zoomedChanged");
    });
    setupInitialUi();
    if (m_settings.options & Option::UseStandardWindowLayout) {
        m_settings.minimizeButton = m_systemMinimizeButton;
        m_settings.maximizeButton = m_systemMaximizeButton;
        m_settings.closeButton = m_systemCloseButton;
    }
}

void FramelessWidgetsHelper::createSystemTitleBar()
{
    if (!(m_settings.options & Option::UseStandardWindowLayout)) {
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
    connect(q, &QWidget::windowTitleChanged, m_systemWindowTitleLabel, &QLabel::setText);
    m_systemMinimizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMinimizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemMinimizeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemMinimizeButton->setToolTip(tr("Minimize"));
    connect(m_systemMinimizeButton, &QPushButton::clicked, q, &QWidget::showMinimized);
    m_systemMaximizeButton = new QPushButton(m_systemTitleBarWidget);
    m_systemMaximizeButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemMaximizeButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemMaximizeButton->setToolTip(tr("Maximize"));
    connect(m_systemMaximizeButton, &QPushButton::clicked, this, &FramelessWidgetsHelper::toggleMaximize);
    m_systemCloseButton = new QPushButton(m_systemTitleBarWidget);
    m_systemCloseButton->setFixedSize(kDefaultSystemButtonSize);
    m_systemCloseButton->setIconSize(kDefaultSystemButtonIconSize);
    m_systemCloseButton->setToolTip(tr("Close"));
    connect(m_systemCloseButton, &QPushButton::clicked, q, &QWidget::close);
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
    if (!(m_settings.options & Option::UseStandardWindowLayout)) {
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
    if (m_settings.options & Option::UseStandardWindowLayout) {
        createSystemTitleBar();
        createUserContentContainer();
        m_mainLayout = new QVBoxLayout(q);
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);
        m_mainLayout->addWidget(m_systemTitleBarWidget);
        m_mainLayout->addWidget(m_userContentContainerWidget);
        q->setLayout(m_mainLayout);
        updateSystemTitleBarStyleSheet();
        q->update();
    }
    updateContentsMargins();
}

QRect FramelessWidgetsHelper::mapWidgetGeometryToScene(const QWidget * const widget) const
{
    Q_ASSERT(widget);
    if (!widget) {
        return {};
    }
    const QPoint originPoint = widget->mapTo(q, QPoint(0, 0));
    const QSize size = widget->size();
    return QRect(originPoint, size);
}

bool FramelessWidgetsHelper::isInSystemButtons(const QPoint &pos, SystemButtonType *button) const
{
    Q_ASSERT(button);
    if (!button) {
        return false;
    }
    *button = SystemButtonType::Unknown;
    if (!m_settings.minimizeButton || !m_settings.maximizeButton || !m_settings.closeButton) {
        return false;
    }
    if (!m_settings.minimizeButton->isWidgetType() || !m_settings.maximizeButton->isWidgetType()
        || !m_settings.closeButton->isWidgetType()) {
        return false;
    }
    const auto minBtn = qobject_cast<QWidget *>(m_settings.minimizeButton);
    if (minBtn->geometry().contains(pos)) {
        *button = SystemButtonType::Minimize;
        return true;
    }
    const auto maxBtn = qobject_cast<QWidget *>(m_settings.maximizeButton);
    if (maxBtn->geometry().contains(pos)) {
        *button = SystemButtonType::Maximize;
        return true;
    }
    const auto closeBtn = qobject_cast<QWidget *>(m_settings.closeButton);
    if (closeBtn->geometry().contains(pos)) {
        *button = SystemButtonType::Close;
        return true;
    }
    return false;
}

bool FramelessWidgetsHelper::isInTitleBarDraggableArea(const QPoint &pos) const
{
    const QRegion draggableRegion = [this]() -> QRegion {
        if (m_userTitleBarWidget) {
            QRegion region = mapWidgetGeometryToScene(m_userTitleBarWidget);
            if (!m_hitTestVisibleWidgets.isEmpty()) {
                for (auto &&widget : qAsConst(m_hitTestVisibleWidgets)) {
                    Q_ASSERT(widget);
                    if (widget) {
                        region -= mapWidgetGeometryToScene(widget);
                    }
                }
            }
            return region;
        }
        if (m_settings.options & Option::UseStandardWindowLayout) {
            QRegion region = mapWidgetGeometryToScene(m_systemTitleBarWidget);
            region -= mapWidgetGeometryToScene(m_systemMinimizeButton);
            region -= mapWidgetGeometryToScene(m_systemMaximizeButton);
            region -= mapWidgetGeometryToScene(m_systemCloseButton);
            return region;
        }
        return {};
    }();
    return draggableRegion.contains(pos);
}

bool FramelessWidgetsHelper::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWin11OrGreater()
            && isNormal() && !(m_settings.options & Option::DontDrawTopWindowFrameBorder));
#else
    return false;
#endif
}

bool FramelessWidgetsHelper::shouldIgnoreMouseEvents(const QPoint &pos) const
{
    return (isNormal()
            && ((pos.y() < kDefaultResizeBorderThickness)
                || (Utils::isWindowFrameBorderVisible()
                        ? false : ((pos.x() < kDefaultResizeBorderThickness)
                           || (pos.x() >= (q->width() - kDefaultResizeBorderThickness))))));
}

void FramelessWidgetsHelper::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    q->setContentsMargins(0, (shouldDrawFrameBorder() ? 1 : 0), 0, 0);
#endif
}

void FramelessWidgetsHelper::updateSystemTitleBarStyleSheet()
{
    if (!(m_settings.options & Option::UseStandardWindowLayout)) {
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
}

void FramelessWidgetsHelper::updateSystemButtonsIcon()
{
    if (!(m_settings.options & Option::UseStandardWindowLayout)) {
        return;
    }
    const SystemTheme theme = ((Utils::shouldAppsUseDarkMode() || Utils::isTitleBarColorized()) ? SystemTheme::Dark : SystemTheme::Light);
    const ResourceType resource = ResourceType::Icon;
    m_systemMinimizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Minimize, theme, resource)));
    if (isZoomed()) {
        m_systemMaximizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Restore, theme, resource)));
    } else {
        m_systemMaximizeButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Maximize, theme, resource)));
    }
    m_systemCloseButton->setIcon(qvariant_cast<QIcon>(Utils::getSystemButtonIconResource(SystemButtonType::Close, theme, resource)));
}

void FramelessWidgetsHelper::toggleMaximize()
{
    if (isFixedSize()) {
        return;
    }
    if (isZoomed()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessWidgetsHelper::toggleFullScreen()
{
    if (isFixedSize()) {
        return;
    }
    const Qt::WindowState windowState = m_params.getWindowState();
    if (windowState == Qt::WindowFullScreen) {
        q->setWindowState(m_savedWindowState);
    } else {
        m_savedWindowState = windowState;
        q->showFullScreen();
    }
}

void FramelessWidgetsHelper::moveToDesktopCenter()
{
    Utils::moveWindowToDesktopCenter(m_params.getWindowScreen,
                                     m_params.getWindowSize, m_params.setWindowPosition, true);
}

void FramelessWidgetsHelper::bringToFront()
{
    if (q->isHidden()) {
        q->show();
    }
    if (q->isMinimized()) {
        q->setWindowState(q->windowState() & ~Qt::WindowMinimized);
    }
    q->raise();
    q->activateWindow();
}

void FramelessWidgetsHelper::showSystemMenu(const QPoint &pos)
{
#ifdef Q_OS_WINDOWS
    const QPoint globalPos = q->mapToGlobal(pos);
    const QPoint nativePos = QPointF(QPointF(globalPos) * q->devicePixelRatioF()).toPoint();
    Utils::showSystemMenu(m_params.windowId, nativePos, m_settings.options,
                          m_settings.systemMenuOffset, m_params.isWindowFixedSize);
#endif
}

void FramelessWidgetsHelper::startSystemMove2()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    m_window->startSystemMove();
#else
    Utils::startSystemMove(m_window);
#endif
}

void FramelessWidgetsHelper::startSystemResize2(const Qt::Edges edges)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    m_window->startSystemResize(edges);
#else
    Utils::startSystemResize(m_window, edges);
#endif
}

bool FramelessWidgetsHelper::eventFilter(QObject *object, QEvent *event)
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
    if (widget != q) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::Show: {
        const auto showEvent = static_cast<QShowEvent *>(event);
        showEventHandler(showEvent);
    } break;
    case QEvent::Paint: {
        const auto paintEvent = static_cast<QPaintEvent *>(event);
        paintEventHandler(paintEvent);
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseMoveEventHandler(mouseEvent);
    } break;
    case QEvent::MouseButtonRelease: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseReleaseEventHandler(mouseEvent);
    } break;
    case QEvent::MouseButtonDblClick: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseDoubleClickEventHandler(mouseEvent);
    } break;
    case QEvent::ActivationChange:
    case QEvent::WindowStateChange: {
        changeEventHandler(event);
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

FRAMELESSHELPER_END_NAMESPACE
