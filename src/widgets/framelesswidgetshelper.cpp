/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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
#include <FramelessHelper/Core/framelessmanager.h>
#include <FramelessHelper/Core/utils.h>
#include <FramelessHelper/Core/private/framelessconfig_p.h>
#include <FramelessHelper/Core/private/framelesshelpercore_global_p.h>
#include <QtCore/qhash.h>
#include <QtCore/qtimer.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qloggingcategory.h>
#include <QtGui/qwindow.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qwidget.h>

#ifndef QWIDGETSIZE_MAX
#  define QWIDGETSIZE_MAX ((1 << 24) - 1)
#endif // QWIDGETSIZE_MAX

FRAMELESSHELPER_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lcFramelessWidgetsHelper, "wangwenx190.framelesshelper.widgets.framelesswidgetshelper")

#ifdef FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessWidgetsHelper)
#  define DEBUG qCDebug(lcFramelessWidgetsHelper)
#  define WARNING qCWarning(lcFramelessWidgetsHelper)
#  define CRITICAL qCCritical(lcFramelessWidgetsHelper)
#endif

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
    QHash<WId, WidgetsHelperData> data = {};
};

Q_GLOBAL_STATIC(WidgetsHelper, g_widgetsHelper)

[[nodiscard]] static inline bool isWidgetFixedSize(const QWidget * const widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return false;
    }
    // "Qt::MSWindowsFixedSizeDialogHint" is used cross-platform actually.
    if (widget->windowFlags() & Qt::MSWindowsFixedSizeDialogHint) {
        return true;
    }
    // Caused by setFixedWidth/Height/Size().
    const QSize minSize = widget->minimumSize();
    const QSize maxSize = widget->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize)) {
        return true;
    }
    // Usually set by the user.
    const QSizePolicy sizePolicy = widget->sizePolicy();
    if ((sizePolicy.horizontalPolicy() == QSizePolicy::Fixed)
        && (sizePolicy.verticalPolicy() == QSizePolicy::Fixed)) {
        return true;
    }
    return false;
}

