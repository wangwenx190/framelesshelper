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

#pragma once

#include "framelesshelpercore_global.h"
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_CORE_API SysApiLoader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SysApiLoader)

public:
    explicit SysApiLoader(QObject *parent = nullptr);
    ~SysApiLoader() override;

    Q_NODISCARD static SysApiLoader *instance();

    Q_NODISCARD static QFunctionPointer resolve(const QString &library, const char *function);
    Q_NODISCARD static QFunctionPointer resolve(const QString &library, const QByteArray &function);
    Q_NODISCARD static QFunctionPointer resolve(const QString &library, const QString &function);

    Q_NODISCARD bool isAvailable(const QString &library, const QString &function);

    Q_NODISCARD QFunctionPointer get(const QString &function);

    template<typename T>
    Q_NODISCARD inline T get(const QString &function)
    {
        return reinterpret_cast<T>(get(function));
    }

private:
    static inline QMutex m_mutex;
    static inline QHash<QString, std::optional<QFunctionPointer>> m_functionCache = {};
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(SysApiLoader))

#ifdef Q_OS_WINDOWS
#  define API_WIN_AVAILABLE(lib, func) (SysApiLoader::instance()->isAvailable(k##lib, k##func))
#  define API_USER_AVAILABLE(func) API_WIN_AVAILABLE(user32, func)
#  define API_THEME_AVAILABLE(func) API_WIN_AVAILABLE(uxtheme, func)
#  define API_DWM_AVAILABLE(func) API_WIN_AVAILABLE(dwmapi, func)
#  define API_SHCORE_AVAILABLE(func) API_WIN_AVAILABLE(shcore, func)
#  define API_WINMM_AVAILABLE(func) API_WIN_AVAILABLE(winmm, func)
#  define API_D2D_AVAILABLE(func) API_WIN_AVAILABLE(d2d1, func)
#  define API_NT_AVAILABLE(func) API_WIN_AVAILABLE(ntdll, func)
#endif

#define API_CALL_FUNCTION(func, ...) \
  ((SysApiLoader::instance()->get<decltype(&::func)>(k##func))(__VA_ARGS__))
