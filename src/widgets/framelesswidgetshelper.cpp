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

#include "framelesswidgetshelper_p.h"
#include "standardsystembutton.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <framelesswindowsmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

static constexpr const char FRAMELESSHELPER_PROP_NAME[] = "__wwx190_FramelessWidgetsHelper_instance";
static constexpr const char QTWIDGETS_MAINWINDOW_CLASS_NAME[] = "QMainWindow";

FRAMELESSHELPER_STRING_CONSTANT2(StyleSheetColorTemplate, "color: %1;")
FRAMELESSHELPER_STRING_CONSTANT2(StyleSheetBackgroundColorTemplate, "background-color: %1;")

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

FramelessWidgetsHelper *FramelessWidgetsHelper::get(QWidget *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return qvariant_cast<FramelessWidgetsHelper *>(pub->property(FRAMELESSHELPER_PROP_NAME));
}

const FramelessWidgetsHelper *FramelessWidgetsHelper::get(const QWidget *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return qvariant_cast<FramelessWidgetsHelper *>(pub->property(FRAMELESSHELPER_PROP_NAME));
}

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
        q->setMinimumSize(kDefaultWindowSize);
        q->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(q->winId(), !value);
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
    if (m_settings.options & Option::CreateStandardWindowLayout) {
        if (m_systemTitleBarWidget && m_systemTitleBarWidget->isVisible()) {
            m_mainLayout->removeWidget(m_systemTitleBarWidget.data());
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
    if (!(m_settings.options & Option::CreateStandardWindowLayout)) {
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
    const bool standardLayout = (m_settings.options & Option::CreateStandardWindowLayout);
    if (type == QEvent::WindowStateChange) {
        if (standardLayout) {
            updateSystemMaximizeButton();
        }
        updateContentsMargins();
    }
    if (standardLayout) {
        updateSystemTitleBarStyleSheet();
    }
    q->update();
    if (type == QEvent::WindowStateChange) {
        QMetaObject::invokeMethod(q, "hiddenChanged");
        QMetaObject::invokeMethod(q, "normalChanged");
        QMetaObject::invokeMethod(q, "zoomedChanged");
    }
}

void FramelessWidgetsHelper::paintEventHandler(QPaintEvent *event)
{
#ifdef Q_OS_WINDOWS
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
    // We should use "q->width() - 1" actually but we can't because
    // Qt's drawing system have some rounding error internally and
    // if we minus one here we'll get a one pixel gap, so sad. But
    // drawing a line with extra pixels won't hurt anyway.
    painter.drawLine(0, 0, q->width(), 0);
    painter.restore();
#else
    Q_UNUSED(event);
#endif
}

void FramelessWidgetsHelper::initialize()
{
    // Let the user be able to get the helper class instance from outside.
    q->setProperty(FRAMELESSHELPER_PROP_NAME, QVariant::fromValue(this));
    // Without this flag, Qt will always create an invisible native parent window
    // for any native widgets which will intercept some win32 messages and confuse
    // our own native event filter, so to prevent some weired bugs from happening,
    // just disable this feature.
    q->setAttribute(Qt::WA_DontCreateNativeAncestors);
    // Force the widget become a native window now so that we can deal with its
    // win32 events as soon as possible.
    q->setAttribute(Qt::WA_NativeWindow);
    m_params.getWindowId = [this]() -> WId { return q->winId(); };
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
        return q->windowHandle()->screen();
#endif
    };
    m_params.isWindowFixedSize = [this]() -> bool { return isFixedSize(); };
    m_params.setWindowFixedSize = [this](const bool value) -> void { setFixedSize(value); };
    m_params.getWindowState = [this]() -> Qt::WindowState { return Utils::windowStatesToWindowState(q->windowState()); };
    m_params.setWindowState = [this](const Qt::WindowState state) -> void { q->setWindowState(state); };
    m_params.getWindowHandle = [this]() -> QWindow * { return q->windowHandle(); };
    m_params.windowToScreen = [this](const QPoint &pos) -> QPoint { return q->mapToGlobal(pos); };
    m_params.screenToWindow = [this](const QPoint &pos) -> QPoint { return q->mapFromGlobal(pos); };
    m_params.isInsideSystemButtons = [this](const QPoint &pos, SystemButtonType *button) -> bool { return isInSystemButtons(pos, button); };
    m_params.isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
    m_params.getWindowDevicePixelRatio = [this]() -> qreal { return q->devicePixelRatioF(); };
    m_params.setSystemButtonState = [this](const SystemButtonType button, const ButtonState state) -> void { setSystemButtonState(button, state); };
    m_params.shouldIgnoreMouseEvents = [this](const QPoint &pos) -> bool { return shouldIgnoreMouseEvents(pos); };
    m_params.showSystemMenu = [this](const QPoint &pos) -> void { showSystemMenu(pos); };
    if (m_settings.options & Option::CreateStandardWindowLayout) {
        if (q->inherits(QTWIDGETS_MAINWINDOW_CLASS_NAME)) {
            m_settings.options &= ~Options(Option::CreateStandardWindowLayout);
            qWarning() << "\"Option::CreateStandardWindowLayout\" is not compatible with QMainWindow and it's subclasses."
                          " Enabling this option will mess up with your main window's layout.";
        }
    }
#ifdef Q_OS_WINDOWS
    if (m_settings.options & Option::TransparentWindowBackground) {
        m_settings.options |= Option::BeCompatibleWithQtFramelessWindowHint;
    }
#endif
    if (m_settings.options & Option::DisableResizing) {
        setFixedSize(true, true);
    }
#ifdef Q_OS_WINDOWS
    if (m_settings.options & Option::BeCompatibleWithQtFramelessWindowHint) {
        Utils::tryToBeCompatibleWithQtFramelessWindowHint(q->winId(), m_params.getWindowFlags,
                                                          m_params.setWindowFlags, true);
    }
#endif
    if (m_settings.options & Option::TransparentWindowBackground) {
        q->setAttribute(Qt::WA_NoSystemBackground);
        q->setAttribute(Qt::WA_TranslucentBackground);
    }
    FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    manager->addWindow(m_settings, m_params);
    q->installEventFilter(this);
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this](){
        if (m_settings.options & Option::CreateStandardWindowLayout) {
            updateSystemTitleBarStyleSheet();
            q->update();
        }
        QMetaObject::invokeMethod(q, "systemThemeChanged");
    });
    setupInitialUi();
}

