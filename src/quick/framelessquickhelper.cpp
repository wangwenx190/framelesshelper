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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qpa/qplatformwindow.h> // For QWINDOWSIZE_MAX
#else
#  include <QtGui/private/qwindow_p.h> // For QWINDOWSIZE_MAX
#endif
#include <QtQuick/qquickwindow.h>
#include <QtQuickTemplates2/private/qquickabstractbutton_p.h>
#include <framelessmanager.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct QuickHelperData
{
    bool attached = false;
    SystemParameters params = {};
    QPointer<QQuickItem> titleBarItem = nullptr;
    QList<QQuickItem *> hitTestVisibleItems = {};
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

static constexpr const char QTQUICK_ITEM_CLASS_NAME[] = "QQuickItem";
static constexpr const char QTQUICK_BUTTON_CLASS_NAME[] = "QQuickAbstractButton";

[[nodiscard]] static inline bool isItem(const QObject * const object)
{
    Q_ASSERT(object);
    if (!object) {
        return false;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    return object->isQuickItemType();
#else
    if (object->inherits(QTQUICK_ITEM_CLASS_NAME)) {
        return true;
    }
    if (const auto mo = object->metaObject()) {
        return (qstrcmp(mo->className(), QTQUICK_ITEM_CLASS_NAME) == 0);
    }
    return false;
#endif
}

[[nodiscard]] static inline bool isButton(const QObject * const object)
{
    Q_ASSERT(object);
    if (!object) {
        return false;
    }
    if (object->inherits(QTQUICK_BUTTON_CLASS_NAME)) {
        return true;
    }
    if (const auto mo = object->metaObject()) {
        return (qstrcmp(mo->className(), QTQUICK_BUTTON_CLASS_NAME) == 0);
    }
    return false;
}

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
    Q_Q(FramelessQuickHelper);
    Q_EMIT q->titleBarItemChanged();
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
    params.getWindowHandle = [q]() -> QWindow * { return q->window(); };
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

    g_quickHelper()->mutex.lock();
    data->params = params;
    data->attached = true;
    g_quickHelper()->mutex.unlock();

    FramelessManager::instance()->addWindow(params);
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

void FramelessQuickHelperPrivate::setHitTestVisible(QQuickItem *item)
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
    static constexpr const bool visible = true;
    const bool exists = data->hitTestVisibleItems.contains(item);
    if (visible && !exists) {
        data->hitTestVisibleItems.append(item);
    }
    if constexpr (!visible && exists) {
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
        window->showNormal(); // ### FIXME
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
        window->setFlags(window->flags() | Qt::MSWindowsFixedSizeDialogHint);
    } else {
        window->setFlags(window->flags() & ~Qt::MSWindowsFixedSizeDialogHint);
        window->setMinimumSize(kDefaultWindowSize);
        window->setMaximumSize(QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(window->winId(), !value);
#endif
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
    if (data.windowIconButton) {
        if (mapItemGeometryToScene(data.windowIconButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::WindowIcon;
            return true;
        }
    }
    if (data.contextHelpButton) {
        if (mapItemGeometryToScene(data.contextHelpButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Help;
            return true;
        }
    }
    if (data.minimizeButton) {
        if (mapItemGeometryToScene(data.minimizeButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Minimize;
            return true;
        }
    }
    if (data.maximizeButton) {
        if (mapItemGeometryToScene(data.maximizeButton).contains(pos)) {
            *button = QuickGlobal::SystemButtonType::Maximize;
            return true;
        }
    }
    if (data.closeButton) {
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
        return false;
    }
    QRegion region = mapItemGeometryToScene(data.titleBarItem);
    const auto systemButtons = {data.windowIconButton, data.contextHelpButton,
                     data.minimizeButton, data.maximizeButton, data.closeButton};
    for (auto &&button : qAsConst(systemButtons)) {
        if (button) {
            region -= mapItemGeometryToScene(button);
        }
    }
    if (!data.hitTestVisibleItems.isEmpty()) {
        for (auto &&item : qAsConst(data.hitTestVisibleItems)) {
            Q_ASSERT(item);
            if (item) {
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
        if (data.windowIconButton && isButton(data.windowIconButton)) {
            quickButton = qobject_cast<QQuickAbstractButton *>(data.windowIconButton);
        }
    } break;
    case QuickGlobal::SystemButtonType::Help: {
        if (data.contextHelpButton && isButton(data.contextHelpButton)) {
            quickButton = qobject_cast<QQuickAbstractButton *>(data.contextHelpButton);
        }
    } break;
    case QuickGlobal::SystemButtonType::Minimize: {
        if (data.minimizeButton && isButton(data.minimizeButton)) {
            quickButton = qobject_cast<QQuickAbstractButton *>(data.minimizeButton);
        }
    } break;
    case QuickGlobal::SystemButtonType::Maximize:
    case QuickGlobal::SystemButtonType::Restore: {
        if (data.maximizeButton && isButton(data.maximizeButton)) {
            quickButton = qobject_cast<QQuickAbstractButton *>(data.maximizeButton);
        }
    } break;
    case QuickGlobal::SystemButtonType::Close: {
        if (data.closeButton && isButton(data.closeButton)) {
            quickButton = qobject_cast<QQuickAbstractButton *>(data.closeButton);
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
                // emits the "clicked" signal of the public interface, so we just emit
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
    if (isItem(object)) {
        const auto item = qobject_cast<QQuickItem *>(object);
        parent = (item->window() ? item->window()->contentItem() : item);
    } else {
        parent = object;
    }
    instance = parent->findChild<FramelessQuickHelper *>();
    if (!instance) {
        instance = new FramelessQuickHelper;
        if (isItem(parent)) {
            instance->setParentItem(qobject_cast<QQuickItem *>(parent));
        } else {
            instance->setParent(parent);
        }
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

void FramelessQuickHelper::setHitTestVisible(QQuickItem *item)
{
    Q_ASSERT(item);
    if (!item) {
        return;
    }
    Q_D(FramelessQuickHelper);
    d->setHitTestVisible(item);
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

void FramelessQuickHelper::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        if (parentItem() != value.window->contentItem()) {
            setParentItem(value.window->contentItem());
        }
        Q_D(FramelessQuickHelper);
        d->attachToWindow();
        d->moveWindowToDesktopCenter(); // Temp hack
    }
}

FRAMELESSHELPER_END_NAMESPACE
