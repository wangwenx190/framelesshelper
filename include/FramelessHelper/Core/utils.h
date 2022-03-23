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
#include <QtGui/qwindowdefs.h>
#include <functional>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

enum class SystemTheme : int
{
    Light = 0,
    Dark = 1,
    HighContrastLight = 2,
    HighContrastDark = 3
};
Q_ENUM_NS(SystemTheme)

enum class SystemButtonType : int
{
    WindowIcon = 0,
    Minimize = 1,
    Maximize = 2,
    Restore = 3,
    Close = 4
};
Q_ENUM_NS(SystemButtonType)

enum class ResourceType : int
{
    Image = 0,
    Pixmap = 1,
    Icon = 2
};
Q_ENUM_NS(ResourceType)

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

using GetWindowFlagsCallback = std::function<Qt::WindowFlags()>;
using SetWindowFlagsCallback = std::function<void(const Qt::WindowFlags)>;

using GetWindowSizeCallback = std::function<QSize()>;
using MoveWindowCallback = std::function<void(const int, const int)>;

using GetWindowScreenCallback = std::function<QScreen *()>;

namespace Utils
{

[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::CursorShape calculateCursorShape(const QWindow *window, const QPoint &pos);
[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::Edges calculateWindowEdges(const QWindow *window, const QPoint &pos);
FRAMELESSHELPER_CORE_API void startSystemMove(QWindow *window);
FRAMELESSHELPER_CORE_API void startSystemResize(QWindow *window, const Qt::Edges edges);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowFixedSize(const QWindow *window);
[[nodiscard]] FRAMELESSHELPER_CORE_API QVariant getSystemButtonIconResource
    (const SystemButtonType button, const SystemTheme theme, const ResourceType type);
FRAMELESSHELPER_CORE_API void sendMouseReleaseEvent();
[[nodiscard]] FRAMELESSHELPER_CORE_API QWindow *findWindow(const WId winId);
FRAMELESSHELPER_CORE_API void moveWindowToDesktopCenter(const GetWindowScreenCallback &getWindowScreen,
    const GetWindowSizeCallback &getWindowSize, const MoveWindowCallback &moveWindow, const bool considerTaskBar);

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8Point1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin101809OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin11OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isDwmCompositionEnabled();
FRAMELESSHELPER_CORE_API void triggerFrameChange(const WId winId);
FRAMELESSHELPER_CORE_API void updateWindowFrameMargins(const WId winId, const bool reset);
FRAMELESSHELPER_CORE_API void updateInternalWindowFrameMargins(QWindow *window, const bool enable);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getSystemErrorMessage(const QString &function);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFullScreen(const WId winId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowNoState(const WId winId);
FRAMELESSHELPER_CORE_API void syncWmPaintWithDwm();
FRAMELESSHELPER_CORE_API void showSystemMenu(const QWindow *window, const QPoint &pos);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getDwmColorizationColor();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode();
[[nodiscard]] FRAMELESSHELPER_CORE_API DwmColorizationArea getDwmColorizationArea();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isHighContrastModeEnabled();
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getPrimaryScreenDpi(const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getWindowDpi(const WId winId, const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThickness(const WId winId, const bool horizontal, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionHeight(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeight(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThickness(const WId winId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getFrameBorderColor(const bool active);
FRAMELESSHELPER_CORE_API void updateWindowFrameBorderColor(const WId winId, const bool dark);
FRAMELESSHELPER_CORE_API void fixupQtInternals(const WId winId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowFrameBorderVisible();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isTitleBarColorized();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFrameBorderColorized();
FRAMELESSHELPER_CORE_API void installSystemMenuHook(const QWindow *window);
FRAMELESSHELPER_CORE_API void uninstallSystemMenuHook(const WId winId);
FRAMELESSHELPER_CORE_API void tryToBeCompatibleWithQtFramelessWindowHint(const WId winId,
       const GetWindowFlagsCallback &getWindowFlags, const SetWindowFlagsCallback &setWindowFlags, const bool enable);
FRAMELESSHELPER_CORE_API void disableAeroSnapping(const WId winId);
FRAMELESSHELPER_CORE_API void tryToEnableHighestDpiAwarenessLevel();
#endif // Q_OS_WINDOWS

} // namespace Utils

FRAMELESSHELPER_END_NAMESPACE
