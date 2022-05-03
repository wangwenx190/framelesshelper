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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4127 )
#pragma warning( disable : 4193 )
#pragma warning( disable : 4201 )
#pragma warning( disable : 4458 )
#pragma warning( disable : 4458 )
#endif

#include "framelessquickwindow.h"
#include "framelessquickwindow_p.h"
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <framelesswindowsmanager.h>
#include <utils.h>


FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

static constexpr const char QT_QUICKITEM_CLASS_NAME[] = "QQuickItem";

[[nodiscard]] static inline QuickGlobal::Options optionsCoreToQuick(const Options value)
{
    QuickGlobal::Options result = {};
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, ForceHideWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, ForceShowWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontDrawTopWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, EnableRoundedWindowCorners, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, TransparentWindowBackground, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, MaximizeButtonDocking, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, CreateStandardWindowLayout, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, BeCompatibleWithQtFramelessWindowHint, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchQtInternals, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchWindowFrameBorderColor, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontInstallSystemMenuHook, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DisableSystemMenu, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, NoDoubleClickMaximizeToggle, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DisableResizing, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DisableDragging, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchCursorShape, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontMoveWindowToDesktopCenter, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTreatFullScreenAsZoomed, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchHighDpiScalingPolicy, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchScaleFactorRoundingPolicy, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontTouchProcessDpiAwarenessLevel, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, DontEnsureNonNativeWidgetSiblings, value, result)
    FRAMELESSHELPER_FLAGS_CORE_TO_QUICK(Option, SyncNativeControlsThemeWithSystem, value, result)
    return result;
}

[[nodiscard]] static inline Options optionsQuickToCore(const QuickGlobal::Options value)
{
    Options result = {};
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, ForceHideWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, ForceShowWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontDrawTopWindowFrameBorder, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, EnableRoundedWindowCorners, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, TransparentWindowBackground, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, MaximizeButtonDocking, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, CreateStandardWindowLayout, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, BeCompatibleWithQtFramelessWindowHint, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchQtInternals, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchWindowFrameBorderColor, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontInstallSystemMenuHook, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DisableSystemMenu, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, NoDoubleClickMaximizeToggle, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DisableResizing, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DisableDragging, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchCursorShape, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontMoveWindowToDesktopCenter, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTreatFullScreenAsZoomed, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchHighDpiScalingPolicy, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchScaleFactorRoundingPolicy, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontTouchProcessDpiAwarenessLevel, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, DontEnsureNonNativeWidgetSiblings, value, result)
    FRAMELESSHELPER_FLAGS_QUICK_TO_CORE(Option, SyncNativeControlsThemeWithSystem, value, result)
    return result;
}

FramelessQuickWindowPrivate::FramelessQuickWindowPrivate(FramelessQuickWindow *q, const UserSettings &settings) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    m_settings = settings;
    initialize();
}

FramelessQuickWindowPrivate::~FramelessQuickWindowPrivate() = default;

FramelessQuickWindowPrivate *FramelessQuickWindowPrivate::get(FramelessQuickWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessQuickWindowPrivate *FramelessQuickWindowPrivate::get(const FramelessQuickWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

bool FramelessQuickWindowPrivate::isHidden() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Hidden);
}

bool FramelessQuickWindowPrivate::isNormal() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Windowed);
}

bool FramelessQuickWindowPrivate::isMinimized() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::Minimized);
}

bool FramelessQuickWindowPrivate::isZoomed() const
{
    Q_Q(const FramelessQuickWindow);
    const FramelessQuickWindow::Visibility visibility = q->visibility();
    return ((visibility == FramelessQuickWindow::Maximized) ||
            ((m_settings.options & Option::DontTreatFullScreenAsZoomed)
                 ? false : (visibility == FramelessQuickWindow::FullScreen)));
}

bool FramelessQuickWindowPrivate::isFullScreen() const
{
    Q_Q(const FramelessQuickWindow);
    return (q->visibility() == FramelessQuickWindow::FullScreen);
}

