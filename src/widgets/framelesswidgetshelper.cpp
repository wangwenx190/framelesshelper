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
#include "framelesswidgetshelper_p.h"
#include "framelesswidget.h"
#include "framelesswidget_p.h"
#include "framelessmainwindow.h"
#include "framelessmainwindow_p.h"
#include "framelessdialog.h"
#include "framelessdialog_p.h"
#include "widgetssharedhelper_p.h"
#include <QtCore/qmutex.h>
#include <QtCore/qhash.h>
#include <QtCore/qtimer.h>
#include <QtGui/qwindow.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qwidget.h>
#include <framelessmanager.h>
#include <framelessconfig_p.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessWidgetsHelper, "wangwenx190.framelesshelper.widgets.framelesswidgetshelper")
#define INFO qCInfo(lcFramelessWidgetsHelper)
#define DEBUG qCDebug(lcFramelessWidgetsHelper)
#define WARNING qCWarning(lcFramelessWidgetsHelper)
#define CRITICAL qCCritical(lcFramelessWidgetsHelper)

using namespace Global;

struct WidgetsHelperData
{
    bool ready = false;
    SystemParameters params = {};
    QPointer<QWidget> titleBarWidget = nullptr;
    QList<QPointer<QWidget>> hitTestVisibleWidgets = {};
    QPointer<QWidget> windowIconButton = nullptr;
    QPointer<QWidget> contextHelpButton = nullptr;
    QPointer<QWidget> minimizeButton = nullptr;
    QPointer<QWidget> maximizeButton = nullptr;
    QPointer<QWidget> closeButton = nullptr;
    QList<QRect> hitTestVisibleRects = {};
};

struct WidgetsHelper
{
    QMutex mutex;
    QHash<WId, WidgetsHelperData> data = {};
};

Q_GLOBAL_STATIC(WidgetsHelper, g_widgetsHelper)

FramelessWidgetsHelperPrivate::FramelessWidgetsHelperPrivate(FramelessWidgetsHelper *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
}

FramelessWidgetsHelperPrivate::~FramelessWidgetsHelperPrivate()
{
    m_destroying = true;
    extendsContentIntoTitleBar(false);
}

FramelessWidgetsHelperPrivate *FramelessWidgetsHelperPrivate::get(FramelessWidgetsHelper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessWidgetsHelperPrivate *FramelessWidgetsHelperPrivate::get(const FramelessWidgetsHelper *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

bool FramelessWidgetsHelperPrivate::isWindowFixedSize() const
{
    if (!m_window) {
        return false;
    }
    if (m_window->windowFlags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    const QSize minSize = m_window->minimumSize();
    const QSize maxSize = m_window->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    if (m_window->sizePolicy() == QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)) {
        return true;
    }
    return false;
}

void FramelessWidgetsHelperPrivate::setWindowFixedSize(const bool value)
{
    if (!m_window) {
        return;
    }
    if (isWindowFixedSize() == value) {
        return;
    }
    if (value) {
        m_window->setFixedSize(m_window->size());
    } else {
        m_window->setMinimumSize(kDefaultWindowSize);
        m_window->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
#ifdef Q_OS_WINDOWS
    Utils::setAeroSnappingEnabled(m_window->winId(), !value);
#endif
    emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("windowFixedSizeChanged"));
}

void FramelessWidgetsHelperPrivate::emitSignalForAllInstances(const QByteArray &signal)
{
    Q_ASSERT(!signal.isEmpty());
    if (signal.isEmpty()) {
        return;
    }
    if (!m_window) {
        return;
    }
    const auto instances = m_window->findChildren<FramelessWidgetsHelper *>();
    if (instances.isEmpty()) {
        return;
    }
    for (auto &&instance : qAsConst(instances)) {
        QMetaObject::invokeMethod(instance, signal.constData());
    }
}

bool FramelessWidgetsHelperPrivate::isBlurBehindWindowEnabled() const
{
    return m_blurBehindWindowEnabled;
}

void FramelessWidgetsHelperPrivate::setBlurBehindWindowEnabled(const bool enable, const QColor &color)
{
    if (!m_window) {
        return;
    }
    if (m_blurBehindWindowEnabled == enable) {
        return;
    }
    if (Utils::isBlurBehindWindowSupported()) {
        BlurMode mode = BlurMode::Disable;
        QPalette palette = m_window->palette();
        if (enable) {
            if (!m_savedWindowBackgroundColor.isValid()) {
                m_savedWindowBackgroundColor = palette.color(QPalette::Window);
            }
            palette.setColor(QPalette::Window, kDefaultTransparentColor);
            mode = BlurMode::Default;
        } else {
            if (m_savedWindowBackgroundColor.isValid()) {
                palette.setColor(QPalette::Window, m_savedWindowBackgroundColor);
                m_savedWindowBackgroundColor = {};
            }
            mode = BlurMode::Disable;
        }
        m_window->setPalette(palette);
        if (Utils::setBlurBehindWindowEnabled(m_window->winId(), mode, color)) {
            m_blurBehindWindowEnabled = enable;
            emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("blurBehindWindowEnabledChanged"));
        } else {
            WARNING << "Failed to enable/disable blur behind window.";
        }
    } else {
        if (WidgetsSharedHelper * const helper = findOrCreateSharedHelper(m_window)) {
            m_blurBehindWindowEnabled = enable;
            helper->setMicaEnabled(m_blurBehindWindowEnabled);
            emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("blurBehindWindowEnabledChanged"));
        } else {
            DEBUG << "Blur behind window is not supported on current platform.";
        }
    }
}

