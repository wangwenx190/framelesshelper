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

#include "registrykey_p.h"
#include "framelesshelper_windows.h"
#if REGISTRYKEY_QWINREGISTRYKEY
#  include <QtCore/private/qwinregistry_p.h>
#else
#  include <QtCore/qsettings.h>
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcCoreRegistryKey, "wangwenx190.framelesshelper.core.registrykey")
#define INFO qCInfo(lcCoreRegistryKey)
#define DEBUG qCDebug(lcCoreRegistryKey)
#define WARNING qCWarning(lcCoreRegistryKey)
#define CRITICAL qCCritical(lcCoreRegistryKey)

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

RegistryKey::RegistryKey(const RegistryRootKey root, const QString &key, QObject *parent) : QObject(parent)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return;
    }
    m_rootKey = root;
    m_subKey = key;
#if REGISTRYKEY_QWINREGISTRYKEY
    m_registryKey.reset(new QWinRegistryKey(g_keyMap[static_cast<int>(m_rootKey)], m_subKey));
    if (!m_registryKey->isValid()) {
        m_registryKey.reset();
    }
#else
    const QString rootKey = g_strMap[static_cast<int>(m_rootKey)];
    const auto lastSlashPos = m_subKey.lastIndexOf(u'\\');
    m_settings.reset(new QSettings(rootKey + u'\\' + m_subKey.left(lastSlashPos), QSettings::NativeFormat));
    if (m_settings->childGroups().contains(m_subKey.mid(lastSlashPos + 1))) {
        m_settings.reset(new QSettings(rootKey + u'\\' + m_subKey, QSettings::NativeFormat));
    } else {
        m_settings.reset();
    }
#endif
}

RegistryKey::~RegistryKey() = default;

RegistryRootKey RegistryKey::rootKey() const
{
    return m_rootKey;
}

QString RegistryKey::subKey() const
{
    return m_subKey;
}

bool RegistryKey::isValid() const
{
#if REGISTRYKEY_QWINREGISTRYKEY
    return (!m_registryKey.isNull() && m_registryKey->isValid());
#else
    return !m_settings.isNull();
#endif
}

QVariant RegistryKey::value(const QString &name) const
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(isValid());
    if (name.isEmpty() || !isValid()) {
        return {};
    }
#if REGISTRYKEY_QWINREGISTRYKEY
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    return m_registryKey->value(name);
#  else // (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    const QPair<DWORD, bool> dwVal = m_registryKey->dwordValue(name);
    if (dwVal.second) {
        return qulonglong(dwVal.first);
    }
    const QString strVal = m_registryKey->stringValue(name);
    if (!strVal.isEmpty()) {
        return strVal;
    }
    return {};
#  endif // (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#else // !REGISTRYKEY_QWINREGISTRYKEY
    return m_settings->value(name);
#endif // REGISTRYKEY_QWINREGISTRYKEY
}

FRAMELESSHELPER_END_NAMESPACE