static inline void forceWidgetRepaint(QWidget * const widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    // Tell the widget to repaint itself, but it may not happen due to QWidget's
    // internal painting optimizations.
    widget->update();
    // Try to force the widget to repaint itself, in case:
    //   (1) It's a child widget;
    //   (2) It's a top level window but not minimized/maximized/fullscreen.
    if (!widget->isWindow() || !(widget->windowState() & (Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen))) {
        // A widget will most likely repaint itself if it's size is changed.
        if (!isWidgetFixedSize(widget)) {
            const QSize originalSize = widget->size();
            static constexpr const auto margins = QMargins{10, 10, 10, 10};
            widget->resize(originalSize.shrunkBy(margins));
            widget->resize(originalSize.grownBy(margins));
            widget->resize(originalSize);
        }
        // However, some widgets won't repaint themselves unless their position is changed.
        const QPoint originalPosition = widget->pos();
        static constexpr const auto offset = QPoint{10, 10};
        widget->move(originalPosition - offset);
        widget->move(originalPosition + offset);
        widget->move(originalPosition);
    }
#ifdef Q_OS_WINDOWS
    // There's some additional things to do for top level windows on Windows.
    if (widget->isWindow()) {
        // Don't crash if the QWindow instance has not been created yet.
        if (QWindow * const window = widget->windowHandle()) {
            // Sync the internal window frame margins with the latest DPI, otherwise
            // we will get wrong window sizes after the DPI change.
            Utils::updateInternalWindowFrameMargins(window, true);
        }
    }
#endif // Q_OS_WINDOWS
    // Let's try again with the ordinary way.
    widget->update();
    // ### TODO: I observed the font size is often wrong after DPI changes,
    // do we need to refresh the font settings here as well?
}

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
    return isWidgetFixedSize(m_window);
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
        m_savedSizePolicy = m_window->sizePolicy();
        m_window->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_window->setFixedSize(m_window->size());
    } else {
        m_window->setSizePolicy(m_savedSizePolicy);
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
    for (auto &&instance : std::as_const(instances)) {
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
        QPalette palette = m_window->palette();
        if (enable) {
            m_savedWindowBackgroundColor = palette.color(QPalette::Window);
        }
        palette.setColor(QPalette::Window, (enable ? kDefaultTransparentColor : m_savedWindowBackgroundColor));
        m_window->setPalette(palette);
        if (Utils::setBlurBehindWindowEnabled(m_window->winId(),
               (enable ? BlurMode::Default : BlurMode::Disable), color)) {
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
    QWidget * const topLevelWindow = window->window();
    WidgetsSharedHelper *helper = topLevelWindow->findChild<WidgetsSharedHelper *>();
    if (!helper) {
        helper = new WidgetsSharedHelper(topLevelWindow);
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
        parent = widget->window();
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

bool FramelessWidgetsHelperPrivate::isReady() const
{
    return m_qpaReady;
}

void FramelessWidgetsHelperPrivate::waitForReady()
{
    if (m_qpaReady) {
        return;
    }
#if 1
    QEventLoop loop;
    Q_Q(FramelessWidgetsHelper);
    const QMetaObject::Connection connection = connect(
        q, &FramelessWidgetsHelper::ready, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(connection);
#else
    while (!m_qpaReady) {
        QCoreApplication::processEvents();
    }
#endif
}

void FramelessWidgetsHelperPrivate::repaintAllChildren(const int delay) const
{
    if (!m_window) {
        return;
    }
    const auto update = [this]() -> void {
        forceWidgetRepaint(m_window);
        const QList<QWidget *> widgets = m_window->findChildren<QWidget *>();
        if (widgets.isEmpty()) {
            return;
        }
        for (auto &&widget : std::as_const(widgets)) {
            forceWidgetRepaint(widget);
        }
    };
    if (delay > 0) {
        QTimer::singleShot(delay, this, update);
    } else {
        update();
    }
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

void FramelessWidgetsHelperPrivate::setHitTestVisible(QObject *object, const bool visible)
{
    Q_ASSERT(object);
    if (!object) {
        return;
    }
    const auto widget = qobject_cast<QWidget *>(object);
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    setHitTestVisible(widget, visible);
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

    if (!window->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
        window->setAttribute(Qt::WA_DontCreateNativeAncestors);
    }
    if (!window->testAttribute(Qt::WA_NativeWindow)) {
        window->setAttribute(Qt::WA_NativeWindow);
    }

    WidgetsHelperData * const data = getWindowDataMutable();
    if (!data || data->ready) {
        return;
    }

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
    params.setProperty = [this](const QByteArray &name, const QVariant &value) -> void { setProperty(name, value); };
    params.getProperty = [this](const QByteArray &name, const QVariant &defaultValue) -> QVariant { return getProperty(name, defaultValue); };
    params.setCursor = [window](const QCursor &cursor) -> void { window->setCursor(cursor); };
    params.unsetCursor = [window]() -> void { window->unsetCursor(); };
    params.getWidgetHandle = [window]() -> QObject * { return window; };
    params.forceChildrenRepaint = [this](const int delay) -> void { repaintAllChildren(delay); };

    FramelessManager::instance()->addWindow(&params);

    data->params = params;
    data->ready = true;

    // We have to wait for a little time before moving the top level window
    // , because the platform window may not finish initializing by the time
    // we reach here, and all the modifications from the Qt side will be lost
    // due to QPA will reset the position and size of the window during it's
    // initialization process.
    QTimer::singleShot(0, this, [this](){
        m_qpaReady = true;
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
    for (auto &&button : std::as_const(systemButtons)) {
        if (button && button->isVisible() && button->isEnabled()) {
            region -= mapWidgetGeometryToScene(button);
        }
    }
    if (!data.hitTestVisibleWidgets.isEmpty()) {
        for (auto &&widget : std::as_const(data.hitTestVisibleWidgets)) {
            if (widget && widget->isVisible() && widget->isEnabled()) {
                region -= mapWidgetGeometryToScene(widget);
            }
        }
    }
    if (!data.hitTestVisibleRects.isEmpty()) {
        for (auto &&rect : std::as_const(data.hitTestVisibleRects)) {
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
    const auto withinFrameBorder = [this, &pos]() -> bool {
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
    case SystemButtonType::Unknown:
        Q_UNREACHABLE_RETURN(void(0));
    }
    if (!widgetButton) {
        return;
    }
    const auto updateButtonState = [state](QWidget *btn) -> void {
        Q_ASSERT(btn);
        if (!btn) {
            return;
        }
        switch (state) {
        case ButtonState::Normal: {
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
        case ButtonState::Released: {
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

void FramelessWidgetsHelperPrivate::moveWindowToDesktopCenter()
{
    if (!m_window) {
        return;
    }
    const SystemParameters params = getWindowData().params;
    Utils::moveWindowToDesktopCenter(&params, true);
}

void FramelessWidgetsHelperPrivate::bringWindowToFront()
{
    if (!m_window) {
        return;
    }
#ifdef Q_OS_WINDOWS
    Utils::bringWindowToFront(m_window->winId());
#else
    if (m_window->isHidden()) {
        m_window->show();
    }
    if (m_window->isMinimized()) {
        m_window->setWindowState(m_window->windowState() & ~Qt::WindowMinimized);
    }
    m_window->raise();
    m_window->activateWindow();
#endif
}

void FramelessWidgetsHelperPrivate::showSystemMenu(const QPoint &pos)
{
    if (!m_window) {
        return;
    }
    const WId windowId = m_window->winId();
    const QPoint nativePos = Utils::toNativeGlobalPosition(m_window->windowHandle(), pos);
#ifdef Q_OS_WINDOWS
    const SystemParameters params = getWindowData().params;
    Utils::showSystemMenu(windowId, nativePos, false, &params);
#elif defined(Q_OS_LINUX)
    Utils::openSystemMenu(windowId, nativePos);
#else
    Q_UNUSED(windowId);
    Q_UNUSED(nativePos);
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
    WidgetsHelperData *data = getWindowDataMutable();
    if (!data) {
        return;
    }
    switch (buttonType) {
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
    case SystemButtonType::Unknown:
        Q_UNREACHABLE_RETURN(void(0));
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

bool FramelessWidgetsHelper::isReady() const
{
    Q_D(const FramelessWidgetsHelper);
    return d->isReady();
}

void FramelessWidgetsHelper::waitForReady()
{
    Q_D(FramelessWidgetsHelper);
    d->waitForReady();
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

void FramelessWidgetsHelper::setHitTestVisible(QObject *object, const bool visible)
{
    Q_ASSERT(object);
    if (!object) {
        return;
    }
    Q_D(FramelessWidgetsHelper);
    d->setHitTestVisible(object, visible);
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