void FramelessWidgetsHelperPrivate::setProperty(const QByteArray &name, const QVariant &value)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(value.isValid());
    if (name.isEmpty() || !value.isValid()) {
        return;
    }
    Q_ASSERT(m_window);
    if (!m_window) {
        return;
    }
    m_window->setProperty(name.constData(), value);
}

QVariant FramelessWidgetsHelperPrivate::getProperty(const QByteArray &name, const QVariant &defaultValue)
{
    Q_ASSERT(!name.isEmpty());
    if (name.isEmpty()) {
        return {};
    }
    Q_ASSERT(m_window);
    if (!m_window) {
        return {};
    }
    const QVariant value = m_window->property(name.constData());
    return (value.isValid() ? value : defaultValue);
}

QWidget *FramelessWidgetsHelperPrivate::window() const
{
    return m_window;
}

MicaMaterial *FramelessWidgetsHelperPrivate::getMicaMaterialIfAny() const
{
    if (!m_window) {
        return nullptr;
    }
    if (const WidgetsSharedHelper * const helper = findOrCreateSharedHelper(m_window)) {
        return helper->rawMicaMaterial();
    }
    return nullptr;
}

WindowBorderPainter *FramelessWidgetsHelperPrivate::getWindowBorderIfAny() const
{
    if (!m_window) {
        return nullptr;
    }
    if (const WidgetsSharedHelper * const helper = findOrCreateSharedHelper(m_window)) {
        return helper->rawWindowBorder();
    }
    return nullptr;
}

WidgetsSharedHelper *FramelessWidgetsHelperPrivate::findOrCreateSharedHelper(QWidget *window)
{
    Q_ASSERT(window);
    if (!window) {
        return nullptr;
    }
    if (const auto widget = qobject_cast<FramelessWidget *>(window)) {
        if (const auto widgetPriv = FramelessWidgetPrivate::get(widget)) {
            return widgetPriv->widgetsSharedHelper();
        }
    }
    if (const auto mainWindow = qobject_cast<FramelessMainWindow *>(window)) {
        if (const auto mainWindowPriv = FramelessMainWindowPrivate::get(mainWindow)) {
            return mainWindowPriv->widgetsSharedHelper();
        }
    }
    if (const auto dialog = qobject_cast<FramelessDialog *>(window)) {
        if (const auto dialogPriv = FramelessDialogPrivate::get(dialog)) {
            return dialogPriv->widgetsSharedHelper();
        }
    }
    QWidget * const topLevelWindow = (window->nativeParentWidget()
        ? window->nativeParentWidget() : window->window());
    WidgetsSharedHelper *helper = topLevelWindow->findChild<WidgetsSharedHelper *>();
    if (!helper) {
        helper = new WidgetsSharedHelper;
        helper->setParent(topLevelWindow);
        helper->setup(topLevelWindow);
    }
    return helper;
}

