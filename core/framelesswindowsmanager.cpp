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

Q_GLOBAL_STATIC(FramelessWindowsManager, g_manager)

FramelessWindowsManagerPrivate::FramelessWindowsManagerPrivate(FramelessWindowsManager *q)
{
    Q_ASSERT(q);
    if (q) {
        q_ptr = q;
    }
}

FramelessWindowsManagerPrivate::~FramelessWindowsManagerPrivate() = default;

QUuid FramelessWindowsManagerPrivate::findIdByWindow(QWindow *value) const
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

QUuid FramelessWindowsManagerPrivate::findIdByWinId(const WId value) const
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

QWindow *FramelessWindowsManagerPrivate::findWindowById(const QUuid &value) const
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

WId FramelessWindowsManagerPrivate::findWinIdById(const QUuid &value) const
{
    Q_ASSERT(!value.isNull());
    if (value.isNull()) {
        return 0;
    }
    const QWindow * const window = findWindowById(value);
    return (window ? window->winId() : 0);
}

FramelessWindowsManager::FramelessWindowsManager(QObject *parent) : QObject(parent)
{
    d_ptr.reset(new FramelessWindowsManagerPrivate(this));
}

FramelessWindowsManager::~FramelessWindowsManager() = default;

FramelessWindowsManager *FramelessWindowsManager::instance()
{
    return g_manager();
}

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    Q_D(FramelessWindowsManager);
    d->mutex.lock();
    if (d->windowMapping.contains(window)) {
        d->mutex.unlock();
        return;
    }
    const QUuid uuid = QUuid::createUuid();
    d->windowMapping.insert(window, uuid);
    d->winIdMapping.insert(window->winId(), uuid);
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
        d->win32WorkaroundConnections.insert(uuid, workaroundConnection);
    }
#endif
    d->mutex.unlock();
    if (g_usePureQtImplementation) {
        FramelessHelperQt::addWindow(window);
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
    Q_D(FramelessWindowsManager);
    QMutexLocker locker(&d->mutex);
    if (!d->windowMapping.contains(window)) {
        return;
    }
    const QUuid uuid = d->windowMapping.value(window);
    Q_ASSERT(!uuid.isNull());
    if (uuid.isNull()) {
        return;
    }
    if (g_usePureQtImplementation) {
        FramelessHelperQt::removeWindow(window);
    }
#ifdef Q_OS_WINDOWS
    if (!g_usePureQtImplementation) {
        FramelessHelperWin::removeWindow(window);
    }
    if (d->win32WorkaroundConnections.contains(uuid)) {
        disconnect(d->win32WorkaroundConnections.value(uuid));
        d->win32WorkaroundConnections.remove(uuid);
    }
#endif
    d->windowMapping.remove(window);
    d->winIdMapping.remove(window->winId());
}

FRAMELESSHELPER_END_NAMESPACE
