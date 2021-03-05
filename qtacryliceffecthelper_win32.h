/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "framelesshelper_global.h"
#include <QtCore/qcoreevent.h>
#include <QtCore/qabstractnativeeventfilter.h>

class FRAMELESSHELPER_EXPORT QtAcrylicWinUpdateEvent : public QEvent
{
public:
    static const int QtAcrylicEffectChangeEventId;

    explicit QtAcrylicWinUpdateEvent(const bool clearWallpaper = false);
    ~QtAcrylicWinUpdateEvent() override;

    inline bool shouldClearPreviousWallpaper() const
    {
        return m_shouldClearPreviousWallpaper;
    }

private:
    bool m_shouldClearPreviousWallpaper = false;
};

class FRAMELESSHELPER_EXPORT QtAcrylicWinEventFilter : public QAbstractNativeEventFilter
{
public:
    explicit QtAcrylicWinEventFilter();
    ~QtAcrylicWinEventFilter() override;

    static void setup();
    static void unsetup();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
};
