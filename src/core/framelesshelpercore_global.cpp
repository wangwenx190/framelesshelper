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

#include "framelesshelpercore_global.h"
#include "framelesshelpercore_global_p.h"
#include "versionnumber_p.h"
#include "utils.h"
#include <QtCore/qiodevice.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>

#ifndef COMPILER_STRING
#  ifdef Q_CC_CLANG // Must be before GNU, because Clang claims to be GNU too.
#    define COMPILER_STRING __VERSION__ // Already includes the compiler's name.
#  elif defined(Q_CC_GHS)
#    define COMPILER_STRING "GHS " QT_STRINGIFY(__GHS_VERSION_NUMBER)
#  elif defined(Q_CC_GNU)
#    define COMPILER_STRING "GCC " __VERSION__
#  elif defined(Q_CC_MSVC)
#    if (_MSC_VER < 1910)
#      define COMPILER_STRING "MSVC 2015"
#    elif (_MSC_VER < 1917)
#      define COMPILER_STRING "MSVC 2017"
#    elif (_MSC_VER < 1930)
#      define COMPILER_STRING "MSVC 2019"
#    elif (_MSC_VER < 2000)
#      define COMPILER_STRING "MSVC 2022"
#    else
#      define COMPILER_STRING "MSVC version " QT_STRINGIFY(_MSC_VER)
#    endif
#  else
#    define COMPILER_STRING "UNKNOWN"
#  endif
#endif

#ifndef QT_NO_DEBUG_STREAM
QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug d, const FRAMELESSHELPER_PREPEND_NAMESPACE(VersionNumber) &ver)
{
    const QDebugStateSaver saver(d);
    d.nospace().noquote() << "VersionNumber("
                          << ver.Major << ", "
                          << ver.Minor << ", "
                          << ver.Patch << ", "
                          << ver.Tweak << ')';
    return d;
}

QDebug operator<<(QDebug d, const FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::VersionInfo &ver)
{
    const QDebugStateSaver saver(d);
    int major = 0, minor = 0, patch = 0;
    FRAMELESSHELPER_EXTRACT_VERSION(ver.version, major, minor, patch)
    const auto ver_num = FRAMELESSHELPER_PREPEND_NAMESPACE(VersionNumber){major, minor, patch};
    d.nospace().noquote() << "VersionInfo("
                          << "version number: " << ver_num << ", "
                          << "version string: " << ver.version_str << ", "
                          << "commit hash: " << ver.commit << ", "
                          << "compiler: " << ver.compiler << ", "
                          << "debug build: " << ver.isDebug << ", "
                          << "static build: " << ver.isStatic << ')';
    return d;
}

QDebug operator<<(QDebug d, const FRAMELESSHELPER_PREPEND_NAMESPACE(Global)::Dpi &dpi)
{
    const QDebugStateSaver saver(d);
    const qreal scaleFactor = (qreal(dpi.x) / qreal(FRAMELESSHELPER_PREPEND_NAMESPACE(Utils)::defaultScreenDpi()));
    d.nospace().noquote() << "Dpi("
                          << "x: " << dpi.x << ", "
                          << "y: " << dpi.y << ", "
                          << "scale factor: " << scaleFactor << ')';
    return d;
}
QT_END_NAMESPACE
#endif // QT_NO_DEBUG_STREAM

#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
// The "Q_INIT_RESOURCE()" macro can't be used within a namespace,
// so we wrap it into a separate function outside of the namespace and
// then call it instead inside the namespace, that's also the recommended
// workaround provided by Qt's official documentation.
void framelesshelpercore_initResource()
{
    Q_INIT_RESOURCE(framelesshelpercore);
}
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE

FRAMELESSHELPER_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lcCoreGlobal, "wangwenx190.framelesshelper.core.global")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcCoreGlobal)
#  define DEBUG qCDebug(lcCoreGlobal)
#  define WARNING qCWarning(lcCoreGlobal)
#  define CRITICAL qCCritical(lcCoreGlobal)
#endif

using namespace Global;

#ifdef Q_OS_WINDOWS
static_assert(std::size(WindowsVersions) == (static_cast<int>(WindowsVersion::Latest) + 1));
#endif

#ifdef Q_OS_LINUX
[[maybe_unused]] static constexpr const char QT_QPA_ENV_VAR[] = "QT_QPA_PLATFORM";
FRAMELESSHELPER_BYTEARRAY_CONSTANT(xcb)
#endif

#ifdef Q_OS_MACOS
[[maybe_unused]] static constexpr const char MAC_LAYER_ENV_VAR[] = "QT_MAC_WANTS_LAYER";
#endif

[[maybe_unused]] static constexpr const char kNoLogoEnvVar[] = "FRAMELESSHELPER_NO_LOGO";

void registerInitializeHook(const InitializeHookCallback &cb)
{
    Q_UNUSED(cb);
    WARNING << "registerInitializeHook: This function is deprecated and will be removed in a future version. Please consider using Qt's official Q_COREAPP_STARTUP_FUNCTION() macro instead.";
}

