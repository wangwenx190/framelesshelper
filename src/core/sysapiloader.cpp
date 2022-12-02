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

#ifndef SYSAPILOADER_FORCE_QLIBRARY
#  define SYSAPILOADER_FORCE_QLIBRARY (0)
#endif // SYSAPILOADER_FORCE_QLIBRARY

#ifndef SYSAPILOADER_IMPL
#  if (defined(Q_OS_WINDOWS) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
#    define SYSAPILOADER_IMPL (1)
#  else // (!Q_OS_WINDOWS || FRAMELESSHELPER_CORE_NO_PRIVATE)
#    define SYSAPILOADER_IMPL (2)
#  endif // (defined(Q_OS_WINDOWS) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
#endif // SYSAPILOADER_IMPL

#ifndef SYSAPILOADER_QSYSTEMLIBRARY
#  define SYSAPILOADER_QSYSTEMLIBRARY ((SYSAPILOADER_IMPL) == 1)
#endif // SYSAPILOADER_QSYSTEMLIBRARY

#ifndef SYSAPILOADER_QLIBRARY
#  define SYSAPILOADER_QLIBRARY ((SYSAPILOADER_IMPL) == 2)
#endif // SYSAPILOADER_QLIBRARY

#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#if SYSAPILOADER_QSYSTEMLIBRARY
#  include <QtCore/private/qsystemlibrary_p.h>
#endif // SYSAPILOADER_QSYSTEMLIBRARY
#if SYSAPILOADER_QLIBRARY
#  include <QtCore/qlibrary.h>
#endif // SYSAPILOADER_QLIBRARY

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSysApiLoader, "wangwenx190.framelesshelper.core.sysapiloader")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcSysApiLoader)
#  define DEBUG qCDebug(lcSysApiLoader)
#  define WARNING qCWarning(lcSysApiLoader)
#  define CRITICAL qCCritical(lcSysApiLoader)
#endif

struct SysApiLoaderData
{
    QMutex mutex;
    QHash<QString, std::optional<QFunctionPointer>> functionCache = {};
};

Q_GLOBAL_STATIC(SysApiLoaderData, g_loaderData)

Q_GLOBAL_STATIC(SysApiLoader, g_sysApiLoader)

SysApiLoader::SysApiLoader(QObject *parent) : QObject(parent)
{
}

SysApiLoader::~SysApiLoader() = default;

SysApiLoader *SysApiLoader::instance()
{
    return g_sysApiLoader();
}

QFunctionPointer SysApiLoader::resolve(const QString &library, const char *function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(function);
    if (library.isEmpty() || !function) {
        return nullptr;
    }
#if SYSAPILOADER_QSYSTEMLIBRARY
    return QSystemLibrary::resolve(library, function);
#endif // SYSAPILOADER_QSYSTEMLIBRARY
#if SYSAPILOADER_QLIBRARY
    return QLibrary::resolve(library, function);
#endif // SYSAPILOADER_QLIBRARY
}

QFunctionPointer SysApiLoader::resolve(const QString &library, const QByteArray &function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(!function.isEmpty());
    if (library.isEmpty() || function.isEmpty()) {
        return nullptr;
    }
    return SysApiLoader::resolve(library, function.constData());
}

QFunctionPointer SysApiLoader::resolve(const QString &library, const QString &function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(!function.isEmpty());
    if (library.isEmpty() || function.isEmpty()) {
        return nullptr;
    }
    return SysApiLoader::resolve(library, function.toUtf8());
}

bool SysApiLoader::isAvailable(const QString &library, const QString &function)
{
    Q_ASSERT(!library.isEmpty());
    Q_ASSERT(!function.isEmpty());
    if (library.isEmpty() || function.isEmpty()) {
        return false;
    }
    const QMutexLocker locker(&g_loaderData()->mutex);
    if (g_loaderData()->functionCache.contains(function)) {
        return g_loaderData()->functionCache.value(function).has_value();
    }
    const QFunctionPointer symbol = SysApiLoader::resolve(library, function);
    if (symbol) {
        g_loaderData()->functionCache.insert(function, symbol);
        DEBUG << "Successfully loaded" << function << "from" << library;
        return true;
    }
    g_loaderData()->functionCache.insert(function, std::nullopt);
    WARNING << "Failed to load" << function << "from" << library;
    return false;
}

QFunctionPointer SysApiLoader::get(const QString &function)
{
    Q_ASSERT(!function.isEmpty());
    if (function.isEmpty()) {
        return nullptr;
    }
    const QMutexLocker locker(&g_loaderData()->mutex);
    if (g_loaderData()->functionCache.contains(function)) {
        return g_loaderData()->functionCache.value(function).value_or(nullptr);
    }
    return nullptr;
}

FRAMELESSHELPER_END_NAMESPACE