void FramelessWidgetsHelper::createSystemTitleBar()
{
    if (!(m_settings.options & Option::CreateStandardWindowLayout)) {
        return;
    }
    m_systemTitleBarWidget.reset(new QWidget(q));
    m_systemTitleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_systemTitleBarWidget->setFixedHeight(kDefaultTitleBarHeight);
    m_systemWindowTitleLabel.reset(new QLabel(m_systemTitleBarWidget.data()));
    m_systemWindowTitleLabel->setFrameShape(QFrame::NoFrame);
    QFont windowTitleFont = q->font();
    windowTitleFont.setPointSize(kDefaultTitleBarFontPointSize);
    m_systemWindowTitleLabel->setFont(windowTitleFont);
    m_systemWindowTitleLabel->setText(q->windowTitle());
    connect(q, &QWidget::windowTitleChanged, m_systemWindowTitleLabel.data(), &QLabel::setText);
    const auto minBtn = new StandardSystemButton(SystemButtonType::Minimize, m_systemTitleBarWidget.data());
    minBtn->setFixedSize(kDefaultSystemButtonSize);
    minBtn->setIconSize(kDefaultSystemButtonIconSize);
    minBtn->setToolTip(tr("Minimize"));
    connect(minBtn, &StandardSystemButton::clicked, q, &QWidget::showMinimized);
    m_settings.minimizeButton = minBtn;
    const auto maxBtn = new StandardSystemButton(SystemButtonType::Maximize, m_systemTitleBarWidget.data());
    maxBtn->setFixedSize(kDefaultSystemButtonSize);
    maxBtn->setIconSize(kDefaultSystemButtonIconSize);
    updateSystemMaximizeButton();
    connect(maxBtn, &StandardSystemButton::clicked, this, &FramelessWidgetsHelper::toggleMaximized);
    m_settings.maximizeButton = maxBtn;
    const auto closeBtn = new StandardSystemButton(SystemButtonType::Close, m_systemTitleBarWidget.data());
    closeBtn->setFixedSize(kDefaultSystemButtonSize);
    closeBtn->setIconSize(kDefaultSystemButtonIconSize);
    closeBtn->setToolTip(tr("Close"));
    connect(closeBtn, &StandardSystemButton::clicked, q, &QWidget::close);
    m_settings.closeButton = closeBtn;
    const auto systemTitleBarLayout = new QHBoxLayout(m_systemTitleBarWidget.data());
    systemTitleBarLayout->setContentsMargins(0, 0, 0, 0);
    systemTitleBarLayout->setSpacing(0);
    systemTitleBarLayout->addSpacerItem(new QSpacerItem(kDefaultTitleBarTitleLabelMargin, kDefaultTitleBarTitleLabelMargin));
    systemTitleBarLayout->addWidget(m_systemWindowTitleLabel.data());
    systemTitleBarLayout->addStretch();
    systemTitleBarLayout->addWidget(minBtn);
    systemTitleBarLayout->addWidget(maxBtn);
    systemTitleBarLayout->addWidget(closeBtn);
    m_systemTitleBarWidget->setLayout(systemTitleBarLayout);
}

