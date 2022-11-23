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

#include "framelesshelper_qt.h"
#include <QtCore/qmutex.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include "framelessmanager.h"
#include "framelessmanager_p.h"
#include "framelessconfig_p.h"
#include "utils.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessHelperQt, "wangwenx190.framelesshelper.core.impl.qt")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessHelperQt)
#  define DEBUG qCDebug(lcFramelessHelperQt)
#  define WARNING qCWarning(lcFramelessHelperQt)
#  define CRITICAL qCCritical(lcFramelessHelperQt)
#endif

using namespace Global;

struct QtHelperData
{
    SystemParameters params = {};
    FramelessHelperQt *eventFilter = nullptr;
    bool cursorShapeChanged = false;
    bool leftButtonPressed = false;
};

struct QtHelper
{
    QMutex mutex;
    QHash<WId, QtHelperData> data = {};
};

Q_GLOBAL_STATIC(QtHelper, g_qtHelper)

FramelessHelperQt::FramelessHelperQt(QObject *parent) : QObject(parent) {}

FramelessHelperQt::~FramelessHelperQt() = default;

void FramelessHelperQt::addWindow(const SystemParameters &params)
{
    Q_ASSERT(params.isValid());
    if (!params.isValid()) {
        return;
    }
    const WId windowId = params.getWindowId();
    g_qtHelper()->mutex.lock();
    if (g_qtHelper()->data.contains(windowId)) {
        g_qtHelper()->mutex.unlock();
        return;
    }
    QtHelperData data = {};
    data.params = params;
    QWindow *window = params.getWindowHandle();
    // Give it a parent so that it can be deleted even if we forget to do so.
    data.eventFilter = new FramelessHelperQt(window);
    g_qtHelper()->data.insert(windowId, data);
    g_qtHelper()->mutex.unlock();
    const auto shouldApplyFramelessFlag = [&params]() -> bool {
#ifdef Q_OS_MACOS
        const auto widget = params.getWidgetHandle();
        if (!(widget && widget->isWidgetType())) {
            return false;
        }
#else
        Q_UNUSED(params);
#endif
        return true;
    }();
    if (shouldApplyFramelessFlag) {
        params.setWindowFlags(params.getWindowFlags() | Qt::FramelessWindowHint);
    }
    window->installEventFilter(data.eventFilter);
#ifdef Q_OS_MACOS
#  if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    window->setProperty("_q_mac_wantsLayer", 1);
#  endif
    Utils::setSystemTitleBarVisible(windowId, false);
#endif
    FramelessHelper::Core::setApplicationOSThemeAware();
}

void FramelessHelperQt::removeWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    const QMutexLocker locker(&g_qtHelper()->mutex);
    if (!g_qtHelper()->data.contains(windowId)) {
        return;
    }
    if (const auto eventFilter = g_qtHelper()->data.value(windowId).eventFilter) {
        if (QWindow * const window = Utils::findWindow(windowId)) {
            window->removeEventFilter(eventFilter);
        }
        delete eventFilter;
    }
    g_qtHelper()->data.remove(windowId);
#ifdef Q_OS_MACOS
    Utils::removeWindowProxy(windowId);
#endif
}

bool FramelessHelperQt::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    // First detect whether we got a theme change event or not, if so,
    // inform the user the system theme has changed.
    if (Utils::isThemeChangeEvent(event)) {
        // Sometimes the FramelessManager instance may be destroyed already.
        if (FramelessManager * const manager = FramelessManager::instance()) {
            if (FramelessManagerPrivate * const managerPriv = FramelessManagerPrivate::get(manager)) {
                managerPriv->notifySystemThemeHasChangedOrNot();
            }
        }
        return QObject::eventFilter(object, event);
    }
