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

#include "framelessquickhelper.h"
#include "framelessquickhelper_p.h"
#include <QtCore/qmutex.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qpa/qplatformwindow.h> // For QWINDOWSIZE_MAX
#else
#  include <QtGui/private/qwindow_p.h> // For QWINDOWSIZE_MAX
#endif
#include <QtQuick/qquickwindow.h>
#include <QtQuickTemplates2/private/qquickabstractbutton_p.h>
#include <framelessmanager.h>
#include <framelessconfig_p.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct QuickHelperData
{
    bool attached = false;
    SystemParameters params = {};
    QPointer<QQuickItem> titleBarItem = nullptr;
    QList<QPointer<QQuickItem>> hitTestVisibleItems = {};
    QPointer<QQuickItem> windowIconButton = nullptr;
    QPointer<QQuickItem> contextHelpButton = nullptr;
    QPointer<QQuickItem> minimizeButton = nullptr;
    QPointer<QQuickItem> maximizeButton = nullptr;
    QPointer<QQuickItem> closeButton = nullptr;
};

struct QuickHelper
{
    QMutex mutex;
    QHash<WId, QuickHelperData> data = {};
};

Q_GLOBAL_STATIC(QuickHelper, g_quickHelper)

FramelessQuickHelperPrivate::FramelessQuickHelperPrivate(FramelessQuickHelper *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
}

FramelessQuickHelperPrivate::~FramelessQuickHelperPrivate() = default;

FramelessQuickHelperPrivate *FramelessQuickHelperPrivate::get(FramelessQuickHelper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessQuickHelperPrivate *FramelessQuickHelperPrivate::get(const FramelessQuickHelper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

QQuickItem *FramelessQuickHelperPrivate::getTitleBarItem() const
{
    return getWindowData().titleBarItem;
}

void FramelessQuickHelperPrivate::setTitleBarItem(QQuickItem *value)
{
    Q_ASSERT(value);
    if (!value) {
        return;
    }
    QMutexLocker locker(&g_quickHelper()->mutex);
    QuickHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    if (data->titleBarItem == value) {
        return;
    }
    data->titleBarItem = value;
    emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("titleBarItemChanged"));
}

void FramelessQuickHelperPrivate::attachToWindow()
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    Q_ASSERT(window);
    if (!window) {
        return;
    }

    g_quickHelper()->mutex.lock();
    QuickHelperData *data = getWindowDataMutable();
    if (!data) {
        g_quickHelper()->mutex.unlock();
        return;
    }
    const bool attached = data->attached;
    g_quickHelper()->mutex.unlock();

    if (attached) {
        return;
    }

    SystemParameters params = {};
    params.getWindowId = [window]() -> WId { return window->winId(); };
    params.getWindowFlags = [window]() -> Qt::WindowFlags { return window->flags(); };
    params.setWindowFlags = [window](const Qt::WindowFlags flags) -> void { window->setFlags(flags); };
    params.getWindowSize = [window]() -> QSize { return window->size(); };
    params.setWindowSize = [window](const QSize &size) -> void { window->resize(size); };
    params.getWindowPosition = [window]() -> QPoint { return window->position(); };
    params.setWindowPosition = [window](const QPoint &pos) -> void { window->setX(pos.x()); window->setY(pos.y()); };
    params.getWindowScreen = [window]() -> QScreen * { return window->screen(); };
    params.isWindowFixedSize = [this]() -> bool { return isWindowFixedSize(); };
    params.setWindowFixedSize = [this](const bool value) -> void { setWindowFixedSize(value); };
    params.getWindowState = [window]() -> Qt::WindowState { return window->windowState(); };
    params.setWindowState = [window](const Qt::WindowState state) -> void { window->setWindowState(state); };
    params.getWindowHandle = [window]() -> QWindow * { return window; };
    params.windowToScreen = [window](const QPoint &pos) -> QPoint { return window->mapToGlobal(pos); };
    params.screenToWindow = [window](const QPoint &pos) -> QPoint { return window->mapFromGlobal(pos); };
    params.isInsideSystemButtons = [this](const QPoint &pos, SystemButtonType *button) -> bool {
        QuickGlobal::SystemButtonType button2 = QuickGlobal::SystemButtonType::Unknown;
        const bool result = isInSystemButtons(pos, &button2);
        *button = FRAMELESSHELPER_ENUM_QUICK_TO_CORE(SystemButtonType, button2);
        return result;
    };
    params.isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
    params.getWindowDevicePixelRatio = [window]() -> qreal { return window->effectiveDevicePixelRatio(); };
    params.setSystemButtonState = [this](const SystemButtonType button, const ButtonState state) -> void {
        setSystemButtonState(FRAMELESSHELPER_ENUM_CORE_TO_QUICK(SystemButtonType, button),
                             FRAMELESSHELPER_ENUM_CORE_TO_QUICK(ButtonState, state));
    };
    params.shouldIgnoreMouseEvents = [this](const QPoint &pos) -> bool { return shouldIgnoreMouseEvents(pos); };
    params.showSystemMenu = [this](const QPoint &pos) -> void { showSystemMenu(pos); };
    params.getCurrentApplicationType = []() -> ApplicationType { return ApplicationType::Quick; };

    g_quickHelper()->mutex.lock();
    data->params = params;
    data->attached = true;
    g_quickHelper()->mutex.unlock();

    FramelessManager::instance()->addWindow(params);

    // We have to wait for a little time before moving the top level window
    // , because the platform window may not finish initializing by the time
    // we reach here, and all the modifications from the Qt side will be lost
    // due to QPA will reset the position and size of the window during it's
    // initialization process.
    QTimer::singleShot(200, this, [this](){
        if (FramelessConfig::instance()->isSet(Option::CenterWindowBeforeShow)) {
            moveWindowToDesktopCenter();
        }
        if (FramelessConfig::instance()->isSet(Option::EnableBlurBehindWindow)) {
            setBlurBehindWindowEnabled(true, {});
        }
        emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("ready"));
    });
}