bool FramelessQuickWindowPrivate::isFixedSize() const
{
    if (m_settings.options & Option::DisableResizing) {
        return true;
    }
    Q_Q(const FramelessQuickWindow);
    if (q->flags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = q->minimumSize();
    const QSize maxSize = q->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    return false;
}

QColor FramelessQuickWindowPrivate::getFrameBorderColor() const
{
#ifdef Q_OS_WINDOWS
    Q_Q(const FramelessQuickWindow);
    return Utils::getFrameBorderColor(q->isActive());
#else
    return {};
#endif
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderTop() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::TopAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderBottom() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::BottomAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderLeft() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::LeftAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderRight() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::RightAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderHorizontalCenter() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::HCenterAnchor);
}

QQuickAnchorLine FramelessQuickWindowPrivate::getTopBorderVerticalCenter() const
{
    return QQuickAnchorLine(m_topBorderRectangle.data(), QQuickAnchors::VCenterAnchor);
}

void FramelessQuickWindowPrivate::setTitleBarItem(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    if (m_titleBarItem == item) {
        return;
    }
    m_titleBarItem = item;
    Q_Q(FramelessQuickWindow);
    Q_EMIT q->titleBarItemChanged();
}

void FramelessQuickWindowPrivate::setHitTestVisible(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    static constexpr const bool visible = true;
    const bool exists = m_hitTestVisibleItems.contains(item);
    if (visible && !exists) {
        m_hitTestVisibleItems.append(item);
    }
    if constexpr (!visible && exists) {
        m_hitTestVisibleItems.removeAll(item);
    }
}

void FramelessQuickWindowPrivate::moveToDesktopCenter()
{
    Utils::moveWindowToDesktopCenter(m_params.getWindowScreen, m_params.getWindowSize,
                                     m_params.setWindowPosition, true);
}

