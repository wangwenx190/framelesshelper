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

#include "framelessconfig_p.h"
#include <QtCore/qmutex.h>
#include <QtCore/qdir.h>
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcFramelessConfig, "wangwenx190.framelesshelper.core.framelessconfig")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessConfig)
#  define DEBUG qCDebug(lcFramelessConfig)
#  define WARNING qCWarning(lcFramelessConfig)
#  define CRITICAL qCCritical(lcFramelessConfig)
#endif

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(ConfigFileName, ".framelesshelper.ini")

static const struct
{
    const QByteArray env = {};
    const QByteArray cfg = {};
} OptionsTable[] = {
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_USE_CROSS_PLATFORM_QT_IMPLEMENTATION"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/UseCrossPlatformQtImplementation")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_FORCE_HIDE_WINDOW_FRAME_BORDER"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/ForceHideWindowFrameBorder")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_FORCE_SHOW_WINDOW_FRAME_BORDER"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/ForceShowWindowFrameBorder")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_DISABLE_WINDOWS_SNAP_LAYOUT"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/DisableWindowsSnapLayout")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_WINDOW_USE_ROUND_CORNERS"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/WindowUseRoundCorners")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_CENTER_WINDOW_BEFORE_SHOW"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/CenterWindowBeforeShow")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_ENABLE_BLUR_BEHIND_WINDOW"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/EnableBlurBehindWindow")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_FORCE_NON_NATIVE_BACKGROUND_BLUR"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/ForceNonNativeBackgroundBlur")},
    {FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_DISABLE_LAZY_INITIALIZATION_FOR_MICA_MATERIAL"),
      FRAMELESSHELPER_BYTEARRAY_LITERAL("Options/DisableLazyInitializationForMicaMaterial")}
};

static constexpr const auto OptionCount = std::size(OptionsTable);

struct ConfigData
{
    QMutex mutex;
    bool loaded = false;
    bool options[OptionCount] = {};
    bool disableEnvVar = false;
    bool disableCfgFile = false;
    QVariantHash internals = {};
};

Q_GLOBAL_STATIC(ConfigData, g_data)

Q_GLOBAL_STATIC(FramelessConfig, g_config)

FramelessConfig::FramelessConfig(QObject *parent) : QObject(parent)
{
    reload();
}

FramelessConfig::~FramelessConfig() = default;

FramelessConfig *FramelessConfig::instance()
{
    return g_config();
}

void FramelessConfig::reload(const bool force)
{
    const QMutexLocker locker(&g_data()->mutex);
    if (g_data()->loaded && !force) {
        return;
    }
    const QScopedPointer<QSettings> configFile([]() -> QSettings * {
        if (!QCoreApplication::instance()) {
            return nullptr;
        }
        const QDir appDir(QCoreApplication::applicationDirPath());
        return new QSettings(appDir.filePath(kConfigFileName), QSettings::IniFormat);
    }());
    for (int i = 0; i != OptionCount; ++i) {
        const bool envVar = (!g_data()->disableEnvVar
            && qEnvironmentVariableIsSet(OptionsTable[i].env.constData())
            && (qEnvironmentVariableIntValue(OptionsTable[i].env.constData()) > 0));
        const bool cfgFile = (!g_data()->disableCfgFile && !configFile.isNull()
            && configFile->value(QUtf8String(OptionsTable[i].cfg), false).toBool());
        g_data()->options[i] = (envVar || cfgFile);
    }
    g_data()->loaded = true;
}

void FramelessConfig::set(const Option option, const bool on)
{
    const QMutexLocker locker(&g_data()->mutex);
    g_data()->options[static_cast<int>(option)] = on;
}

bool FramelessConfig::isSet(const Option option) const
{
    const QMutexLocker locker(&g_data()->mutex);
    return g_data()->options[static_cast<int>(option)];
}

void FramelessConfig::setLoadFromEnvironmentVariablesDisabled(const bool on)
{
    const QMutexLocker locker(&g_data()->mutex);
    g_data()->disableEnvVar = on;
}

void FramelessConfig::setLoadFromConfigurationFileDisabled(const bool on)
{
    const QMutexLocker locker(&g_data()->mutex);
    g_data()->disableCfgFile = on;
}

QVariant FramelessConfig::setInternal(const QString &key, const QVariant &value)
{
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(value.isValid());
    if (key.isEmpty() || !value.isValid()) {
        return {};
    }
    QVariant previous = {};
    const QMutexLocker locker(&g_data()->mutex);
    if (g_data()->internals.contains(key)) {
        previous = g_data()->internals.value(key);
        g_data()->internals.remove(key);
    }
    g_data()->internals.insert(key, value);
    return previous;
}

QVariant FramelessConfig::getInternal(const QString &key) const
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return {};
    }
    const QMutexLocker locker(&g_data()->mutex);
    if (g_data()->internals.contains(key)) {
        return g_data()->internals.value(key);
    }
    return {};
}

FRAMELESSHELPER_END_NAMESPACE