void registerUninitializeHook(const UninitializeHookCallback &cb)
{
    Q_UNUSED(cb);
    WARNING << "registerUninitializeHook: This function is deprecated and will be removed in a future version. Please consider using Qt's official qAddPostRoutine() function instead.";
}

namespace FramelessHelper::Core
{

void initialize()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;

    outputLogo();

#ifdef Q_OS_LINUX
    // Qt's Wayland experience is not good, so we force the XCB backend here.
    // TODO: Remove this hack once Qt's Wayland implementation is good enough.
    // We are setting the preferred QPA backend, so we have to set it early
    // enough, that is, before the construction of any Q(Gui)Application
    // instances. QCoreApplication won't instantiate the platform plugin.
    qputenv(QT_QPA_ENV_VAR, kxcb);
    // Fedora and Arch users report segfault when calling XInitThreads() and gtk_init().
    //XInitThreads(); // Users report that GTK is crashing without this.
    //gtk_init(nullptr, nullptr); // Users report that GTK functionalities won't work without this.
#endif

#if (defined(Q_OS_MACOS) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
    qputenv(MAC_LAYER_ENV_VAR, FRAMELESSHELPER_BYTEARRAY_LITERAL("1"));
#endif

#ifdef Q_OS_WINDOWS
    // This is equivalent to set the "dpiAware" and "dpiAwareness" field in
    // your manifest file. It works through out Windows Vista to Windows 11.
    // It's highly recommended to enable the highest DPI awareness mode
    // (currently it's PerMonitor Version 2, or PMv2 for short) for any GUI
    // applications, to allow your user interface scale to an appropriate
    // size and still stay sharp, though you will have to do the calculation
    // and resize by yourself.
    Utils::tryToEnableHighestDpiAwarenessLevel();
    // This function need to be called before any dialogs are created, so
    // to be safe we call it here.
    // Without this hack, our native dialogs won't be able to respond to
    // DPI change messages correctly, especially the non-client area.
    Utils::fixupDialogsDpiScaling();
#endif

    // We need this flag to enable nested frameless windows, however,
    // this flag is known to be __NOT__ compatible with QGLWidget.
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Enable high DPI scaling by default, but only for Qt5 applications,
    // because this has become the default setting since Qt6 and it can't
    // be changed from outside anymore (except for internal testing purposes).
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

void uninitialize()
{
    static bool uninited = false;
    if (uninited) {
        return;
    }
    uninited = true;
}

VersionInfo version()
{
    static const auto result = []() -> VersionInfo {
        const auto _compiler = []() -> const char * { return COMPILER_STRING; }();
        const auto _debug = []() -> bool {
#ifdef _DEBUG
            return true;
#else
            return false;
#endif
        }();
        const auto _static = []() -> bool {
#ifdef FRAMELESSHELPER_CORE_STATIC
            return true;
#else
            return false;
#endif
        }();
        return VersionInfo{
            /* .version */ FRAMELESSHELPER_VERSION,
            /* .version_str */ FRAMELESSHELPER_VERSION_STR,
            /* .commit */ FRAMELESSHELPER_COMMIT_STR,
            /* .compileDateTime */ FRAMELESSHELPER_COMPILE_DATETIME_STR,
            /* .compiler */ _compiler,
            /* .isDebug */ _debug,
            /* .isStatic */ _static
        };
    }();
    return result;
}

void setApplicationOSThemeAware()
{
    static bool set = false;
    if (set) {
        return;
    }
    set = true;

#ifdef Q_OS_WINDOWS
    // This hack is needed to let AllowDarkModeForWindow() work.
    Utils::setDarkModeAllowedForApp(true);
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    // Qt gained the ability to detect system theme change since 5.15 but
    // it's not quite useful until Qt6.
    Utils::setQtDarkModeAwareEnabled(true);
#  endif
#endif

#if ((defined(Q_OS_LINUX) && (QT_VERSION < QT_VERSION_CHECK(6, 4, 0))) || \
    (defined(Q_OS_MACOS) && (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))))
    // Linux: Qt 6.4 gained the ability to detect system theme change.
    // macOS: Qt 5.12.
    Utils::registerThemeChangeNotification();
#endif
}

void outputLogo()
{
    if (qEnvironmentVariableIntValue(kNoLogoEnvVar)) {
        return;
    }
    const VersionInfo &ver = version();
    QString message = {};
    QTextStream stream(&message, QIODevice::WriteOnly);
    stream << "FramelessHelper (" << (ver.isStatic ? "static" : "shared")
           << ", " << (ver.isDebug ? "debug" : "release") << ") version "
           << ver.version_str << ", author wangwenx190 (Yuhang Zhao)."
           << " Built by " << ver.compiler << " from " << ver.commit
           << " on " << ver.compileDateTime << " (UTC).";
    INFO.nospace().noquote() << message;
}

}

FRAMELESSHELPER_END_NAMESPACE