void FramelessQuickWindowPrivate::setFixedSize(const bool value, const bool force)
{
    if ((isFixedSize() == value) && !force) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    if (value) {
        const QSize size = q->size();
        q->setMinimumSize(size);
        q->setMaximumSize(size);
        q->setFlags(q->flags() | Qt::MSWindowsFixedSizeDialogHint);
    } else {
        q->setFlags(q->flags() & ~Qt::MSWindowsFixedSizeDialogHint);
        q->setMinimumSize(kDefaultWindowSize);
        q->setMaximumSize(QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(q->winId(), !value);
#endif
    Q_EMIT q->fixedSizeChanged();
}

void FramelessQuickWindowPrivate::bringToFront()
{
    Q_Q(FramelessQuickWindow);
    if (isHidden()) {
        q->show();
    }
    if (isMinimized()) {
        q->showNormal(); // ### FIXME !!!
    }
    q->raise();
    q->requestActivate();
}

void FramelessQuickWindowPrivate::snapToTopBorder(QQuickItem *item, const QuickGlobal::Anchor itemAnchor, const QuickGlobal::Anchor topBorderAnchor)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    const QQuickAnchorLine targetAnchorLine = [this, topBorderAnchor]() -> QQuickAnchorLine {
        switch (topBorderAnchor) {
        case QuickGlobal::Anchor::Top:
            return getTopBorderTop();
        case QuickGlobal::Anchor::Bottom:
            return getTopBorderBottom();
        case QuickGlobal::Anchor::Left:
            return getTopBorderLeft();
        case QuickGlobal::Anchor::Right:
            return getTopBorderRight();
        case QuickGlobal::Anchor::HorizontalCenter:
            return getTopBorderHorizontalCenter();
        case QuickGlobal::Anchor::VerticalCenter:
            return getTopBorderVerticalCenter();
        default:
            break;
        }
        return {};
    }();
    const QQuickItemPrivate * const itemPrivate = QQuickItemPrivate::get(item);
    QQuickAnchors * const anchors = itemPrivate->anchors();
    switch (itemAnchor) {
    case QuickGlobal::Anchor::Top:
        anchors->setTop(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Bottom:
        anchors->setBottom(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Left:
        anchors->setLeft(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Right:
        anchors->setRight(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::HorizontalCenter:
        anchors->setHorizontalCenter(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::VerticalCenter:
        anchors->setVerticalCenter(targetAnchorLine);
        break;
    case QuickGlobal::Anchor::Center:
        anchors->setCenterIn(m_topBorderRectangle.data());
        break;
    }
}

void FramelessQuickWindowPrivate::setOptions(const QuickGlobal::Options value)
{
    Q_Q(FramelessQuickWindow);
    if (m_quickOptions == value) {
        return;
    }
    // ### TODO: re-evaluate some usable options.
    m_quickOptions = value;
    m_settings.options = optionsQuickToCore(m_quickOptions);
    Q_EMIT q->optionsChanged();
}

bool FramelessQuickWindowPrivate::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!object->isWindowType()) {
        return QObject::eventFilter(object, event);
    }
    Q_Q(FramelessQuickWindow);
    const auto window = qobject_cast<FramelessQuickWindow *>(object);
    if (window != q) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::Show: {
        const auto showEvent = static_cast<QShowEvent *>(event);
        showEventHandler(showEvent);
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseMoveEventHandler(mouseEvent);
    } break;
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mousePressEventHandler(mouseEvent);
    } break;
    case QEvent::MouseButtonRelease: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseReleaseEventHandler(mouseEvent);
    } break;
    case QEvent::MouseButtonDblClick: {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        mouseDoubleClickEventHandler(mouseEvent);
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void FramelessQuickWindowPrivate::showMinimized2()
{
    Q_Q(FramelessQuickWindow);
#ifdef Q_OS_WINDOWS
    // Work-around a QtQuick bug: https://bugreports.qt.io/browse/QTBUG-69711
    // Don't use "SW_SHOWMINIMIZED" because it will activate the current window
    // instead of the next window in the Z order, which is not the default behavior
    // of native Win32 applications.
    ShowWindow(reinterpret_cast<HWND>(q->winId()), SW_MINIMIZE);
#else
    q->showMinimized();
#endif
}

void FramelessQuickWindowPrivate::toggleMaximized()
{
    if (isFixedSize()) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    if (isZoomed()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessQuickWindowPrivate::toggleFullScreen()
{
    if (isFixedSize()) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    if (isFullScreen()) {
        q->setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = q->visibility();
        q->showFullScreen();
    }
}

void FramelessQuickWindowPrivate::showSystemMenu(const QPoint &pos)
{
#ifdef Q_OS_WINDOWS
    Q_Q(FramelessQuickWindow);
    const QPoint globalPos = q->mapToGlobal(pos);
    const QPoint nativePos = QPointF(QPointF(globalPos) * q->effectiveDevicePixelRatio()).toPoint();
    Utils::showSystemMenu(q->winId(), nativePos, m_settings.systemMenuOffset,
                          false, m_settings.options, m_params.isWindowFixedSize);
#else
    Q_UNUSED(pos);
#endif
}

void FramelessQuickWindowPrivate::startSystemMove2(const QPoint &pos)
{
    Q_Q(FramelessQuickWindow);
    Utils::startSystemMove(q, pos);
}

void FramelessQuickWindowPrivate::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    if (isFixedSize()) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
    Q_Q(FramelessQuickWindow);
    Utils::startSystemResize(q, edges, pos);
}

void FramelessQuickWindowPrivate::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    Q_Q(FramelessQuickWindow);
    m_params.getWindowId = [q]() -> WId { return q->winId(); };
    m_params.getWindowFlags = [q]() -> Qt::WindowFlags { return q->flags(); };
    m_params.setWindowFlags = [q](const Qt::WindowFlags flags) -> void { q->setFlags(flags); };
    m_params.getWindowSize = [q]() -> QSize { return q->size(); };
    m_params.setWindowSize = [q](const QSize &size) -> void { q->resize(size); };
    m_params.getWindowPosition = [q]() -> QPoint { return q->position(); };
    m_params.setWindowPosition = [q](const QPoint &pos) -> void { q->setX(pos.x()); q->setY(pos.y()); };
    m_params.getWindowScreen = [q]() -> QScreen * { return q->screen(); };
    m_params.isWindowFixedSize = [this]() -> bool { return isFixedSize(); };
    m_params.setWindowFixedSize = [this](const bool value) -> void { setFixedSize(value); };
    m_params.getWindowState = [q]() -> Qt::WindowState { return q->windowState(); };
    m_params.setWindowState = [q](const Qt::WindowState state) -> void { q->setWindowState(state); };
    m_params.getWindowHandle = [q]() -> QWindow * { return q; };
    m_params.windowToScreen = [q](const QPoint &pos) -> QPoint { return q->mapToGlobal(pos); };
    m_params.screenToWindow = [q](const QPoint &pos) -> QPoint { return q->mapFromGlobal(pos); };
    m_params.isInsideSystemButtons = [this](const QPoint &pos, SystemButtonType *button) -> bool {
        QuickGlobal::SystemButtonType button2 = QuickGlobal::SystemButtonType::Unknown;
        const bool result = isInSystemButtons(pos, &button2);
        *button = FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, button2);
        return result;
    };
    m_params.isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
    m_params.getWindowDevicePixelRatio = [q]() -> qreal { return q->effectiveDevicePixelRatio(); };
    m_params.setSystemButtonState = [q](const SystemButtonType button, const ButtonState state) -> void {
        Q_ASSERT(button != SystemButtonType::Unknown);
        if (button == SystemButtonType::Unknown) {
            return;
        }
        Q_EMIT q->systemButtonStateChanged(FRAMELESSHELPER_ENUM_CORE_TO_QUICK(SystemButtonType, button),
                                           FRAMELESSHELPER_ENUM_CORE_TO_QUICK(ButtonState, state));
    };
    if (m_settings.options & Option::DisableResizing) {
        setFixedSize(true, true);
    }
    if (m_settings.options & Option::TransparentWindowBackground) {
        q->setColor(kDefaultTransparentColor);
    }
    m_quickOptions = optionsCoreToQuick(m_settings.options);
    FramelessWindowsManager * const manager = FramelessWindowsManager::instance();
    manager->addWindow(m_settings, m_params);
    q->installEventFilter(this);
    QQuickItem * const rootItem = q->contentItem();
    const QQuickItemPrivate * const rootItemPrivate = QQuickItemPrivate::get(rootItem);
    m_topBorderRectangle.reset(new QQuickRectangle(rootItem));
    m_topBorderRectangle->setColor(kDefaultTransparentColor);
    m_topBorderRectangle->setHeight(0.0);
    QQuickPen * const _border = m_topBorderRectangle->border();
    _border->setWidth(0.0);
    _border->setColor(kDefaultTransparentColor);
    updateTopBorderHeight();
    updateTopBorderColor();
    m_topBorderAnchors.reset(new QQuickAnchors(m_topBorderRectangle.data(), m_topBorderRectangle.data()));
    m_topBorderAnchors->setTop(rootItemPrivate->top());
    m_topBorderAnchors->setLeft(rootItemPrivate->left());
    m_topBorderAnchors->setRight(rootItemPrivate->right());
    connect(q, &FramelessQuickWindow::visibilityChanged, this, [this, q](){
        updateTopBorderHeight();
        Q_EMIT q->hiddenChanged();
        Q_EMIT q->normalChanged();
        Q_EMIT q->minimizedChanged();
        Q_EMIT q->zoomedChanged();
        Q_EMIT q->fullScreenChanged();
    });
    connect(q, &FramelessQuickWindow::activeChanged, this, &FramelessQuickWindowPrivate::updateTopBorderColor);
    connect(manager, &FramelessWindowsManager::systemThemeChanged, this, [this, q](){
        updateTopBorderColor();
        Q_EMIT q->frameBorderColorChanged();
    });
}

QRect FramelessQuickWindowPrivate::mapItemGeometryToScene(const QQuickItem * const item) const
{
    Q_ASSERT(item);
    if (!item) {
        return {};
    }
    const QPointF originPoint = item->mapToScene(QPointF(0.0, 0.0));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const QSizeF size = item->size();
#else
    const QSizeF size = {item->width(), item->height()};
#endif
    return QRectF(originPoint, size).toRect();
}

bool FramelessQuickWindowPrivate::isInSystemButtons(const QPoint &pos, QuickGlobal::SystemButtonType *button) const
{
    Q_ASSERT(button);
    if (!button) {
        return false;
    }
    *button = QuickGlobal::SystemButtonType::Unknown;
    if (!m_settings.windowIconButton && !m_settings.contextHelpButton
        && !m_settings.minimizeButton && !m_settings.maximizeButton && !m_settings.closeButton) {
        return false;
    }
    if (m_settings.windowIconButton && m_settings.windowIconButton->inherits(QT_QUICKITEM_CLASS_NAME)) {
        const auto iconBtn = qobject_cast<QQuickItem *>(m_settings.windowIconButton);
        if (mapItemGeometryToScene(iconBtn).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::WindowIcon;
            return true;
        }
    }
    if (m_settings.contextHelpButton && m_settings.contextHelpButton->inherits(QT_QUICKITEM_CLASS_NAME)) {
        const auto helpBtn = qobject_cast<QQuickItem *>(m_settings.contextHelpButton);
        if (mapItemGeometryToScene(helpBtn).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Help;
            return true;
        }
    }
    if (m_settings.minimizeButton && m_settings.minimizeButton->inherits(QT_QUICKITEM_CLASS_NAME)) {
        const auto minBtn = qobject_cast<QQuickItem *>(m_settings.minimizeButton);
        if (mapItemGeometryToScene(minBtn).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Minimize;
            return true;
        }
    }
    if (m_settings.maximizeButton && m_settings.maximizeButton->inherits(QT_QUICKITEM_CLASS_NAME)) {
        const auto maxBtn = qobject_cast<QQuickItem *>(m_settings.maximizeButton);
        if (mapItemGeometryToScene(maxBtn).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Maximize;
            return true;
        }
    }
    if (m_settings.closeButton && m_settings.closeButton->inherits(QT_QUICKITEM_CLASS_NAME)) {
        const auto closeBtn = qobject_cast<QQuickItem *>(m_settings.closeButton);
        if (mapItemGeometryToScene(closeBtn).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Close;
            return true;
        }
    }
    return false;
}

bool FramelessQuickWindowPrivate::isInTitleBarDraggableArea(const QPoint &pos) const
{
    if (!m_titleBarItem) {
        return false;
    }
    QRegion region = mapItemGeometryToScene(m_titleBarItem);
    if (!m_hitTestVisibleItems.isEmpty()) {
        for (auto &&item : qAsConst(m_hitTestVisibleItems)) {
            Q_ASSERT(item);
            if (item) {
                region -= mapItemGeometryToScene(item);
            }
        }
    }
    return region.contains(pos);
}

bool FramelessQuickWindowPrivate::shouldIgnoreMouseEvents(const QPoint &pos) const
{
    Q_Q(const FramelessQuickWindow);
    const bool withinFrameBorder = [&pos, q]() -> bool {
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

bool FramelessQuickWindowPrivate::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    return (Utils::isWindowFrameBorderVisible() && !Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2)
            && !(m_settings.options & Option::DontDrawTopWindowFrameBorder));
#else
    return false;
#endif
}

void FramelessQuickWindowPrivate::showEventHandler(QShowEvent *event)
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

void FramelessQuickWindowPrivate::mouseMoveEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (!m_mouseLeftButtonPressed) {
        return;
    }
    if (m_settings.options & Option::DisableDragging) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = event->scenePosition().toPoint();
    const QPoint globalPos = event->globalPosition().toPoint();
#else
    const QPoint scenePos = event->windowPos().toPoint();
    const QPoint globalPos = event->screenPos().toPoint();
#endif
    if (shouldIgnoreMouseEvents(scenePos)) {
        return;
    }
    if (!isInTitleBarDraggableArea(scenePos)) {
        return;
    }
    startSystemMove2(globalPos);
}

void FramelessQuickWindowPrivate::mousePressEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_mouseLeftButtonPressed = true;
    }
}