void FramelessWidgetsHelper::createUserContentContainer()
{
    if (!(m_settings.options & Option::CreateStandardWindowLayout)) {
        return;
    }
    m_userContentContainerWidget.reset(new QWidget(q));
    m_userContentContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userContentContainerLayout.reset(new QVBoxLayout(m_userContentContainerWidget.data()));
    m_userContentContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_userContentContainerLayout->setSpacing(0);
    m_userContentContainerWidget->setLayout(m_userContentContainerLayout.data());
}

void FramelessWidgetsHelper::setupInitialUi()
{
    if (m_settings.options & Option::CreateStandardWindowLayout) {
        createSystemTitleBar();
        createUserContentContainer();
        m_mainLayout.reset(new QVBoxLayout(q));
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);
        m_mainLayout->addWidget(m_systemTitleBarWidget.data());
        m_mainLayout->addWidget(m_userContentContainerWidget.data());
        q->setLayout(m_mainLayout.data());
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
    if (m_settings.windowIconButton && m_settings.windowIconButton->isWidgetType()) {
        const auto iconBtn = qobject_cast<QWidget *>(m_settings.windowIconButton);
        if (iconBtn->geometry().contains(pos)) {
            *button = SystemButtonType::WindowIcon;
            return true;
        }
    }
    if (m_settings.contextHelpButton && m_settings.contextHelpButton->isWidgetType()) {
        const auto helpBtn = qobject_cast<QWidget *>(m_settings.contextHelpButton);
        if (helpBtn->geometry().contains(pos)) {
            *button = SystemButtonType::Help;
            return true;
        }
    }
    if (m_settings.minimizeButton && m_settings.minimizeButton->isWidgetType()) {
        const auto minBtn = qobject_cast<QWidget *>(m_settings.minimizeButton);
        if (minBtn->geometry().contains(pos)) {
            *button = SystemButtonType::Minimize;
            return true;
        }
    }
    if (m_settings.maximizeButton && m_settings.maximizeButton->isWidgetType()) {
        const auto maxBtn = qobject_cast<QWidget *>(m_settings.maximizeButton);
        if (maxBtn->geometry().contains(pos)) {
            *button = SystemButtonType::Maximize;
            return true;
        }
    }
    if (m_settings.closeButton && m_settings.closeButton->isWidgetType()) {
        const auto closeBtn = qobject_cast<QWidget *>(m_settings.closeButton);
        if (closeBtn->geometry().contains(pos)) {
            *button = SystemButtonType::Close;
            return true;
        }
    }
    return false;
}

