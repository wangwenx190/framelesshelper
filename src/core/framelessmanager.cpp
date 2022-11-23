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

#include "framelessmanager_p.h"
#include <QtCore/qmutex.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/qfontdatabase.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#  include <QtGui/qguiapplication.h>
#  include <QtGui/qstylehints.h>
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#include "framelesshelper_qt.h"
#include "framelessconfig_p.h"
#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "framelesshelper_win.h"
#  include "winverhelper_p.h"
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

Q_LOGGING_CATEGORY(lcFramelessManager, "wangwenx190.framelesshelper.core.framelessmanager")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessManager)
#  define DEBUG qCDebug(lcFramelessManager)
#  define WARNING qCWarning(lcFramelessManager)
#  define CRITICAL qCCritical(lcFramelessManager)
#endif

using namespace Global;

struct FramelessManagerHelper
{
    QMutex mutex;
    QList<WId> windowIds = {};
};

Q_GLOBAL_STATIC(FramelessManagerHelper, g_helper)

Q_GLOBAL_STATIC(FramelessManager, g_manager)

[[maybe_unused]] static constexpr const char kGlobalFlagVarName[] = "__FRAMELESSHELPER__";
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFilePath, ":/org.wangwenx190.FramelessHelper/resources/fonts/Micon.ttf")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_win11, "Segoe Fluent Icons")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_win10, "Segoe MDL2 Assets")
FRAMELESSHELPER_STRING_CONSTANT2(IconFontFamilyName_common, "micon_nb")
#ifdef Q_OS_MACOS
  [[maybe_unused]] static constexpr const int kIconFontPointSize = 10;
#else
  [[maybe_unused]] static constexpr const int kIconFontPointSize = 8;
#endif

[[nodiscard]] static inline QString iconFontFamilyName()
{
    static const auto result = []() -> QString {
#ifdef Q_OS_WINDOWS
        if (WindowsVersionHelper::isWin11OrGreater()) {
            return kIconFontFamilyName_win11;
        }
        if (WindowsVersionHelper::isWin10OrGreater()) {
            return kIconFontFamilyName_win10;
        }
#endif
        return kIconFontFamilyName_common;
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
    // We always register this font because it's our only fallback.
    const int id = QFontDatabase::addApplicationFont(kIconFontFilePath);
    if (id < 0) {
        WARNING << "Failed to load icon font:" << kIconFontFilePath;
    } else {
        DEBUG << "Successfully registered icon font:" << QFontDatabase::applicationFontFamilies(id);
    }
}

QFont FramelessManagerPrivate::getIconFont()
{
    static const auto font = []() -> QFont {
        QFont f = {};
        f.setFamily(iconFontFamilyName());
        f.setPointSize(kIconFontPointSize);
        return f;
    }();
    return font;
}

SystemTheme FramelessManagerPrivate::systemTheme() const
{
    const QMutexLocker locker(&g_helper()->mutex);
    return m_systemTheme;
}

QColor FramelessManagerPrivate::systemAccentColor() const
{
    const QMutexLocker locker(&g_helper()->mutex);
    return m_accentColor;
}

QString FramelessManagerPrivate::wallpaper() const
{
    const QMutexLocker locker(&g_helper()->mutex);
    return m_wallpaper;
}

WallpaperAspectStyle FramelessManagerPrivate::wallpaperAspectStyle() const
{
    const QMutexLocker locker(&g_helper()->mutex);
    return m_wallpaperAspectStyle;
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
    static const bool pureQt = usePureQtImplementation();
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

void FramelessManagerPrivate::removeWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return;
    }
    g_helper()->mutex.lock();
    if (!g_helper()->windowIds.contains(windowId)) {
        g_helper()->mutex.unlock();
        return;
    }
    g_helper()->windowIds.removeAll(windowId);
    g_helper()->mutex.unlock();
    static const bool pureQt = usePureQtImplementation();
    if (pureQt) {
        FramelessHelperQt::removeWindow(windowId);
    }
#ifdef Q_OS_WINDOWS
    if (!pureQt) {
        FramelessHelperWin::removeWindow(windowId);
    }
    Utils::uninstallSystemMenuHook(windowId);
#endif
}