void FramelessQuickWindowPrivate::mouseReleaseEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    const Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton) {
        m_mouseLeftButtonPressed = false;
    }
    if (button != Qt::RightButton) {
        return;
    }
    if (m_settings.options & Option::DisableSystemMenu) {
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

void FramelessQuickWindowPrivate::mouseDoubleClickEventHandler(QMouseEvent *event)
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
    toggleMaximized();
}

QuickGlobal::Options FramelessQuickWindowPrivate::getOptions() const
{
    return m_quickOptions;
}

QQuickItem *FramelessQuickWindowPrivate::getTitleBarItem() const
{
    return m_titleBarItem;
}

void FramelessQuickWindowPrivate::updateTopBorderColor()
{
#ifdef Q_OS_WINDOWS
    if (!shouldDrawFrameBorder()) {
        return;
    }
    m_topBorderRectangle->setColor(getFrameBorderColor());
#endif
}

void FramelessQuickWindowPrivate::updateTopBorderHeight()
{
#ifdef Q_OS_WINDOWS
    if (!shouldDrawFrameBorder()) {
        return;
    }
    const qreal newHeight = (isNormal() ? 1.0 : 0.0);
    m_topBorderRectangle->setHeight(newHeight);
#endif
}


FramelessQuickWindow::FramelessQuickWindow(QWindow *parent, const UserSettings &settings) : QQuickWindow(parent)
{
    d_ptr.reset(new FramelessQuickWindowPrivate(this, settings));
}