bool FramelessWidgetsHelper::isInTitleBarDraggableArea(const QPoint &pos) const
{
    QRegion region = (m_userTitleBarWidget ? mapWidgetGeometryToScene(m_userTitleBarWidget) :
         (m_systemTitleBarWidget ? mapWidgetGeometryToScene(m_systemTitleBarWidget.data()) : QRegion{}));
    const auto systemButtons = {m_settings.windowIconButton, m_settings.contextHelpButton,
                  m_settings.minimizeButton, m_settings.maximizeButton, m_settings.closeButton};
    for (auto &&button : qAsConst(systemButtons)) {
        if (button && button->isWidgetType()) {
            const auto widgetButton = qobject_cast<QWidget *>(button);
            region -= mapWidgetGeometryToScene(widgetButton);
        }
    }
    if (!m_hitTestVisibleWidgets.isEmpty()) {
        for (auto &&widget : qAsConst(m_hitTestVisibleWidgets)) {
            Q_ASSERT(widget);
            if (widget) {
                region -= mapWidgetGeometryToScene(widget);
            }
        }
    }
    return region.contains(pos);
}

bool FramelessWidgetsHelper::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2)
            && isNormal() && !(m_settings.options & Option::DontDrawTopWindowFrameBorder));
#else
    return false;
#endif
}

bool FramelessWidgetsHelper::shouldIgnoreMouseEvents(const QPoint &pos) const
{
    const bool withinFrameBorder = [&pos, this]() -> bool {
        if (pos.y() < kDefaultResizeBorderThickness) {
            return true;
        }
#ifdef Q_OS_WINDOWS
        if (Utils::isWindowFrameBorderVisible()) {
            return false;
        }
#endif
        return ((pos.x() < kDefaultResizeBorderThickness)
                || (pos.x() >= (q->width() - kDefaultResizeBorderThickness)));
    }();
    return (isNormal() && withinFrameBorder);
}

void FramelessWidgetsHelper::setSystemButtonState(const SystemButtonType button, const ButtonState state)
{
    Q_ASSERT(button != SystemButtonType::Unknown);
    if (button == SystemButtonType::Unknown) {
        return;
    }
    QWidget *widgetButton = nullptr;
    switch (button) {
    case SystemButtonType::Unknown: {
        Q_ASSERT(false);
    } break;
    case SystemButtonType::WindowIcon: {
        if (m_settings.windowIconButton && m_settings.windowIconButton->isWidgetType()) {
            widgetButton = qobject_cast<QWidget *>(m_settings.windowIconButton);
        }
    } break;
    case SystemButtonType::Help: {
        if (m_settings.contextHelpButton && m_settings.contextHelpButton->isWidgetType()) {
            widgetButton = qobject_cast<QWidget *>(m_settings.contextHelpButton);
        }
    } break;
    case SystemButtonType::Minimize: {
        if (m_settings.minimizeButton && m_settings.minimizeButton->isWidgetType()) {
            widgetButton = qobject_cast<QWidget *>(m_settings.minimizeButton);
        }
    } break;
    case SystemButtonType::Maximize:
    case SystemButtonType::Restore: {
        if (m_settings.maximizeButton && m_settings.maximizeButton->isWidgetType()) {
            widgetButton = qobject_cast<QWidget *>(m_settings.maximizeButton);
        }
    } break;
    case SystemButtonType::Close: {
        if (m_settings.closeButton && m_settings.closeButton->isWidgetType()) {
            widgetButton = qobject_cast<QWidget *>(m_settings.closeButton);
        }
    } break;
    }
    if (widgetButton) {
        const auto updateButtonState = [state](QWidget *btn) -> void {
            Q_ASSERT(btn);
            if (!btn) {
                return;
            }
            switch (state) {
            case ButtonState::Unspecified: {
                QMetaObject::invokeMethod(btn, "setPressed", Q_ARG(bool, false));
                QMetaObject::invokeMethod(btn, "setHovered", Q_ARG(bool, false));
            } break;
            case ButtonState::Hovered: {
                QMetaObject::invokeMethod(btn, "setPressed", Q_ARG(bool, false));
                QMetaObject::invokeMethod(btn, "setHovered", Q_ARG(bool, true));
            } break;
            case ButtonState::Pressed: {
                QMetaObject::invokeMethod(btn, "setHovered", Q_ARG(bool, true));
                QMetaObject::invokeMethod(btn, "setPressed", Q_ARG(bool, true));
            } break;
            case ButtonState::Clicked: {
                // Clicked: pressed --> released, so behave like hovered.
                QMetaObject::invokeMethod(btn, "setPressed", Q_ARG(bool, false));
                QMetaObject::invokeMethod(btn, "setHovered", Q_ARG(bool, true));
                // Trigger the clicked signal.
                QMetaObject::invokeMethod(btn, "clicked");
            } break;
            }
        };
        updateButtonState(widgetButton);
    }
}

