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

#include "framelesswindowsmanager.h"
#include "framelesswindowsmanager_p.h"
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include "framelesshelper_qt.h"
#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win.h"
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

#ifdef Q_OS_WINDOWS
static const bool g_usePureQtImplementation = (qEnvironmentVariableIntValue("FRAMELESSHELPER_PURE_QT_IMPL") != 0);
#else
static constexpr const bool g_usePureQtImplementation = true;
#endif

Q_GLOBAL_STATIC(FramelessManagerPrivate, g_managerPrivate)

FramelessManagerPrivate::FramelessManagerPrivate() = default;

FramelessManagerPrivate::~FramelessManagerPrivate()
{
    QMutexLocker locker(&mutex);
    if (!qtFramelessHelpers.isEmpty()) {
        auto it = qtFramelessHelpers.constBegin();
        while (it != qtFramelessHelpers.constEnd()) {
            const auto helper = it.value();
            if (helper) {
                delete helper;
            }
            ++it;
        }
        qtFramelessHelpers.clear();
    }
}

FramelessManagerPrivate *FramelessManagerPrivate::instance()
{
    return g_managerPrivate();
}

QUuid FramelessManagerPrivate::findIdByWindow(QWindow *value) const
{
    Q_ASSERT(value);
    if (!value) {
        return {};
    }
    QMutexLocker locker(&mutex);
    if (windowMapping.isEmpty()) {
        return {};
    }
    if (!windowMapping.contains(value)) {
        return {};
    }
    return windowMapping.value(value);
}

QUuid FramelessManagerPrivate::findIdByWinId(const WId value) const
{
    Q_ASSERT(value);
    if (!value) {
        return {};
    }
    QMutexLocker locker(&mutex);
    if (winIdMapping.isEmpty()) {
        return {};
    }
    if (!winIdMapping.contains(value)) {
        return {};
    }
    return winIdMapping.value(value);
}

QWindow *FramelessManagerPrivate::findWindowById(const QUuid &value) const
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return nullptr;
    }
    QMutexLocker locker(&mutex);
    if (windowMapping.isEmpty()) {
        return nullptr;
    }
    auto it = windowMapping.constBegin();
    while (it != windowMapping.constEnd()) {
        if (it.value() == value) {
            return it.key();
        }
        ++it;
    }
    return nullptr;
}

WId FramelessManagerPrivate::findWinIdById(const QUuid &value) const
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return 0;
    }
    const QWindow * const window = findWindowById(value);
    return (window ? window->winId() : 0);
}

Q_GLOBAL_STATIC(FramelessWindowsManager, g_managerPublic)

FramelessWindowsManager::FramelessWindowsManager(QObject *parent) : QObject(parent) {}

FramelessWindowsManager::~FramelessWindowsManager() = default;

FramelessWindowsManager *FramelessWindowsManager::instance()
{
    return g_managerPublic();
}

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    g_managerPrivate()->mutex.lock();
    if (g_managerPrivate()->windowMapping.contains(window)) {
        g_managerPrivate()->mutex.unlock();
        return;
    }
    const QUuid uuid = QUuid::createUuid();
    g_managerPrivate()->windowMapping.insert(window, uuid);
    g_managerPrivate()->winIdMapping.insert(window->winId(), uuid);
    FramelessHelperQt *qtFramelessHelper = nullptr;
    if (g_usePureQtImplementation) {
        // Give it a parent so that it can be deleted even if we forget to do.
        qtFramelessHelper = new FramelessHelperQt(window);
        g_managerPrivate()->qtFramelessHelpers.insert(uuid, qtFramelessHelper);
    }
#ifdef Q_OS_WINDOWS
    if (!g_usePureQtImplementation) {
        // Work-around Win32 multi-monitor artifacts.
        const QMetaObject::Connection workaroundConnection =
            connect(window, &QWindow::screenChanged, window, [window](QScreen *screen){
                Q_UNUSED(screen);
                // Force a WM_NCCALCSIZE event to inform Windows about our custom window frame,
                // this is only necessary when the window is being moved cross monitors.
                Utils::triggerFrameChange(window->winId());
                // For some reason the window is not repainted correctly when moving cross monitors,
                // we workaround this issue by force a re-paint and re-layout of the window by triggering
                // a resize event manually. Although the actual size does not change, the issue we
                // observed disappeared indeed, amazingly.
                window->resize(window->size());
            });
        g_managerPrivate()->win32WorkaroundConnections.insert(uuid, workaroundConnection);
    }
#endif
    g_managerPrivate()->mutex.unlock();
    if (g_usePureQtImplementation) {
        Q_ASSERT(qtFramelessHelper);
        if (qtFramelessHelper) {
            qtFramelessHelper->addWindow(window);
        }
    }
#ifdef Q_OS_WINDOWS
    if (!g_usePureQtImplementation) {
        FramelessHelperWin::addWindow(window);
    }
#endif
}

void FramelessWindowsManager::removeWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    QMutexLocker locker(&g_managerPrivate()->mutex);
    if (!g_managerPrivate()->windowMapping.contains(window)) {
        return;
    }
    const QUuid uuid = g_managerPrivate()->windowMapping.value(window);
    Q_ASSERT(!uuid.isNull());
    if (uuid.isNull()) {
        return;
    }
    if (g_managerPrivate()->qtFramelessHelpers.contains(uuid)) {
        const auto helper = g_managerPrivate()->qtFramelessHelpers.value(uuid);
        Q_ASSERT(helper);
        if (helper) {
            helper->removeWindow(window);
            delete helper;
        }
        g_managerPrivate()->qtFramelessHelpers.remove(uuid);
    }
#ifdef Q_OS_WINDOWS
    if (g_managerPrivate()->win32WorkaroundConnections.contains(uuid)) {
        disconnect(g_managerPrivate()->win32WorkaroundConnections.value(uuid));
        g_managerPrivate()->win32WorkaroundConnections.remove(uuid);
    }
#endif
    g_managerPrivate()->windowMapping.remove(window);
    g_managerPrivate()->winIdMapping.remove(window->winId());
}

FRAMELESSHELPER_END_NAMESPACE
