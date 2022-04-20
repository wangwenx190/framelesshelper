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

#ifdef Q_OS_LINUX
static constexpr const char QT_QPA_ENV_VAR[] = "QT_QPA_PLATFORM";
FRAMELESSHELPER_BYTEARRAY_CONSTANT(xcb)
#endif

#ifdef Q_OS_MACOS
static constexpr const char MAC_LAYER_ENV_VAR[] = "QT_MAC_WANTS_LAYER";
FRAMELESSHELPER_BYTEARRAY_CONSTANT2(ValueOne, "1")
#endif

FramelessWindowsManagerPrivate::FramelessWindowsManagerPrivate(FramelessWindowsManager *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessWindowsManagerPrivate::~FramelessWindowsManagerPrivate() = default;

FramelessWindowsManagerPrivate *FramelessWindowsManagerPrivate::get(FramelessWindowsManager *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessWindowsManagerPrivate *FramelessWindowsManagerPrivate::get(const FramelessWindowsManager *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

bool FramelessWindowsManagerPrivate::usePureQtImplementation()
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

SystemTheme FramelessWindowsManagerPrivate::systemTheme() const
{
    return m_systemTheme;
}

QColor FramelessWindowsManagerPrivate::systemAccentColor() const
{
    return m_accentColor;
}

void FramelessWindowsManagerPrivate::addWindow(const UserSettings &settings, const SystemParameters &params)
{
    Q_ASSERT(params.isValid());
    if (!params.isValid()) {
        return;
    }
    const WId windowId = params.getWindowId();
    g_helper()->mutex.lock();
    if (g_helper()->windowIds.contains(windowId)) {
        g_helper()->mutex.unlock();
        return;
    }
    g_helper()->windowIds.append(windowId);
    g_helper()->mutex.unlock();
    static const bool pureQt = usePureQtImplementation();
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        // Work-around Win32 multi-monitor artifacts.
        QWindow * const window = params.getWindowHandle();
        Q_ASSERT(window);
        connect(window, &QWindow::screenChanged, window, [windowId, window](QScreen *screen){
            Q_UNUSED(screen);
            // Force a WM_NCCALCSIZE event to inform Windows about our custom window frame,
            // this is only necessary when the window is being moved cross monitors.
            Utils::triggerFrameChange(windowId);
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
        Utils::installSystemMenuHook(windowId, settings.options, settings.systemMenuOffset, params.isWindowFixedSize);
    }
#endif
}

void FramelessWindowsManagerPrivate::notifySystemThemeHasChangedOrNot()
{
    Q_Q(FramelessWindowsManager);
    const SystemTheme currentSystemTheme = Utils::getSystemTheme();
#ifdef Q_OS_WINDOWS
    const DwmColorizationArea currentColorizationArea = Utils::getDwmColorizationArea();
    const QColor currentAccentColor = Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
    const QColor currentAccentColor = Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
    const QColor currentAccentColor = Utils::getControlsAccentColor();
#endif
    bool notify = false;
    if (m_systemTheme != currentSystemTheme) {
        m_systemTheme = currentSystemTheme;
        notify = true;
    }
    if (m_accentColor != currentAccentColor) {
        m_accentColor = currentAccentColor;
        notify = true;
    }
#ifdef Q_OS_WINDOWS
    if (m_colorizationArea != currentColorizationArea) {
        m_colorizationArea = currentColorizationArea;
        notify = true;
    }
#endif
    if (notify) {
        Q_EMIT q->systemThemeChanged();
    }
}

void FramelessWindowsManagerPrivate::initialize()
{
    m_systemTheme = Utils::getSystemTheme();
#ifdef Q_OS_WINDOWS
    m_colorizationArea = Utils::getDwmColorizationArea();
    m_accentColor = Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_MACOS
    m_accentColor = Utils::getControlsAccentColor();
#endif
}

FramelessWindowsManager::FramelessWindowsManager(QObject *parent) : QObject(parent), d_ptr(new FramelessWindowsManagerPrivate(this))
{
}

FramelessWindowsManager::~FramelessWindowsManager() = default;

FramelessWindowsManager *FramelessWindowsManager::instance()
{
    return g_manager();
}

bool FramelessWindowsManager::usePureQtImplementation() const
{
    Q_D(const FramelessWindowsManager);
    return d->usePureQtImplementation();
}

SystemTheme FramelessWindowsManager::systemTheme() const
{
    Q_D(const FramelessWindowsManager);
    return d->systemTheme();
}

QColor FramelessWindowsManager::systemAccentColor() const
{
    Q_D(const FramelessWindowsManager);
    return d->systemAccentColor();
}

void FramelessWindowsManager::addWindow(const UserSettings &settings, const SystemParameters &params)
{
    Q_D(FramelessWindowsManager);
    d->addWindow(settings, params);
}

void FramelessHelper::Core::initialize(const Options options)
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
#ifdef Q_OS_LINUX
    // Qt's Wayland experience is not good, so we force the X11 backend here.
    // TODO: Remove this hack once Qt's Wayland implementation is good enough.
    // We are setting the preferred QPA backend, so we have to set it early
    // enough, that is, before the construction of any Q(Gui)Application
    // instances. QCoreApplication won't instantiate the platform plugin.
    qputenv(QT_QPA_ENV_VAR, kxcb);
#endif
#ifdef Q_OS_MACOS
    // This has become the default setting since some unknown Qt version,
    // check whether we can remove this hack safely or not.
    qputenv(MAC_LAYER_ENV_VAR, kValueOne);
#endif
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
    if (!(options & Option::DontEnsureNonNativeWidgetSiblings)) {
        // This attribute is known to be __NOT__ compatible with QGLWidget.
        // Please consider migrating to the recommended QOpenGLWidget instead.
        QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
#endif
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
    qRegisterMetaType<ButtonState>();
    qRegisterMetaType<UserSettings>();
    qRegisterMetaType<SystemParameters>();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Only needed by Qt5 Quick applications, it's hard to say whether it's a
    // bug or a lack of features. The QML engine is having a hard time to find
    // the correct type if the type has a long namespace with a deep hierarchy.
    qRegisterMetaType<Option>("Global::Option");
    qRegisterMetaType<SystemTheme>("Global::SystemTheme");
    qRegisterMetaType<SystemButtonType>("Global::SystemButtonType");
    qRegisterMetaType<ResourceType>("Global::ResourceType");
    qRegisterMetaType<DwmColorizationArea>("Global::DwmColorizationArea");
    qRegisterMetaType<Anchor>("Global::Anchor");
    qRegisterMetaType<ButtonState>("Global::ButtonState");
    qRegisterMetaType<UserSettings>("Global::UserSettings");
    qRegisterMetaType<SystemParameters>("Global::SystemParameters");
#endif
}

FRAMELESSHELPER_END_NAMESPACE