void FramelessQuickHelperPrivate::setSystemButton(QQuickItem *item, const QuickGlobal::SystemButtonType buttonType)
{
    Q_ASSERT(item);
    Q_ASSERT(buttonType != QuickGlobal::SystemButtonType::Unknown);
    if (!item || (buttonType == QuickGlobal::SystemButtonType::Unknown)) {
        return;
    }
    QMutexLocker locker(&g_quickHelper()->mutex);
    QuickHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    switch (buttonType) {
    case QuickGlobal::SystemButtonType::Unknown:
        Q_ASSERT(false);
        break;
    case QuickGlobal::SystemButtonType::WindowIcon:
        data->windowIconButton = item;
        break;
    case QuickGlobal::SystemButtonType::Help:
        data->contextHelpButton = item;
        break;
    case QuickGlobal::SystemButtonType::Minimize:
        data->minimizeButton = item;
        break;
    case QuickGlobal::SystemButtonType::Maximize:
    case QuickGlobal::SystemButtonType::Restore:
        data->maximizeButton = item;
        break;
    case QuickGlobal::SystemButtonType::Close:
        data->closeButton = item;
        break;
    }
}

void FramelessQuickHelperPrivate::setHitTestVisible(QQuickItem *item, const bool visible)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    QMutexLocker locker(&g_quickHelper()->mutex);
    QuickHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    const bool exists = data->hitTestVisibleItems.contains(item);
    if (visible && !exists) {
        data->hitTestVisibleItems.append(item);
    }
    if (!visible && exists) {
        data->hitTestVisibleItems.removeAll(item);
    }
}

void FramelessQuickHelperPrivate::showSystemMenu(const QPoint &pos)
{
#ifdef Q_OS_WINDOWS
    Q_Q(FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    const QPoint globalPos = window->mapToGlobal(pos);
    const QPoint nativePos = QPointF(QPointF(globalPos) * window->effectiveDevicePixelRatio()).toPoint();
    Utils::showSystemMenu(window->winId(), nativePos, false, [this]() -> bool { return isWindowFixedSize(); });
#else
    Q_UNUSED(pos);
#endif
}

void FramelessQuickHelperPrivate::windowStartSystemMove2(const QPoint &pos)
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    Utils::startSystemMove(window, pos);
}

