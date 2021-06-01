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
#include <QtGui/qcolor.h>
#include <QtGui/qwindow.h>

namespace Utilities {

enum class SystemMetric
{
    BorderWidth,
    BorderHeight,
    TitleBarHeight
};

// Common
FRAMELESSHELPER_API int getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiAware, const bool forceSystemValue = false);

FRAMELESSHELPER_API bool isLightThemeEnabled();
FRAMELESSHELPER_API bool isDarkThemeEnabled();

FRAMELESSHELPER_API QWindow *findWindow(const WId winId);

FRAMELESSHELPER_API bool shouldUseNativeTitleBar();

FRAMELESSHELPER_API bool isWindowFixedSize(const QWindow *window);

FRAMELESSHELPER_API QPointF getGlobalMousePosition(const QWindow *window);

FRAMELESSHELPER_API bool isHitTestVisibleInChrome(const QWindow *window);

FRAMELESSHELPER_API QColor getNativeWindowFrameColor(const bool isActive = true);

FRAMELESSHELPER_API QObject *getNativeParent(const QObject *object);

FRAMELESSHELPER_API QPointF mapOriginPointToWindow(const QObject *object);

#ifdef Q_OS_WINDOWS
// Windows specific
FRAMELESSHELPER_API bool isWin7OrGreater();
FRAMELESSHELPER_API bool isWin8OrGreater();
FRAMELESSHELPER_API bool isWin8Point1OrGreater();
FRAMELESSHELPER_API bool isWin10OrGreater();
FRAMELESSHELPER_API bool isWin10OrGreater(const int subVer);

FRAMELESSHELPER_API bool isDwmBlurAvailable();

FRAMELESSHELPER_API bool isColorizationEnabled();
FRAMELESSHELPER_API QColor getColorizationColor();

FRAMELESSHELPER_API void triggerFrameChange(const QWindow *window);
FRAMELESSHELPER_API void updateFrameMargins(const QWindow *window, const bool reset);
FRAMELESSHELPER_API void updateQtFrameMargins(QWindow *window, const bool enable);

FRAMELESSHELPER_API quint32 getWindowDpi(const QWindow *window);
FRAMELESSHELPER_API QMargins getWindowNativeFrameMargins(const QWindow *window);

FRAMELESSHELPER_API void displaySystemMenu(const QWindow *window, const QPoint &pos = {});
#endif

}