void FramelessManagerPrivate::notifySystemThemeHasChangedOrNot()
{
    const QMutexLocker locker(&g_helper()->mutex);
    const SystemTheme currentSystemTheme = Utils::getSystemTheme();
#ifdef Q_OS_WINDOWS
    const DwmColorizationArea currentColorizationArea = Utils::getDwmColorizationArea();
    const QColor currentAccentColor = Utils::getDwmAccentColor();
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
        Q_Q(FramelessManager);
        Q_EMIT q->systemThemeChanged();
        DEBUG.nospace() << "System theme changed. Current theme: " << m_systemTheme
                        << ", accent color: " << m_accentColor.name(QColor::HexArgb).toUpper()
#ifdef Q_OS_WINDOWS
                        << ", colorization area: " << m_colorizationArea
#endif
                        << '.';
    }
}

void FramelessManagerPrivate::notifyWallpaperHasChangedOrNot()
{
    const QMutexLocker locker(&g_helper()->mutex);
    const QString currentWallpaper = Utils::getWallpaperFilePath();
    const WallpaperAspectStyle currentWallpaperAspectStyle = Utils::getWallpaperAspectStyle();
    bool notify = false;
    if (m_wallpaper != currentWallpaper) {
        m_wallpaper = currentWallpaper;
        notify = true;
    }
    if (m_wallpaperAspectStyle != currentWallpaperAspectStyle) {
        m_wallpaperAspectStyle = currentWallpaperAspectStyle;
        notify = true;
    }
    if (notify) {
        Q_Q(FramelessManager);
        Q_EMIT q->wallpaperChanged();
        DEBUG.nospace() << "Wallpaper changed. Current wallpaper: " << m_wallpaper
                        << ", aspect style: " << m_wallpaperAspectStyle << '.';
    }
}

bool FramelessManagerPrivate::usePureQtImplementation()
{
    static const auto result = []() -> bool {
#ifdef Q_OS_WINDOWS
        return FramelessConfig::instance()->isSet(Option::UseCrossPlatformQtImplementation);
#else
        return true;
#endif
    }();
    return result;
}

void FramelessManagerPrivate::initialize()
{
    const QMutexLocker locker(&g_helper()->mutex);
    m_systemTheme = Utils::getSystemTheme();
#ifdef Q_OS_WINDOWS
    m_colorizationArea = Utils::getDwmColorizationArea();
    m_accentColor = Utils::getDwmAccentColor();
#endif
#ifdef Q_OS_LINUX
    m_accentColor = Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
    m_accentColor = Utils::getControlsAccentColor();
#endif
    m_wallpaper = Utils::getWallpaperFilePath();
    m_wallpaperAspectStyle = Utils::getWallpaperAspectStyle();
    DEBUG.nospace() << "Current system theme: " << m_systemTheme
                    << ", accent color: " << m_accentColor.name(QColor::HexArgb).toUpper()
#ifdef Q_OS_WINDOWS
                    << ", colorization area: " << m_colorizationArea
#endif
                    << ", wallpaper: " << m_wallpaper
                    << ", aspect style: " << m_wallpaperAspectStyle
                    << '.';
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    QStyleHints * const styleHints = QGuiApplication::styleHints();
    Q_ASSERT(styleHints);
    if (styleHints) {
        connect(styleHints, &QStyleHints::appearanceChanged, this, [this](const Qt::Appearance appearance){
            Q_UNUSED(appearance);
            notifySystemThemeHasChangedOrNot();
        });
    }
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    static bool flagSet = false;
    if (!flagSet) {
        flagSet = true;
        // Set a global flag so that people can check whether FramelessHelper is being
        // used without actually accessing the FramelessHelper interface.
        const int ver = FramelessHelper::Core::version().version;
        qputenv(kGlobalFlagVarName, QByteArray::number(ver));
        qApp->setProperty(kGlobalFlagVarName, ver);
    }
}

FramelessManager::FramelessManager(QObject *parent) :
    QObject(parent), d_ptr(new FramelessManagerPrivate(this))
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

QString FramelessManager::wallpaper() const
{
    Q_D(const FramelessManager);
    return d->wallpaper();
}

WallpaperAspectStyle FramelessManager::wallpaperAspectStyle() const
{
    Q_D(const FramelessManager);
    return d->wallpaperAspectStyle();
}

void FramelessManager::addWindow(const SystemParameters &params)
{
    Q_D(FramelessManager);
    d->addWindow(params);
}

void FramelessManager::removeWindow(const WId windowId)
{
    Q_D(FramelessManager);
    d->removeWindow(windowId);
}

FRAMELESSHELPER_END_NAMESPACE