FramelessWidgetsHelper *FramelessWidgetsHelperPrivate::findOrCreateFramelessHelper(QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return nullptr;
    }
    QObject *parent = nullptr;
    if (const auto widget = qobject_cast<QWidget *>(object)) {
        if (QWidget * const nativeParent = widget->nativeParentWidget()) {
            parent = nativeParent;
        } else {
            parent = widget->window();
        }
    } else {
        parent = object;
    }
    FramelessWidgetsHelper *instance = parent->findChild<FramelessWidgetsHelper *>();
    if (!instance) {
        instance = new FramelessWidgetsHelper(parent);
        instance->extendsContentIntoTitleBar();
    }
    return instance;
}

bool FramelessWidgetsHelperPrivate::isContentExtendedIntoTitleBar() const
{
    return getWindowData().ready;
}

void FramelessWidgetsHelperPrivate::setTitleBarWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    WidgetsHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    if (data->titleBarWidget == widget) {
        return;
    }
    data->titleBarWidget = widget;
    emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("titleBarWidgetChanged"));
}

QWidget *FramelessWidgetsHelperPrivate::getTitleBarWidget() const
{
    return getWindowData().titleBarWidget;
}

void FramelessWidgetsHelperPrivate::setHitTestVisible(QWidget *widget, const bool visible)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    WidgetsHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    const bool exists = data->hitTestVisibleWidgets.contains(widget);
    if (visible && !exists) {
        data->hitTestVisibleWidgets.append(widget);
    }
    if (!visible && exists) {
        data->hitTestVisibleWidgets.removeAll(widget);
    }
}

void FramelessWidgetsHelperPrivate::setHitTestVisible(const QRect &rect, const bool visible)
{
    Q_ASSERT(rect.isValid());
    if (!rect.isValid()) {
        return;
    }
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    WidgetsHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    const bool exists = data->hitTestVisibleRects.contains(rect);
    if (visible && !exists) {
        data->hitTestVisibleRects.append(rect);
    }
    if (!visible && exists) {
        data->hitTestVisibleRects.removeAll(rect);
    }
}