FramelessQuickWindow::~FramelessQuickWindow() = default;

bool FramelessQuickWindow::isHidden() const
{
    Q_D(const FramelessQuickWindow);
    return d->isHidden();
}

bool FramelessQuickWindow::isNormal() const
{
    Q_D(const FramelessQuickWindow);
    return d->isNormal();
}

bool FramelessQuickWindow::isMinimized() const
{
    Q_D(const FramelessQuickWindow);
    return d->isMinimized();
}

bool FramelessQuickWindow::isZoomed() const
{
    Q_D(const FramelessQuickWindow);
    return d->isZoomed();
}

bool FramelessQuickWindow::isFullScreen() const
{
    Q_D(const FramelessQuickWindow);
    return d->isFullScreen();
}

bool FramelessQuickWindow::isFixedSize() const
{
    Q_D(const FramelessQuickWindow);
    return d->isFixedSize();
}

void FramelessQuickWindow::setFixedSize(const bool value)
{
    Q_D(FramelessQuickWindow);
    d->setFixedSize(value);
}

QColor FramelessQuickWindow::frameBorderColor() const
{
    Q_D(const FramelessQuickWindow);
    return d->getFrameBorderColor();
}

QuickGlobal::Options FramelessQuickWindow::options() const
{
    Q_D(const FramelessQuickWindow);
    return d->getOptions();
}