void FramelessWidgetsHelper::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    q->setContentsMargins(0, (shouldDrawFrameBorder() ? 1 : 0), 0, 0);
#endif
}

void FramelessWidgetsHelper::updateSystemTitleBarStyleSheet()
{
    if (!(m_settings.options & Option::CreateStandardWindowLayout)) {
        return;
    }
    const bool active = q->isActiveWindow();
    const bool dark = Utils::shouldAppsUseDarkMode();
    const bool colorizedTitleBar = Utils::isTitleBarColorized();
    const QColor systemTitleBarWidgetBackgroundColor = [active, colorizedTitleBar, dark]() -> QColor {
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
    const QColor systemWindowTitleLabelTextColor = (active ? ((dark || colorizedTitleBar) ? kDefaultWhiteColor : kDefaultBlackColor) : kDefaultDarkGrayColor);
    m_systemWindowTitleLabel->setStyleSheet(kStyleSheetColorTemplate.arg(systemWindowTitleLabelTextColor.name()));
    m_systemTitleBarWidget->setStyleSheet(kStyleSheetBackgroundColorTemplate.arg(systemTitleBarWidgetBackgroundColor.name()));
}

void FramelessWidgetsHelper::updateSystemMaximizeButton()
{
    if (const auto button = qobject_cast<StandardSystemButton *>(m_settings.maximizeButton)) {
        const bool zoomed = isZoomed();
        button->setToolTip(zoomed ? tr("Restore") : tr("Maximize"));
        button->setButtonType(zoomed ? SystemButtonType::Restore : SystemButtonType::Maximize);
    }
}

void FramelessWidgetsHelper::toggleMaximized()
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
    Utils::showSystemMenu(q->winId(), nativePos, m_settings.systemMenuOffset,
                          false, m_settings.options, m_params.isWindowFixedSize);
#else
    Q_UNUSED(pos);
#endif
}

void FramelessWidgetsHelper::startSystemMove2(const QPoint &pos)
{
    Utils::startSystemMove(q->windowHandle(), pos);
}

void FramelessWidgetsHelper::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    Utils::startSystemResize(q->windowHandle(), edges, pos);
}

void FramelessWidgetsHelper::setSystemButton(QWidget *widget, const SystemButtonType buttonType)
{
    Q_ASSERT(widget);
    Q_ASSERT(buttonType != SystemButtonType::Unknown);
    if (!widget || (buttonType == SystemButtonType::Unknown)) {
        return;
    }
    switch (buttonType) {
    case SystemButtonType::Unknown:
        Q_ASSERT(false);
        break;
    case SystemButtonType::WindowIcon:
        m_settings.windowIconButton = widget;
        break;
    case SystemButtonType::Help:
        m_settings.contextHelpButton = widget;
        break;
    case SystemButtonType::Minimize:
        m_settings.minimizeButton = widget;
        break;
    case SystemButtonType::Maximize:
    case SystemButtonType::Restore:
        m_settings.maximizeButton = widget;
        break;
    case SystemButtonType::Close:
        m_settings.closeButton = widget;
        break;
    }
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