void FramelessWidgetsHelperPrivate::attach()
{
    QWidget * const window = findTopLevelWindow();
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    if (m_window == window) {
        return;
    }
    m_window = window;

    g_widgetsHelper()->mutex.lock();
    WidgetsHelperData * const data = getWindowDataMutable();
    if (!data || data->ready) {
        g_widgetsHelper()->mutex.unlock();
        return;
    }
    g_widgetsHelper()->mutex.unlock();

    // Without this flag, Qt will always create an invisible native parent window
    // for any native widgets which will intercept some win32 messages and confuse
    // our own native event filter, so to prevent some weired bugs from happening,
    // just disable this feature.
    window->setAttribute(Qt::WA_DontCreateNativeAncestors);
    // Force the widget become a native window now so that we can deal with its
    // win32 events as soon as possible.
    window->setAttribute(Qt::WA_NativeWindow);

    SystemParameters params = {};
    params.getWindowId = [window]() -> WId { return window->winId(); };
    params.getWindowFlags = [window]() -> Qt::WindowFlags { return window->windowFlags(); };
    params.setWindowFlags = [window](const Qt::WindowFlags flags) -> void { window->setWindowFlags(flags); };
    params.getWindowSize = [window]() -> QSize { return window->size(); };
    params.setWindowSize = [window](const QSize &size) -> void { window->resize(size); };
    params.getWindowPosition = [window]() -> QPoint { return window->pos(); };
    params.setWindowPosition = [window](const QPoint &pos) -> void { window->move(pos); };
    params.getWindowScreen = [window]() -> QScreen * {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return window->screen();
#else
        return window->windowHandle()->screen();
#endif
    };
    params.isWindowFixedSize = [this]() -> bool { return isWindowFixedSize(); };
    params.setWindowFixedSize = [this](const bool value) -> void { setWindowFixedSize(value); };
    params.getWindowState = [window]() -> Qt::WindowState { return Utils::windowStatesToWindowState(window->windowState()); };
    params.setWindowState = [window](const Qt::WindowState state) -> void { window->setWindowState(state); };
    params.getWindowHandle = [window]() -> QWindow * { return window->windowHandle(); };
    params.windowToScreen = [window](const QPoint &pos) -> QPoint { return window->mapToGlobal(pos); };
    params.screenToWindow = [window](const QPoint &pos) -> QPoint { return window->mapFromGlobal(pos); };
    params.isInsideSystemButtons = [this](const QPoint &pos, SystemButtonType *button) -> bool { return isInSystemButtons(pos, button); };
    params.isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
    params.getWindowDevicePixelRatio = [window]() -> qreal { return window->devicePixelRatioF(); };
    params.setSystemButtonState = [this](const SystemButtonType button, const ButtonState state) -> void { setSystemButtonState(button, state); };
    params.shouldIgnoreMouseEvents = [this](const QPoint &pos) -> bool { return shouldIgnoreMouseEvents(pos); };
    params.showSystemMenu = [this](const QPoint &pos) -> void { showSystemMenu(pos); };
    params.getCurrentApplicationType = []() -> ApplicationType { return ApplicationType::Widgets; };
    params.setProperty = [this](const QByteArray &name, const QVariant &value) -> void { setProperty(name, value); };
    params.getProperty = [this](const QByteArray &name, const QVariant &defaultValue) -> QVariant { return getProperty(name, defaultValue); };
    params.setCursor = [window](const QCursor &cursor) -> void { window->setCursor(cursor); };
    params.unsetCursor = [window]() -> void { window->unsetCursor(); };

    FramelessManager::instance()->addWindow(params);

    g_widgetsHelper()->mutex.lock();
    data->params = params;
    data->ready = true;
    g_widgetsHelper()->mutex.unlock();

    // We have to wait for a little time before moving the top level window
    // , because the platform window may not finish initializing by the time
    // we reach here, and all the modifications from the Qt side will be lost
    // due to QPA will reset the position and size of the window during it's
    // initialization process.
    QTimer::singleShot(0, this, [this](){
        if (FramelessConfig::instance()->isSet(Option::CenterWindowBeforeShow)) {
            moveWindowToDesktopCenter();
        }
        if (FramelessConfig::instance()->isSet(Option::EnableBlurBehindWindow)) {
            setBlurBehindWindowEnabled(true, {});
        }
        emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("windowChanged"));
        emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("ready"));
    });
}

void FramelessWidgetsHelperPrivate::detach()
{
    if (!m_window) {
        return;
    }
    const WId windowId = m_window->winId();
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    if (!g_widgetsHelper()->data.contains(windowId)) {
        return;
    }
    g_widgetsHelper()->data.remove(windowId);
    FramelessManager::instance()->removeWindow(windowId);
    m_window = nullptr;
    emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("windowChanged"));
}

void FramelessWidgetsHelperPrivate::extendsContentIntoTitleBar(const bool value)
{
    if (isContentExtendedIntoTitleBar() == value) {
        return;
    }
    if (value) {
        attach();
    } else {
        detach();
    }
    if (!m_destroying) {
        emitSignalForAllInstances(FRAMELESSHELPER_BYTEARRAY_LITERAL("extendsContentIntoTitleBarChanged"));
    }
}

QWidget *FramelessWidgetsHelperPrivate::findTopLevelWindow() const
{
    Q_Q(const FramelessWidgetsHelper);
    const QObject * const p = q->parent();
    Q_ASSERT(p);
    if (p) {
        if (const auto parentWidget = qobject_cast<const QWidget *>(p)) {
            if (QWidget * const nativeParent = parentWidget->nativeParentWidget()) {
                return nativeParent;
            }
            return parentWidget->window();
        }
    }
    return nullptr;
}

WidgetsHelperData FramelessWidgetsHelperPrivate::getWindowData() const
{
    //Q_ASSERT(m_window);
    if (!m_window) {
        return {};
    }
    const WId windowId = m_window->winId();
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    if (!g_widgetsHelper()->data.contains(windowId)) {
        g_widgetsHelper()->data.insert(windowId, {});
    }
    return g_widgetsHelper()->data.value(windowId);
}

