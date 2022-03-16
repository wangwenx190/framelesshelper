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
#include <QtCore/qmutex.h>
#include <QtCore/qhash.h>
#include <QtCore/quuid.h>
#include <QtGui/qwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessHelperQt;

class FRAMELESSHELPER_CORE_API FramelessManagerPrivate
{
    Q_DISABLE_COPY_MOVE(FramelessManagerPrivate)

    friend class FramelessWindowsManager;

public:
    explicit FramelessManagerPrivate();
    ~FramelessManagerPrivate();

    [[nodiscard]] static FramelessManagerPrivate *instance();

    [[nodiscard]] QUuid findIdByWindow(QWindow *value) const;
    [[nodiscard]] QUuid findIdByWinId(const WId value) const;

    [[nodiscard]] QWindow *findWindowById(const QUuid &value) const;
    [[nodiscard]] WId findWinIdById(const QUuid &value) const;

private:
    mutable QMutex mutex = {};
    QHash<QWindow *, QUuid> windowMapping = {};
    QHash<WId, QUuid> winIdMapping = {};
    QHash<QUuid, FramelessHelperQt *> qtFramelessHelpers = {};
#ifdef Q_OS_WINDOWS
    QHash<QUuid, QMetaObject::Connection> win32WorkaroundConnections = {};
#endif
};

FRAMELESSHELPER_END_NAMESPACE
