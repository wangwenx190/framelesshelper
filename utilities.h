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

#include "framelesshelper_global.h"
#include <QtGui/qwindow.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

#ifdef Q_OS_WINDOWS
enum class DwmColorizationArea : int
{
    None = 0,
    StartMenu_TaskBar_ActionCenter = 1,
    TitleBar_WindowBorder = 2,
    All = 3
};
Q_ENUM_NS(DwmColorizationArea)
#endif

namespace Utilities
{

[[nodiscard]] FRAMELESSHELPER_API Qt::CursorShape calculateCursorShape(const QWindow *window, const QPointF &pos);
[[nodiscard]] FRAMELESSHELPER_API Qt::Edges calculateWindowEdges(const QWindow *window, const QPointF &pos);
FRAMELESSHELPER_API void startSystemMove(QWindow *window);
FRAMELESSHELPER_API void startSystemResize(QWindow *window, const Qt::Edges edges);
[[nodiscard]] FRAMELESSHELPER_API bool isWindowFixedSize(const QWindow *window);

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_API bool isWin8OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin8Point1OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin10OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin101809OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isWin11OrGreater();
[[nodiscard]] FRAMELESSHELPER_API bool isDwmCompositionEnabled();
FRAMELESSHELPER_API void triggerFrameChange(const WId winId);
FRAMELESSHELPER_API void updateWindowFrameMargins(const WId winId, const bool reset);
FRAMELESSHELPER_API void updateInternalWindowFrameMargins(QWindow *window, const bool enable);
[[nodiscard]] FRAMELESSHELPER_API QString getSystemErrorMessage(const QString &function);
[[nodiscard]] FRAMELESSHELPER_API bool isFullScreen(const WId winId);
[[nodiscard]] FRAMELESSHELPER_API bool isWindowNoState(const WId winId);
FRAMELESSHELPER_API void syncWmPaintWithDwm();
FRAMELESSHELPER_API void showSystemMenu(const WId winId, const QPointF &pos);
[[nodiscard]] FRAMELESSHELPER_API QColor getDwmColorizationColor();
[[nodiscard]] FRAMELESSHELPER_API bool shouldAppsUseDarkMode();
[[nodiscard]] FRAMELESSHELPER_API DwmColorizationArea getDwmColorizationArea();
[[nodiscard]] FRAMELESSHELPER_API bool isHighContrastModeEnabled();
[[nodiscard]] FRAMELESSHELPER_API quint32 getPrimaryScreenDpi(const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_API quint32 getWindowDpi(const WId winId, const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_API quint32 getResizeBorderThickness(const WId winId, const bool horizontal, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_API quint32 getCaptionHeight(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_API quint32 getTitleBarHeight(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_API quint32 getFrameBorderThickness(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_API QColor getFrameBorderColor(const bool active);
FRAMELESSHELPER_API void updateWindowFrameBorderColor(const WId winId, const bool dark);
FRAMELESSHELPER_API void fixupQtInternals(const WId winId);
[[nodiscard]] FRAMELESSHELPER_API bool isWindowFrameBorderVisible();
#endif // Q_OS_WINDOWS

} // namespace Utilities

FRAMELESSHELPER_END_NAMESPACE
