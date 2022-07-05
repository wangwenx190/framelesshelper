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

#include "sysapiloader_p.h"
#include <QtCore/qdebug.h>
#ifdef Q_OS_WINDOWS
#  include <QtCore/private/qsystemlibrary_p.h>
#else
#  include <QtCore/qlibrary.h>
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(SysApiLoader, g_sysApiLoader)

SysApiLoader::SysApiLoader(QObject *parent) : QObject(parent)
{
}

SysApiLoader::~SysApiLoader() = default;

SysApiLoader *SysApiLoader::instance()
{
    return g_sysApiLoader();
}

QFunctionPointer SysApiLoader::resolve(const QString &library, const QString &function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(!function.isEmpty());
    if (library.isEmpty() || function.isEmpty()) {
        return nullptr;
    }
#ifdef Q_OS_WINDOWS
    return QSystemLibrary::resolve(library, qUtf8Printable(function));
#else
    return QLibrary::resolve(library, qUtf8Printable(function));
#endif
}

bool SysApiLoader::isAvailable(const QString &library, const QString &function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(!function.isEmpty());
    if (library.isEmpty() || function.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_mutex);
    if (m_functionCache.contains(function)) {
        return m_functionCache.value(function).has_value();
    }
    const QFunctionPointer symbol = SysApiLoader::resolve(library, function);
    if (symbol) {
        m_functionCache.insert(function, symbol);
        qDebug() << "Successfully loaded" << function << "from" << library;
        return true;
    }
    m_functionCache.insert(function, std::nullopt);
    qWarning() << "Failed to load" << function << "from" << library;
    return false;
}

QFunctionPointer SysApiLoader::get(const QString &function)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return nullptr;
    }
    QMutexLocker locker(&m_mutex);
    if (m_functionCache.contains(function)) {
        const std::optional<QFunctionPointer> symbol = m_functionCache.value(function);
        if (symbol.has_value()) {
            return symbol.value();
        }
    }
    return nullptr;
}

FRAMELESSHELPER_END_NAMESPACE
