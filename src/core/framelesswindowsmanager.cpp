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
#include <QtCore/qvariant.h>
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include "framelesshelper_qt.h"
#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win.h"
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(FramelessWindowsManager, g_manager)

FramelessWindowsManagerPrivate::FramelessWindowsManagerPrivate(FramelessWindowsManager *q)
{
    Q_ASSERT(q);
    if (q) {
        q_ptr = q;
    }
}

FramelessWindowsManagerPrivate::~FramelessWindowsManagerPrivate() = default;

FramelessWindowsManagerPrivate *FramelessWindowsManagerPrivate::get(FramelessWindowsManager *manager)
{
    Q_ASSERT(manager);
    if (!manager) {
        return nullptr;
    }
    return manager->d_func();
}

const FramelessWindowsManagerPrivate *FramelessWindowsManagerPrivate::get(const FramelessWindowsManager *manager)
{
    Q_ASSERT(manager);
    if (!manager) {
        return nullptr;
    }
    return manager->d_func();
}

bool FramelessWindowsManagerPrivate::usePureQtImplementation() const
{
#ifdef Q_OS_WINDOWS
    static const bool result = []() -> bool {
        if (qEnvironmentVariableIntValue(kUsePureQtImplFlag) != 0) {
            return true;
        }
        const QString iniFilePath = QCoreApplication::applicationDirPath() + u'/' + kConfigFileName;
        QSettings settings(iniFilePath, QSettings::IniFormat);
        return settings.value(kUsePureQtImplKeyPath, false).toBool();
    }();
#else
    static constexpr const bool result = true;
#endif
    return result;
}

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
    if (!QCoreApplication::testAttribute(Qt::AA_DontCreateNativeWidgetSiblings)) {
        QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
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
    const WId winId = window->winId();
    d->winIdMapping.insert(winId, uuid);
    static const bool pureQt = d->usePureQtImplementation();
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        // Work-around Win32 multi-monitor artifacts.
        const QMetaObject::Connection workaroundConnection =
            connect(window, &QWindow::screenChanged, window, [winId, window](QScreen *screen){
                Q_UNUSED(screen);
                // Force a WM_NCCALCSIZE event to inform Windows about our custom window frame,
                // this is only necessary when the window is being moved cross monitors.
                Utils::triggerFrameChange(winId);
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
    const auto options = qvariant_cast<Options>(window->property(kInternalOptionsFlag));
    if (pureQt) {
        FramelessHelperQt::addWindow(window);
    }
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        FramelessHelperWin::addWindow(window);
    }
    if (!(options & Option::DontInstallSystemMenuHook)) {
        Utils::installSystemMenuHook(window);
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
    const WId winId = window->winId();
#ifdef Q_OS_WINDOWS
    if (d->win32WorkaroundConnections.contains(uuid)) {
        disconnect(d->win32WorkaroundConnections.value(uuid));
        d->win32WorkaroundConnections.remove(uuid);
    }
    const auto options = qvariant_cast<Options>(window->property(kInternalOptionsFlag));
    if (!(options & Option::DontInstallSystemMenuHook)) {
        Utils::uninstallSystemMenuHook(winId);
    }
#endif
    static const bool pureQt = d->usePureQtImplementation();
    if (pureQt) {
        FramelessHelperQt::removeWindow(window);
    }
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        FramelessHelperWin::removeWindow(window);
    }
#endif
    d->windowMapping.remove(window);
    d->winIdMapping.remove(winId);
}

FRAMELESSHELPER_END_NAMESPACE