void FramelessQuickHelperPrivate::windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
    Utils::startSystemResize(window, edges, pos);
}

void FramelessQuickHelperPrivate::moveWindowToDesktopCenter()
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    Utils::moveWindowToDesktopCenter([window]() -> QScreen * { return window->screen(); },
       [window]() -> QSize { return window->size(); },
       [window](const QPoint &pos) -> void { window->setX(pos.x()); window->setY(pos.y()); }, true);
}

void FramelessQuickHelperPrivate::bringWindowToFront()
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    if (window->visibility() == QQuickWindow::Hidden) {
        window->show();
    }
    if (window->visibility() == QQuickWindow::Minimized) {
        window->showNormal(); // ### FIXME: we should not show normal, we should restore the previous state.
    }
    window->raise();
    window->requestActivate();
}

bool FramelessQuickHelperPrivate::isWindowFixedSize() const
{
    Q_Q(const FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return false;
    }
    if (window->flags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = window->minimumSize();
    const QSize maxSize = window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    return false;
}

void FramelessQuickHelperPrivate::setWindowFixedSize(const bool value)
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    if (isWindowFixedSize() == value) {
        return;
    }
    if (value) {
        const QSize size = window->size();
        window->setMinimumSize(size);
        window->setMaximumSize(size);
    } else {
        window->setMinimumSize(kDefaultWindowSize);
        window->setMaximumSize(QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(window->winId(), !value);
#endif
    Q_EMIT q->windowFixedSizeChanged();
}

void FramelessQuickHelperPrivate::emitSignalForAllInstances(const QByteArray &signal)
{
    Q_ASSERT(!signal.isEmpty());
    if (signal.isEmpty()) {
        return;
    }
    Q_Q(FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    const auto rootObject = (window->contentItem() ? qobject_cast<const QObject *>(window->contentItem()) : qobject_cast<const QObject *>(window));
    const auto instances = rootObject->findChildren<FramelessQuickHelper *>();
    if (instances.isEmpty()) {
        return;
    }
    for (auto &&instance : qAsConst(instances)) {
        QMetaObject::invokeMethod(instance, signal.constData());
    }
}

bool FramelessQuickHelperPrivate::isBlurBehindWindowEnabled() const
{
    return m_blurBehindWindowEnabled;
}

void FramelessQuickHelperPrivate::setBlurBehindWindowEnabled(const bool value, const QColor &color)
{
    Q_Q(FramelessQuickHelper);
    QQuickWindow * const window = q->window();
    if (!window) {
        return;
    }
    if (m_blurBehindWindowEnabled == value) {
        return;
    }
    QuickGlobal::BlurMode mode = QuickGlobal::BlurMode::Disable;
    if (value) {
        if (!m_savedWindowBackgroundColor.isValid()) {
            m_savedWindowBackgroundColor = window->color();
        }
        window->setColor(kDefaultTransparentColor);
        mode = QuickGlobal::BlurMode::Default;
    } else {
        if (m_savedWindowBackgroundColor.isValid()) {
            window->setColor(m_savedWindowBackgroundColor);
            m_savedWindowBackgroundColor = {};
        }
        mode = QuickGlobal::BlurMode::Disable;
    }
    if (Utils::setBlurBehindWindowEnabled(window->winId(),
        FRAMELESSHELPER_ENUM_QUICK_TO_CORE(BlurMode, mode), color)) {
        m_blurBehindWindowEnabled = value;
        Q_EMIT q->blurBehindWindowEnabledChanged();
    } else {
        qWarning() << "Failed to enable/disable blur behind window.";
    }
}

QRect FramelessQuickHelperPrivate::mapItemGeometryToScene(const QQuickItem * const item) const
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

bool FramelessQuickHelperPrivate::isInSystemButtons(const QPoint &pos, QuickGlobal::SystemButtonType *button) const
{
    Q_ASSERT(button);
    if (!button) {
        return false;
    }
    *button = QuickGlobal::SystemButtonType::Unknown;
    const QuickHelperData data = getWindowData();
    if (data.windowIconButton && data.windowIconButton->isVisible() && data.windowIconButton->isEnabled()) {
        if (mapItemGeometryToScene(data.windowIconButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::WindowIcon;
            return true;
        }
    }
    if (data.contextHelpButton && data.contextHelpButton->isVisible() && data.contextHelpButton->isEnabled()) {
        if (mapItemGeometryToScene(data.contextHelpButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Help;
            return true;
        }
    }
    if (data.minimizeButton && data.minimizeButton->isVisible() && data.minimizeButton->isEnabled()) {
        if (mapItemGeometryToScene(data.minimizeButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Minimize;
            return true;
        }
    }
    if (data.maximizeButton && data.maximizeButton->isVisible() && data.maximizeButton->isEnabled()) {
        if (mapItemGeometryToScene(data.maximizeButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Maximize;
            return true;
        }
    }
    if (data.closeButton && data.closeButton->isVisible() && data.closeButton->isEnabled()) {
        if (mapItemGeometryToScene(data.closeButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Close;
            return true;
        }
    }
    return false;
}

bool FramelessQuickHelperPrivate::isInTitleBarDraggableArea(const QPoint &pos) const
{
    const QuickHelperData data = getWindowData();
    if (!data.titleBarItem) {
        // There's no title bar at all, the mouse will always be in the client area.
        return false;
    }
    if (!data.titleBarItem->isVisible() || !data.titleBarItem->isEnabled()) {
        // The title bar is hidden or disabled for some reason, treat it as there's no title bar.
        return false;
    }
    Q_Q(const FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    if (!window) {
        // The FramelessQuickHelper item has not been attached to a specific window yet,
        // so we assume there's no title bar.
        return false;
    }
    const QRect windowRect = {QPoint(0, 0), window->size()};
    const QRect titleBarRect = mapItemGeometryToScene(data.titleBarItem);
    if (!titleBarRect.intersects(windowRect)) {
        // The title bar is totally outside of the window for some reason,
        // also treat it as there's no title bar.
        return false;
    }
    QRegion region = titleBarRect;
    const auto systemButtons = {data.windowIconButton, data.contextHelpButton,
                     data.minimizeButton, data.maximizeButton, data.closeButton};
    for (auto &&button : qAsConst(systemButtons)) {
        if (button && button->isVisible() && button->isEnabled()) {
            region -= mapItemGeometryToScene(button);
        }
    }
    if (!data.hitTestVisibleItems.isEmpty()) {
        for (auto &&item : qAsConst(data.hitTestVisibleItems)) {
            if (item && item->isVisible() && item->isEnabled()) {
                region -= mapItemGeometryToScene(item);
            }
        }
    }
    return region.contains(pos);
}

bool FramelessQuickHelperPrivate::shouldIgnoreMouseEvents(const QPoint &pos) const
{
    Q_Q(const FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    if (!window) {
        return false;
    }
    const bool withinFrameBorder = [&pos, window]() -> bool {
        if (pos.y() < kDefaultResizeBorderThickness) {
            return true;
        }
#ifdef Q_OS_WINDOWS
        if (Utils::isWindowFrameBorderVisible()) {
            return false;
        }
#endif
        return ((pos.x() < kDefaultResizeBorderThickness)
                || (pos.x() >= (window->width() - kDefaultResizeBorderThickness)));
    }();
    return ((window->visibility() == QQuickWindow::Windowed) && withinFrameBorder);
}

void FramelessQuickHelperPrivate::setSystemButtonState(const QuickGlobal::SystemButtonType button,
                                                       const QuickGlobal::ButtonState state)
{
    Q_ASSERT(button != QuickGlobal::SystemButtonType::Unknown);
    if (button == QuickGlobal::SystemButtonType::Unknown) {
        return;
    }
    const QuickHelperData data = getWindowData();
    QQuickAbstractButton *quickButton = nullptr;
    switch (button) {
    case QuickGlobal::SystemButtonType::Unknown: {
        Q_ASSERT(false);
    } break;
    case QuickGlobal::SystemButtonType::WindowIcon: {
        if (data.windowIconButton) {
            if (const auto btn = qobject_cast<QQuickAbstractButton *>(data.windowIconButton)) {
                quickButton = btn;
            }
        }
    } break;
    case QuickGlobal::SystemButtonType::Help: {
        if (data.contextHelpButton) {
            if (const auto btn = qobject_cast<QQuickAbstractButton *>(data.contextHelpButton)) {
                quickButton = btn;
            }
        }
    } break;
    case QuickGlobal::SystemButtonType::Minimize: {
        if (data.minimizeButton) {
            if (const auto btn = qobject_cast<QQuickAbstractButton *>(data.minimizeButton)) {
                quickButton = btn;
            }
        }
    } break;
    case QuickGlobal::SystemButtonType::Maximize:
    case QuickGlobal::SystemButtonType::Restore: {
        if (data.maximizeButton) {
            if (const auto btn = qobject_cast<QQuickAbstractButton *>(data.maximizeButton)) {
                quickButton = btn;
            }
        }
    } break;
    case QuickGlobal::SystemButtonType::Close: {
        if (data.closeButton) {
            if (const auto btn = qobject_cast<QQuickAbstractButton *>(data.closeButton)) {
                quickButton = btn;
            }
        }
    } break;
    }
    if (quickButton) {
        const auto updateButtonState = [state](QQuickAbstractButton *btn) -> void {
            Q_ASSERT(btn);
            if (!btn) {
                return;
            }
            switch (state) {
            case QuickGlobal::ButtonState::Unspecified: {
                btn->setPressed(false);
                btn->setHovered(false);
            } break;
            case QuickGlobal::ButtonState::Hovered: {
                btn->setPressed(false);
                btn->setHovered(true);
            } break;
            case QuickGlobal::ButtonState::Pressed: {
                btn->setHovered(true);
                btn->setPressed(true);
            } break;
            case QuickGlobal::ButtonState::Clicked: {
                // Clicked: pressed --> released, so behave like hovered.
                btn->setPressed(false);
                btn->setHovered(true);
                // "QQuickAbstractButtonPrivate::click()"'s implementation is nothing but
                // only emits the "clicked" signal of the public interface, so we just emit
                // the signal directly to avoid accessing the private implementation.
                Q_EMIT btn->clicked();
            } break;
            }
        };
        updateButtonState(quickButton);
    }
}

QuickHelperData FramelessQuickHelperPrivate::getWindowData() const
{
    Q_Q(const FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    //Q_ASSERT(window);
    if (!window) {
        return {};
    }
    const WId windowId = window->winId();
    QMutexLocker locker(&g_quickHelper()->mutex);
    if (!g_quickHelper()->data.contains(windowId)) {
        g_quickHelper()->data.insert(windowId, {});
    }
    return g_quickHelper()->data.value(windowId);
}

QuickHelperData *FramelessQuickHelperPrivate::getWindowDataMutable() const
{
    Q_Q(const FramelessQuickHelper);
    const QQuickWindow * const window = q->window();
    //Q_ASSERT(window);
    if (!window) {
        return nullptr;
    }
    const WId windowId = window->winId();
    if (!g_quickHelper()->data.contains(windowId)) {
        g_quickHelper()->data.insert(windowId, {});
    }
    return &g_quickHelper()->data[windowId];
}

FramelessQuickHelper::FramelessQuickHelper(QQuickItem *parent)
    : QQuickItem(parent), d_ptr(new FramelessQuickHelperPrivate(this))
{
}

FramelessQuickHelper::~FramelessQuickHelper() = default;

FramelessQuickHelper *FramelessQuickHelper::get(QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return nullptr;
    }
    FramelessQuickHelper *instance = nullptr;
    QObject *parent = nullptr;
    if (const auto item = qobject_cast<QQuickItem *>(object)) {
        parent = ((item->window() && item->window()->contentItem()) ? item->window()->contentItem() : item);
    } else {
        parent = object;
    }
    instance = parent->findChild<FramelessQuickHelper *>();
    if (!instance) {
        instance = new FramelessQuickHelper;
        if (const auto item = qobject_cast<QQuickItem *>(parent)) {
            instance->setParentItem(item);
        }
        instance->setParent(parent);
        // No need to do this here, we'll do it once the item has been assigned to a specific window.
        //instance->d_func()->attachToWindow();
    }
    return instance;
}

FramelessQuickHelper *FramelessQuickHelper::qmlAttachedProperties(QObject *parentObject)
{
    Q_ASSERT(parentObject);
    if (!parentObject) {
        return nullptr;
    }
    return get(parentObject);
}

QQuickItem *FramelessQuickHelper::titleBarItem() const
{
    Q_D(const FramelessQuickHelper);
    return d->getTitleBarItem();
}

bool FramelessQuickHelper::isWindowFixedSize() const
{
    Q_D(const FramelessQuickHelper);
    return d->isWindowFixedSize();
}

bool FramelessQuickHelper::isBlurBehindWindowEnabled() const
{
    Q_D(const FramelessQuickHelper);
    return d->isBlurBehindWindowEnabled();
}

void FramelessQuickHelper::extendsContentIntoTitleBar()
{
    // Intentionally not doing anything here.
}

void FramelessQuickHelper::setTitleBarItem(QQuickItem *value)
{
    Q_ASSERT(value);
    if (!value) {
        return;
    }
    Q_D(FramelessQuickHelper);
    d->setTitleBarItem(value);
}

void FramelessQuickHelper::setSystemButton(QQuickItem *item, const QuickGlobal::SystemButtonType buttonType)
{
    Q_ASSERT(item);
    Q_ASSERT(buttonType != QuickGlobal::SystemButtonType::Unknown);
    if (!item || (buttonType == QuickGlobal::SystemButtonType::Unknown)) {
        return;
    }
    Q_D(FramelessQuickHelper);
    d->setSystemButton(item, buttonType);
}

void FramelessQuickHelper::setHitTestVisible(QQuickItem *item, const bool visible)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickHelper);
    d->setHitTestVisible(item, visible);
}

void FramelessQuickHelper::showSystemMenu(const QPoint &pos)
{
    Q_D(FramelessQuickHelper);
    d->showSystemMenu(pos);
}

void FramelessQuickHelper::windowStartSystemMove2(const QPoint &pos)
{
    Q_D(FramelessQuickHelper);
    d->windowStartSystemMove2(pos);
}

void FramelessQuickHelper::windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    if (edges == Qt::Edges{}) {
        return;
    }
    Q_D(FramelessQuickHelper);
    d->windowStartSystemResize2(edges, pos);
}

void FramelessQuickHelper::moveWindowToDesktopCenter()
{
    Q_D(FramelessQuickHelper);
    d->moveWindowToDesktopCenter();
}

void FramelessQuickHelper::bringWindowToFront()
{
    Q_D(FramelessQuickHelper);
    d->bringWindowToFront();
}

void FramelessQuickHelper::setWindowFixedSize(const bool value)
{
    Q_D(FramelessQuickHelper);
    d->setWindowFixedSize(value);
}

void FramelessQuickHelper::setBlurBehindWindowEnabled(const bool value)
{
    Q_D(FramelessQuickHelper);
    d->setBlurBehindWindowEnabled(value, {});
}

void FramelessQuickHelper::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        QQuickItem * const rootItem = value.window->contentItem();
        if (rootItem) {
            if ((parentItem() != rootItem) || (parent() != rootItem)) {
                setParentItem(rootItem);
                setParent(rootItem);
            }
        } else {
            if (parent() != value.window) {
                setParent(value.window);
            }
        }
        Q_D(FramelessQuickHelper);
        d->attachToWindow();
    }
}

FRAMELESSHELPER_END_NAMESPACE