WidgetsHelperData *FramelessWidgetsHelperPrivate::getWindowDataMutable() const
{
    //Q_ASSERT(m_window);
    if (!m_window) {
        return nullptr;
    }
    const WId windowId = m_window->winId();
    if (!g_widgetsHelper()->data.contains(windowId)) {
        g_widgetsHelper()->data.insert(windowId, {});
    }
    return &g_widgetsHelper()->data[windowId];
}

QRect FramelessWidgetsHelperPrivate::mapWidgetGeometryToScene(const QWidget * const widget) const
{
    Q_ASSERT(widget);
    if (!widget) {
        return {};
    }
    if (!m_window) {
        return {};
    }
    const QPoint originPoint = widget->mapTo(m_window, QPoint(0, 0));
    const QSize size = widget->size();
    return QRect(originPoint, size);
}

bool FramelessWidgetsHelperPrivate::isInSystemButtons(const QPoint &pos, SystemButtonType *button) const
{
    Q_ASSERT(button);
    if (!button) {
        return false;
    }
    *button = SystemButtonType::Unknown;
    const WidgetsHelperData data = getWindowData();
    if (data.windowIconButton && data.windowIconButton->isVisible() && data.windowIconButton->isEnabled()) {
        if (data.windowIconButton->geometry().contains(pos)) {
            *button = SystemButtonType::WindowIcon;
            return true;
        }
    }
    if (data.contextHelpButton && data.contextHelpButton->isVisible() && data.contextHelpButton->isEnabled()) {
        if (data.contextHelpButton->geometry().contains(pos)) {
            *button = SystemButtonType::Help;
            return true;
        }
    }
    if (data.minimizeButton && data.minimizeButton->isVisible() && data.minimizeButton->isEnabled()) {
        if (data.minimizeButton->geometry().contains(pos)) {
            *button = SystemButtonType::Minimize;
            return true;
        }
    }
    if (data.maximizeButton && data.maximizeButton->isVisible() && data.maximizeButton->isEnabled()) {
        if (data.maximizeButton->geometry().contains(pos)) {
            *button = SystemButtonType::Maximize;
            return true;
        }
    }
    if (data.closeButton && data.closeButton->isVisible() && data.closeButton->isEnabled()) {
        if (data.closeButton->geometry().contains(pos)) {
            *button = SystemButtonType::Close;
            return true;
        }
    }
    return false;
}

bool FramelessWidgetsHelperPrivate::isInTitleBarDraggableArea(const QPoint &pos) const
{
    const WidgetsHelperData data = getWindowData();
    if (!data.titleBarWidget) {
        // There's no title bar at all, the mouse will always be in the client area.
        return false;
    }
    if (!data.titleBarWidget->isVisible() || !data.titleBarWidget->isEnabled()) {
        // The title bar is hidden or disabled for some reason, treat it as there's no title bar.
        return false;
    }
    if (!m_window) {
        // The FramelessWidgetsHelper object has not been attached to a specific window yet,
        // so we assume there's no title bar.
        return false;
    }
    const QRect windowRect = {QPoint(0, 0), m_window->size()};
    const QRect titleBarRect = mapWidgetGeometryToScene(data.titleBarWidget);
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
            region -= mapWidgetGeometryToScene(button);
        }
    }
    if (!data.hitTestVisibleWidgets.isEmpty()) {
        for (auto &&widget : qAsConst(data.hitTestVisibleWidgets)) {
            if (widget && widget->isVisible() && widget->isEnabled()) {
                region -= mapWidgetGeometryToScene(widget);
            }
        }
    }
    if (!data.hitTestVisibleRects.isEmpty()) {
        for (auto &&rect : qAsConst(data.hitTestVisibleRects)) {
            if (rect.isValid()) {
                region -= rect;
            }
        }
    }
    return region.contains(pos);
}

bool FramelessWidgetsHelperPrivate::shouldIgnoreMouseEvents(const QPoint &pos) const
{
    if (!m_window) {
        return false;
    }
    const bool withinFrameBorder = [this, &pos]() -> bool {
        if (pos.y() < kDefaultResizeBorderThickness) {
            return true;
        }
#ifdef Q_OS_WINDOWS
        if (Utils::isWindowFrameBorderVisible()) {
            return false;
        }
#endif
        return ((pos.x() < kDefaultResizeBorderThickness)
                || (pos.x() >= (m_window->width() - kDefaultResizeBorderThickness)));
    }();
    return ((Utils::windowStatesToWindowState(m_window->windowState()) == Qt::WindowNoState) && withinFrameBorder);
}

