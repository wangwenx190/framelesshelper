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
#include <QtCore/qvariant.h>
#include <QtCore/qmutex.h>
#include <QtCore/qsettings.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include "framelesshelper_qt.h"
#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win.h"
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct FramelessWindowsManagerHelper
{
    QMutex mutex = {};
    QList<WId> windowIds = {};
};

Q_GLOBAL_STATIC(FramelessWindowsManagerHelper, g_helper)

Q_GLOBAL_STATIC(FramelessWindowsManager, g_manager)

FramelessWindowsManager::FramelessWindowsManager(QObject *parent) : QObject(parent)
{
}

FramelessWindowsManager::~FramelessWindowsManager() = default;

FramelessWindowsManager *FramelessWindowsManager::instance()
{
    return g_manager();
}

bool FramelessWindowsManager::usePureQtImplementation()
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

SystemTheme FramelessWindowsManager::systemTheme()
{
#ifdef Q_OS_WINDOWS
    return Utils::getSystemTheme();
#else
    return SystemTheme::Unknown;
#endif
}

void FramelessWindowsManager::addWindow(const UserSettings &settings, const SystemParameters &params)
{
    Q_ASSERT(params.isValid());
    if (!params.isValid()) {
        return;
    }
    g_helper()->mutex.lock();
    if (g_helper()->windowIds.contains(params.windowId)) {
        g_helper()->mutex.unlock();
        return;
    }
    g_helper()->windowIds.append(params.windowId);
    g_helper()->mutex.unlock();
    static const bool pureQt = usePureQtImplementation();
    QWindow *window = params.getWindowHandle();
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        // Work-around Win32 multi-monitor artifacts.
        connect(window, &QWindow::screenChanged, window, [&params, window](QScreen *screen){
            Q_UNUSED(screen);
            // Force a WM_NCCALCSIZE event to inform Windows about our custom window frame,
            // this is only necessary when the window is being moved cross monitors.
            Utils::triggerFrameChange(params.windowId);
            // For some reason the window is not repainted correctly when moving cross monitors,
            // we workaround this issue by force a re-paint and re-layout of the window by triggering
            // a resize event manually. Although the actual size does not change, the issue we
            // observed disappeared indeed, amazingly.
            window->resize(window->size());
        });
    }
#endif
    if (pureQt) {
        FramelessHelperQt::addWindow(settings, params);
    }
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        FramelessHelperWin::addWindow(settings, params);
    }
    if (!(settings.options & Option::DontInstallSystemMenuHook)) {
        Utils::installSystemMenuHook(params.windowId, settings.options,
                                     settings.systemMenuOffset, params.isWindowFixedSize);
    }
#endif
}

void FramelessHelper::Core::initialize(const Options options)
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
#ifdef Q_OS_WINDOWS
    if (!(options & Option::DontTouchProcessDpiAwarenessLevel)) {
        // This is equivalent to set the "dpiAware" and "dpiAwareness" field in
        // your manifest file. It works through out Windows Vista to Windows 11.
        // It's highly recommended to enable the highest DPI awareness level
        // (currently it's PerMonitor Version 2, or PMv2 for short) for any GUI
        // applications, to allow your user interface scale to an appropriate
        // size and still stay sharp, though you will have to do the calculation
        // and resize by yourself.
        Utils::tryToEnableHighestDpiAwarenessLevel();
    }
#endif
    if (!(options & Option::DontEnsureNonNativeWidgetSiblings)) {
        // This attribute is known to be __NOT__ compatible with QGLWidget.
        // Please consider migrating to the recommended QOpenGLWidget instead.
        QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (!(options & Option::DontTouchHighDpiScalingPolicy)) {
        // Enable high DPI scaling by default, but only for Qt5 applications,
        // because this has become the default setting since Qt6 and it can't
        // be changed from outside anymore (except for internal testing purposes).
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if (!(options & Option::DontTouchScaleFactorRoundingPolicy)) {
        // Non-integer scale factors will cause Qt have some painting defects
        // for both Qt Widgets and Qt Quick applications, and it's still not
        // totally fixed till now (Qt 6.4), so we round the scale factors to
        // get a better looking. Non-integer scale factors will also cause
        // flicker and jitter during window resizing.
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
    }
#endif
    // Mainly for Qt Quick applications, but won't bring any harm to Qt Widgets
    // applications either.
    qRegisterMetaType<Option>();
    qRegisterMetaType<SystemTheme>();
    qRegisterMetaType<SystemButtonType>();
    qRegisterMetaType<ResourceType>();
    qRegisterMetaType<DwmColorizationArea>();
    qRegisterMetaType<Anchor>();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Only needed by Qt5 Quick applications, it's hard to say whether it's a
    // bug or a lack of features. The QML engine is having a hard time to find
    // the correct type if the type has a long namespace with a deep hierarchy.
    qRegisterMetaType<Anchor>("Global::Anchor");
#endif
    qRegisterMetaType<UserSettings>();
    qRegisterMetaType<SystemParameters>();
}

FRAMELESSHELPER_END_NAMESPACE
