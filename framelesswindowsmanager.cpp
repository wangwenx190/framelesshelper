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
#include <QtGui/qscreen.h>
#include "framelesshelper.h"
#include "utilities.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win32.h"
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

#ifdef Q_OS_WINDOWS
static const bool g_usePureQtImplementation = (qEnvironmentVariableIntValue("FRAMELESSHELPER_PURE_QT_IMPL") != 0);
#else
static constexpr const bool g_usePureQtImplementation = true;
#endif

namespace Private
{

Q_GLOBAL_STATIC(FramelessManager, g_manager)

FramelessManager::FramelessManager() = default;

FramelessManager::~FramelessManager() = default;

FramelessManager *FramelessManager::instance()
{
    return g_manager();
}

}

void FramelessWindowsManager::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    Private::g_manager()->mutex.lock();
    if (Private::g_manager()->qwindow.contains(window)) {
        Private::g_manager()->mutex.unlock();
        return;
    }
    QVariantHash data = {};
    data.insert(kWindow, QVariant::fromValue(window));
    auto qtFramelessHelper = new FramelessHelper(window);
    if (g_usePureQtImplementation) {
        data.insert(kFramelessHelper, QVariant::fromValue(qtFramelessHelper));
    } else {
        delete qtFramelessHelper;
        qtFramelessHelper = nullptr;
    }
    const QUuid uuid = QUuid::createUuid();
    Private::g_manager()->data.insert(uuid, data);
    Private::g_manager()->qwindow.insert(window, uuid);
    Private::g_manager()->winId.insert(window->winId(), uuid);
    Private::g_manager()->mutex.unlock();
#ifdef Q_OS_WINDOWS
    // Work-around Win32 multi-monitor artifacts.
    QObject::connect(window, &QWindow::screenChanged, window, [window](QScreen *screen){
        Q_UNUSED(screen);
        // Force a WM_NCCALCSIZE event to inform Windows about our custom window frame,
        // this is only necessary when the window is being moved cross monitors.
        Utilities::triggerFrameChange(window->winId());
        // For some reason the window is not repainted correctly when moving cross monitors,
        // we workaround this issue by force a re-paint and re-layout of the window by triggering
        // a resize event manually. Although the actual size does not change, the issue we
        // observed disappeared indeed, amazingly.
        window->resize(window->size());
    });
#endif
    if (g_usePureQtImplementation && qtFramelessHelper) {
        qtFramelessHelper->addWindow(window);
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
    QMutexLocker locker(&Private::g_manager()->mutex);
    if (!Private::g_manager()->qwindow.contains(window)) {
        return;
    }
    const QUuid uuid = Private::g_manager()->qwindow.value(window);
    Q_ASSERT(Private::g_manager()->data.contains(uuid));
    if (!Private::g_manager()->data.contains(uuid)) {
        return;
    }
    const QVariantHash data = Private::g_manager()->data.value(uuid);
    if (data.contains(kFramelessHelper)) {
        const auto qtFramelessHelper = qvariant_cast<FramelessHelper *>(data.value(kFramelessHelper));
        Q_ASSERT(qtFramelessHelper);
        if (qtFramelessHelper) {
            qtFramelessHelper->removeWindow(window);
            delete qtFramelessHelper;
        }
    }
    Private::g_manager()->qwindow.remove(window);
    Private::g_manager()->winId.remove(window->winId());
    Private::g_manager()->data.remove(uuid);
}

FRAMELESSHELPER_END_NAMESPACE