void FramelessWidgetsHelperPrivate::setSystemButtonState(const SystemButtonType button, const ButtonState state)
{
    Q_ASSERT(button != SystemButtonType::Unknown);
    if (button == SystemButtonType::Unknown) {
        return;
    }
    const WidgetsHelperData data = getWindowData();
    QWidget *widgetButton = nullptr;
    switch (button) {
    case SystemButtonType::Unknown:
        Q_ASSERT(false);
        break;
    case SystemButtonType::WindowIcon:
        if (data.windowIconButton) {
            widgetButton = data.windowIconButton;
        }
        break;
    case SystemButtonType::Help:
        if (data.contextHelpButton) {
            widgetButton = data.contextHelpButton;
        }
        break;
    case SystemButtonType::Minimize:
        if (data.minimizeButton) {
            widgetButton = data.minimizeButton;
        }
        break;
    case SystemButtonType::Maximize:
    case SystemButtonType::Restore:
        if (data.maximizeButton) {
            widgetButton = data.maximizeButton;
        }
        break;
    case SystemButtonType::Close:
        if (data.closeButton) {
            widgetButton = data.closeButton;
        }
        break;
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
        if (const auto mo = widgetButton->metaObject()) {
            const int pressedIndex = mo->indexOfSlot(QMetaObject::normalizedSignature("setPressed(bool)").constData());
            const int hoveredIndex = mo->indexOfSlot(QMetaObject::normalizedSignature("setHovered(bool)").constData());
            const int clickedIndex = mo->indexOfSignal(QMetaObject::normalizedSignature("clicked()").constData());
            if ((pressedIndex >= 0) && (hoveredIndex >= 0) && (clickedIndex >= 0)) {
                updateButtonState(widgetButton);
            }
        }
    }
}

void FramelessWidgetsHelperPrivate::moveWindowToDesktopCenter()
{
    if (!m_window) {
        return;
    }
    Utils::moveWindowToDesktopCenter([this]() -> QScreen * {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return m_window->screen();
#else
        return m_window->windowHandle()->screen();
#endif
        },
        [this]() -> QSize { return m_window->size(); },
        [this](const QPoint &pos) -> void { m_window->move(pos); }, true);
}

void FramelessWidgetsHelperPrivate::bringWindowToFront()
{
    if (!m_window) {
        return;
    }
    if (m_window->isHidden()) {
        m_window->show();
    }
    if (m_window->isMinimized()) {
        m_window->setWindowState(m_window->windowState() & ~Qt::WindowMinimized);
    }
    m_window->raise();
    m_window->activateWindow();
}

void FramelessWidgetsHelperPrivate::showSystemMenu(const QPoint &pos)
{
#ifdef Q_OS_WINDOWS
    if (!m_window) {
        return;
    }
    const QPoint globalPos = m_window->mapToGlobal(pos);
    const QPoint nativePos = QPointF(QPointF(globalPos) * m_window->devicePixelRatioF()).toPoint();
    Utils::showSystemMenu(m_window->winId(), nativePos, false, [this]() -> bool { return isWindowFixedSize(); });
#else
    // ### TODO
    Q_UNUSED(pos);
#endif
}

void FramelessWidgetsHelperPrivate::windowStartSystemMove2(const QPoint &pos)
{
    if (!m_window) {
        return;
    }
    Utils::startSystemMove(m_window->windowHandle(), pos);
}

void FramelessWidgetsHelperPrivate::windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    if (!m_window) {
        return;
    }
    if (edges == Qt::Edges{}) {
        return;
    }
    Utils::startSystemResize(m_window->windowHandle(), edges, pos);
}

