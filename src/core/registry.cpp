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

#include "registry_p.h"
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#  include <QtCore/private/qwinregistry_p.h>
#else
#  include <QtCore/qsettings.h>
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcCoreRegistry, "wangwenx190.framelesshelper.core.registry")
#define INFO qCInfo(lcCoreRegistry)
#define DEBUG qCDebug(lcCoreRegistry)
#define WARNING qCWarning(lcCoreRegistry)
#define CRITICAL qCCritical(lcCoreRegistry)

using namespace Global;

static const HKEY g_keyMap[] = {
    HKEY_CLASSES_ROOT,
    HKEY_CURRENT_USER,
    HKEY_LOCAL_MACHINE,
    HKEY_USERS,
    HKEY_PERFORMANCE_DATA,
    HKEY_CURRENT_CONFIG,
    HKEY_DYN_DATA,
    HKEY_CURRENT_USER_LOCAL_SETTINGS,
    HKEY_PERFORMANCE_TEXT,
    HKEY_PERFORMANCE_NLSTEXT
};
static_assert(std::size(g_keyMap) == (static_cast<int>(RegistryRootKey::PerformanceNlsText) + 1));

static const QString g_strMap[] = {
    FRAMELESSHELPER_STRING_LITERAL("HKEY_CLASSES_ROOT"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_CURRENT_USER"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_LOCAL_MACHINE"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_USERS"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_PERFORMANCE_DATA"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_CURRENT_CONFIG"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_DYN_DATA"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_CURRENT_USER_LOCAL_SETTINGS"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_PERFORMANCE_TEXT"),
    FRAMELESSHELPER_STRING_LITERAL("HKEY_PERFORMANCE_NLSTEXT")
};
static_assert(std::size(g_strMap) == std::size(g_keyMap));

Registry::Registry(const RegistryRootKey root, const QString &key, QObject *parent) : QObject(parent)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return;
    }
    m_rootKey = root;
    m_subKey = key;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    m_registry.reset(new QWinRegistryKey(g_keyMap[static_cast<int>(m_rootKey)], m_subKey));
#else
    const QString rootKey = g_strMap[static_cast<int>(m_rootKey)];
    m_settings.reset(new QSettings(rootKey, QSettings::NativeFormat));
    if (m_settings->contains(m_subKey)) {
        m_settings.reset(new QSettings(rootKey + u'\\' + m_subKey, QSettings::NativeFormat));
        m_valid = true;
    }
#endif
}

Registry::~Registry() = default;

RegistryRootKey Registry::rootKey() const
{
    return m_rootKey;
}

QString Registry::subKey() const
{
    return m_subKey;
}

bool Registry::isValid() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return m_registry->isValid();
#else
    return m_valid;
#endif
}

QVariant Registry::value(const QString &name) const
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(isValid());
    if (name.isEmpty() || !isValid()) {
        return {};
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QPair<DWORD, bool> dwValue = m_registry->dwordValue(name);
    if (dwValue.second) {
        return qulonglong(dwValue.first);
    }
    return m_registry->stringValue(name);
#else
    return m_settings->value(name);
#endif
}

FRAMELESSHELPER_END_NAMESPACE