#endif // (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    // We are only interested in events that are dispatched to top level windows.
    if (!object->isWindowType()) {
        return QObject::eventFilter(object, event);
    }
    const QEvent::Type type = event->type();
    // We are only interested in some specific mouse events.
    if ((type != QEvent::MouseButtonPress) && (type != QEvent::MouseButtonRelease)
        && (type != QEvent::MouseButtonDblClick) && (type != QEvent::MouseMove)) {
        return QObject::eventFilter(object, event);
    }
    const auto window = qobject_cast<QWindow *>(object);
    const WId windowId = window->winId();
    g_qtHelper()->mutex.lock();
    if (!g_qtHelper()->data.contains(windowId)) {
        g_qtHelper()->mutex.unlock();
        return QObject::eventFilter(object, event);
    }
    const QtHelperData data = g_qtHelper()->data.value(windowId);
    g_qtHelper()->mutex.unlock();
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
    const Qt::MouseButton button = mouseEvent->button();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = mouseEvent->scenePosition().toPoint();
    const QPoint globalPos = mouseEvent->globalPosition().toPoint();
#else
    const QPoint scenePos = mouseEvent->windowPos().toPoint();
    const QPoint globalPos = mouseEvent->screenPos().toPoint();
#endif
    const bool windowFixedSize = data.params.isWindowFixedSize();
    const bool ignoreThisEvent = data.params.shouldIgnoreMouseEvents(scenePos);
    const bool insideTitleBar = data.params.isInsideTitleBarDraggableArea(scenePos);
    const bool dontOverrideCursor = data.params.getProperty(kDontOverrideCursorVar, false).toBool();
    const bool dontToggleMaximize = data.params.getProperty(kDontToggleMaximizeVar, false).toBool();
    switch (type) {
    case QEvent::MouseButtonPress: {
        if (button == Qt::LeftButton) {
            g_qtHelper()->mutex.lock();
            g_qtHelper()->data[windowId].leftButtonPressed = true;
            g_qtHelper()->mutex.unlock();
            if (!windowFixedSize) {
                const Qt::Edges edges = Utils::calculateWindowEdges(window, scenePos);
                if (edges != Qt::Edges{}) {
                    Utils::startSystemResize(window, edges, globalPos);
                    event->accept();
                    return true;
                }
            }
        }
    } break;
    case QEvent::MouseButtonRelease: {
        if (button == Qt::LeftButton) {
            const QMutexLocker locker(&g_qtHelper()->mutex);
            g_qtHelper()->data[windowId].leftButtonPressed = false;
        }
        if (button == Qt::RightButton) {
            if (!ignoreThisEvent && insideTitleBar) {
                data.params.showSystemMenu(scenePos);
                event->accept();
                return true;
            }
        }
    } break;
    case QEvent::MouseButtonDblClick: {
        if (!dontToggleMaximize && (button == Qt::LeftButton) && !windowFixedSize && !ignoreThisEvent && insideTitleBar) {
            Qt::WindowState newWindowState = Qt::WindowNoState;
            if (data.params.getWindowState() != Qt::WindowMaximized) {
                newWindowState = Qt::WindowMaximized;
            }
            data.params.setWindowState(newWindowState);
            event->accept();
            return true;
        }
    } break;
    case QEvent::MouseMove: {
        if (!dontOverrideCursor && !windowFixedSize) {
            const Qt::CursorShape cs = Utils::calculateCursorShape(window, scenePos);
            if (cs == Qt::ArrowCursor) {
                if (data.cursorShapeChanged) {
                    data.params.unsetCursor();
                    const QMutexLocker locker(&g_qtHelper()->mutex);
                    g_qtHelper()->data[windowId].cursorShapeChanged = false;
                }
            } else {
                data.params.setCursor(cs);
                const QMutexLocker locker(&g_qtHelper()->mutex);
                g_qtHelper()->data[windowId].cursorShapeChanged = true;
            }
        }
        if (data.leftButtonPressed) {
            if (!ignoreThisEvent && insideTitleBar) {
                Utils::startSystemMove(window, globalPos);
                event->accept();
                return true;
            }
        }
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

FRAMELESSHELPER_END_NAMESPACE