void FramelessWidgetsHelperPrivate::setSystemButton(QWidget *widget, const SystemButtonType buttonType)
{
    Q_ASSERT(widget);
    Q_ASSERT(buttonType != SystemButtonType::Unknown);
    if (!widget || (buttonType == SystemButtonType::Unknown)) {
        return;
    }
    const QMutexLocker locker(&g_widgetsHelper()->mutex);
    WidgetsHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    switch (buttonType) {
    case SystemButtonType::Unknown:
        Q_ASSERT(false);
        break;
    case SystemButtonType::WindowIcon:
        data->windowIconButton = widget;
        break;
    case SystemButtonType::Help:
        data->contextHelpButton = widget;
        break;
    case SystemButtonType::Minimize:
        data->minimizeButton = widget;
        break;
    case SystemButtonType::Maximize:
    case SystemButtonType::Restore:
        data->maximizeButton = widget;
        break;
    case SystemButtonType::Close:
        data->closeButton = widget;
        break;
    }
}

FramelessWidgetsHelper::FramelessWidgetsHelper(QObject *parent)
    : QObject(parent), d_ptr(new FramelessWidgetsHelperPrivate(this))
{
}

FramelessWidgetsHelper::~FramelessWidgetsHelper() = default;

FramelessWidgetsHelper *FramelessWidgetsHelper::get(QObject *object)
{
    Q_ASSERT(object);
    if (!object) {
        return nullptr;
    }
    return FramelessWidgetsHelperPrivate::findOrCreateFramelessHelper(object);
}

QWidget *FramelessWidgetsHelper::titleBarWidget() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->getTitleBarWidget();
}

bool FramelessWidgetsHelper::isWindowFixedSize() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->isWindowFixedSize();
}

bool FramelessWidgetsHelper::isBlurBehindWindowEnabled() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->isBlurBehindWindowEnabled();
}

QWidget *FramelessWidgetsHelper::window() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->window();
}

bool FramelessWidgetsHelper::isContentExtendedIntoTitleBar() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->isContentExtendedIntoTitleBar();
}

MicaMaterial *FramelessWidgetsHelper::micaMaterial() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->getMicaMaterialIfAny();
}

WindowBorderPainter *FramelessWidgetsHelper::windowBorder() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->getWindowBorderIfAny();
}

void FramelessWidgetsHelper::extendsContentIntoTitleBar(const bool value)
{
    Q_D(FramelessWidgetsHelper);
    d->extendsContentIntoTitleBar(value);
}

void FramelessWidgetsHelper::setTitleBarWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->setTitleBarWidget(widget);
}

void FramelessWidgetsHelper::setSystemButton(QWidget *widget, const SystemButtonType buttonType)
{
    Q_ASSERT(widget);
    Q_ASSERT(buttonType != SystemButtonType::Unknown);
    if (!widget || (buttonType == SystemButtonType::Unknown)) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->setSystemButton(widget, buttonType);
}

void FramelessWidgetsHelper::setHitTestVisible(QWidget *widget, const bool visible)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->setHitTestVisible(widget, visible);
}

void FramelessWidgetsHelper::setHitTestVisible(const QRect &rect, const bool visible)
{
    Q_ASSERT(rect.isValid());
    if (!rect.isValid()) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->setHitTestVisible(rect, visible);
}

void FramelessWidgetsHelper::showSystemMenu(const QPoint &pos)
{
    Q_D(FramelessWidgetsHelper);
    d->showSystemMenu(pos);
}

void FramelessWidgetsHelper::windowStartSystemMove2(const QPoint &pos)
{
    Q_D(FramelessWidgetsHelper);
    d->windowStartSystemMove2(pos);
}

void FramelessWidgetsHelper::windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos)
{
    if (edges == Qt::Edges{}) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->windowStartSystemResize2(edges, pos);
}

void FramelessWidgetsHelper::moveWindowToDesktopCenter()
{
    Q_D(FramelessWidgetsHelper);
    d->moveWindowToDesktopCenter();
}

void FramelessWidgetsHelper::bringWindowToFront()
{
    Q_D(FramelessWidgetsHelper);
    d->bringWindowToFront();
}

void FramelessWidgetsHelper::setWindowFixedSize(const bool value)
{
    Q_D(FramelessWidgetsHelper);
    d->setWindowFixedSize(value);
}

void FramelessWidgetsHelper::setBlurBehindWindowEnabled(const bool value)
{
    Q_D(FramelessWidgetsHelper);
    d->setBlurBehindWindowEnabled(value, {});
}

FRAMELESSHELPER_END_NAMESPACE
