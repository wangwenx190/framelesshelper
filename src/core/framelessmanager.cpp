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

#include "framelessmanager.h"
#include "framelessmanager_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include <QtGui/qfontdatabase.h>
#include "framelesshelper_qt.h"
#include "framelessconfig_p.h"
#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win.h"
#endif

// The "Q_INIT_RESOURCE()" macro can't be used within a namespace,
// so we wrap it into a separate function outside of the namespace and
// then call it instead inside the namespace, that's also the recommended
// workaround provided by Qt's official documentation.
static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelpercore);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

struct FramelessManagerHelper
{
    QMutex mutex;
    QList<WId> windowIds = {};
};

Q_GLOBAL_STATIC(FramelessManagerHelper, g_helper)

Q_GLOBAL_STATIC(FramelessManager, g_manager)

FRAMELESSHELPER_STRING_CONSTANT2(IconFontResourcePrefix, ":/org.wangwenx190.FramelessHelper/resources/fonts/")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFileName_win11, "Segoe Fluent Icons.ttf")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFileName_win, "Segoe MDL2 Assets.ttf")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFileName_unix, "Micon.ttf")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_win11, "Segoe Fluent Icons")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_win, "Segoe MDL2 Assets")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_unix, "micon_nb")
static constexpr const int kIconFontPointSize = 8;

[[nodiscard]] static inline QString iconFontFilePath()
{
    static const QString result = []() -> QString {
#ifdef Q_OS_WINDOWS
        static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
        return (kIconFontResourcePrefix + (isWin11OrGreater ? kIconFontFileName_win11 : kIconFontFileName_win));
#else
        return (kIconFontResourcePrefix + kIconFontFileName_unix);
#endif
    }();
    return result;
}

[[nodiscard]] static inline QString iconFontFamilyName()
{
    static const QString result = []() -> QString {
#ifdef Q_OS_WINDOWS
        static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
        return (isWin11OrGreater ? kIconFontFamilyName_win11 : kIconFontFamilyName_win);
#else
        return kIconFontFamilyName_unix;
#endif
    }();
    return result;
}

FramelessManagerPrivate::FramelessManagerPrivate(FramelessManager *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessManagerPrivate::~FramelessManagerPrivate() = default;

FramelessManagerPrivate *FramelessManagerPrivate::get(FramelessManager *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessManagerPrivate *FramelessManagerPrivate::get(const FramelessManager *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void FramelessManagerPrivate::initializeIconFont()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
    initResource();
    const int id = QFontDatabase::addApplicationFont(iconFontFilePath());
    if (id < 0) {
        qWarning() << "Failed to load icon font:" << iconFontFilePath();
    } else {
        qDebug() << "Successfully registered icon font:" << QFontDatabase::applicationFontFamilies(id);
    }
}

QFont FramelessManagerPrivate::getIconFont()
{
    static const QFont font = []() -> QFont {
        QFont f = {};
        f.setFamily(iconFontFamilyName());
        f.setPointSize(kIconFontPointSize);
        return f;
    }();
    return font;
}

SystemTheme FramelessManagerPrivate::systemTheme() const
{
    return m_systemTheme;
}

QColor FramelessManagerPrivate::systemAccentColor() const
{
    return m_accentColor;
}

void FramelessManagerPrivate::addWindow(const SystemParameters &params)
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
    static const bool pureQt = []() -> bool {
#ifdef Q_OS_WINDOWS
        return FramelessConfig::instance()->isSet(Option::UseCrossPlatformQtImplementation);
#else
        return true;
#endif
    }();
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
        FramelessHelperQt::addWindow(params);
    }
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        FramelessHelperWin::addWindow(params);
    }
    Utils::installSystemMenuHook(windowId, params.isWindowFixedSize,
        params.isInsideTitleBarDraggableArea, params.getWindowDevicePixelRatio);
#endif
}

void FramelessManagerPrivate::notifySystemThemeHasChangedOrNot()
{
    Q_Q(FramelessManager);
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

void FramelessManagerPrivate::initialize()
{
    m_systemTheme = Utils::getSystemTheme();
#ifdef Q_OS_WINDOWS
    m_colorizationArea = Utils::getDwmColorizationArea();
    m_accentColor = Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
    m_accentColor = Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
    m_accentColor = Utils::getControlsAccentColor();
#endif
}

FramelessManager::FramelessManager(QObject *parent) : QObject(parent), d_ptr(new FramelessManagerPrivate(this))
{
}

FramelessManager::~FramelessManager() = default;

FramelessManager *FramelessManager::instance()
{
    return g_manager();
}

SystemTheme FramelessManager::systemTheme() const
{
    Q_D(const FramelessManager);
    return d->systemTheme();
}

QColor FramelessManager::systemAccentColor() const
{
    Q_D(const FramelessManager);
    return d->systemAccentColor();
}

void FramelessManager::addWindow(const SystemParameters &params)
{
    Q_D(FramelessManager);
    d->addWindow(params);
}

FRAMELESSHELPER_END_NAMESPACE
