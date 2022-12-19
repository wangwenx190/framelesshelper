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

#include "settings.h"
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qrect.h>
#include <framelesshelpercore_global.h>

static std::unique_ptr<QSettings> g_settings = nullptr;

[[nodiscard]] static inline QSettings *appConfigFile()
{
    const QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    const QString iniFileName = fileInfo.completeBaseName() + FRAMELESSHELPER_STRING_LITERAL(".ini");
    const QString iniFilePath = fileInfo.canonicalPath() + u'/' + iniFileName;
    return new QSettings(iniFilePath, QSettings::IniFormat);
}

[[nodiscard]] static inline QString appKey(const QString &id, const QString &key)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return {};
    }
    return (id.isEmpty() ? key : (id + u'/' + key));
}

template<typename T>
void Settings::set(const QString &id, const QString &key, const T &data)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return;
    }
    if (!g_settings) {
        g_settings.reset(appConfigFile());
    }
    g_settings->setValue(appKey(id, key), data);
}

template<typename T>
T Settings::get(const QString &id, const QString &key)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty()) {
        return T{};
    }
    if (!g_settings) {
        g_settings.reset(appConfigFile());
    }
    return qvariant_cast<T>(g_settings->value(appKey(id, key)));
}

template void Settings::set<QRect>(const QString &, const QString &, const QRect &);
template void Settings::set<qreal>(const QString &, const QString &, const qreal &);
template void Settings::set<QByteArray>(const QString &, const QString &, const QByteArray &);

template QRect Settings::get<QRect>(const QString &, const QString &);
template qreal Settings::get<qreal>(const QString &, const QString &);
template QByteArray Settings::get<QByteArray>(const QString &, const QString &);