void FramelessQuickWindow::setOptions(const QuickGlobal::Options value)
{
    Q_D(FramelessQuickWindow);
    d->setOptions(value);
}

QQuickItem *FramelessQuickWindow::titleBarItem() const
{
    Q_D(const FramelessQuickWindow);
    return d->getTitleBarItem();
}

void FramelessQuickWindow::setTitleBarItem(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->setTitleBarItem(item);
}

void FramelessQuickWindow::setHitTestVisible(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->setHitTestVisible(item);
}

void FramelessQuickWindow::moveToDesktopCenter()
{
    Q_D(FramelessQuickWindow);
    d->moveToDesktopCenter();
}

void FramelessQuickWindow::bringToFront()
{
    Q_D(FramelessQuickWindow);
    d->bringToFront();
}

void FramelessQuickWindow::snapToTopBorder(QQuickItem *item, const QuickGlobal::Anchor itemAnchor, const QuickGlobal::Anchor topBorderAnchor)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->snapToTopBorder(item, itemAnchor, topBorderAnchor);
}

void FramelessQuickWindow::showMinimized2()
{
    Q_D(FramelessQuickWindow);
    d->showMinimized2();
}

void FramelessQuickWindow::toggleMaximized()
{
    Q_D(FramelessQuickWindow);
    d->toggleMaximized();
}

void FramelessQuickWindow::toggleFullScreen()
{
    Q_D(FramelessQuickWindow);
    d->toggleFullScreen();
}

void FramelessQuickWindow::showSystemMenu(const QPoint &pos)
{
    Q_D(FramelessQuickWindow);
    d->showSystemMenu(pos);
}

void FramelessQuickWindow::startSystemMove2(const QPoint &pos)
{
    Q_D(FramelessQuickWindow);
    d->startSystemMove2(pos);
}

void FramelessQuickWindow::startSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    if (edges == Qt::Edges{}) {
        return;
    }
    Q_D(FramelessQuickWindow);
    d->startSystemResize2(edges, pos);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif


FRAMELESSHELPER_END_NAMESPACE
