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

FRAMELESSHELPER_BEGIN_NAMESPACE

namespace Utils
{

[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::CursorShape calculateCursorShape(const QWindow *window,
                                                                            const QPoint &pos);
[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::Edges calculateWindowEdges(const QWindow *window,
                                                                      const QPoint &pos);
FRAMELESSHELPER_CORE_API void startSystemMove(QWindow *window);
FRAMELESSHELPER_CORE_API void startSystemResize(QWindow *window, const Qt::Edges edges);
[[nodiscard]] FRAMELESSHELPER_CORE_API QVariant
getSystemButtonIconResource(const Global::SystemButtonType button,
                            const Global::SystemTheme theme,
                            const Global::ResourceType type);
[[nodiscard]] FRAMELESSHELPER_CORE_API QWindow *findWindow(const WId windowId);
FRAMELESSHELPER_CORE_API void moveWindowToDesktopCenter(
    const Global::GetWindowScreenCallback &getWindowScreen,
    const Global::GetWindowSizeCallback &getWindowSize,
    const Global::SetWindowPositionCallback &setWindowPosition,
    const bool considerTaskBar);
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::SystemTheme getSystemTheme();
[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::WindowState windowStatesToWindowState(
    const Qt::WindowStates states);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isThemeChangeEvent(const QEvent * const event);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor calculateSystemButtonBackgroundColor(
    const Global::SystemButtonType button, const Global::ButtonState state);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode();

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8Point1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin101607OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin101809OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin11OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isDwmCompositionEnabled();
FRAMELESSHELPER_CORE_API void triggerFrameChange(const WId windowId);
FRAMELESSHELPER_CORE_API void updateWindowFrameMargins(const WId windowId, const bool reset);
FRAMELESSHELPER_CORE_API void updateInternalWindowFrameMargins(QWindow *window, const bool enable);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getSystemErrorMessage(const QString &function);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFullScreen(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowNoState(const WId windowId);
FRAMELESSHELPER_CORE_API void syncWmPaintWithDwm();
FRAMELESSHELPER_CORE_API void showSystemMenu(
    const WId windowId,
    const QPoint &pos,
    const QPoint &offset,
    const bool selectFirstEntry,
    const Global::Options options,
    const Global::IsWindowFixedSizeCallback &isWindowFixedSize);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getDwmColorizationColor();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::DwmColorizationArea getDwmColorizationArea();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isHighContrastModeEnabled();
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getPrimaryScreenDpi(const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getWindowDpi(const WId windowId, const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThickness(const WId windowId,
                                                                        const bool horizontal,
                                                                        const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThickness(const WId windowId,
                                                                       const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getFrameBorderColor(const bool active);
FRAMELESSHELPER_CORE_API void updateWindowFrameBorderColor(const WId windowId, const bool dark);
FRAMELESSHELPER_CORE_API void fixupQtInternals(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowFrameBorderVisible();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isTitleBarColorized();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFrameBorderColorized();
FRAMELESSHELPER_CORE_API void installSystemMenuHook(
    const WId windowId,
    const Global::Options options,
    const QPoint &offset,
    const Global::IsWindowFixedSizeCallback &isWindowFixedSize);
FRAMELESSHELPER_CORE_API void uninstallSystemMenuHook(const WId windowId);
FRAMELESSHELPER_CORE_API void tryToBeCompatibleWithQtFramelessWindowHint(
    const WId windowId,
    const Global::GetWindowFlagsCallback &getWindowFlags,
    const Global::SetWindowFlagsCallback &setWindowFlags,
    const bool enable);
FRAMELESSHELPER_CORE_API void setAeroSnappingEnabled(const WId windowId, const bool enable);
FRAMELESSHELPER_CORE_API void tryToEnableHighestDpiAwarenessLevel();
FRAMELESSHELPER_CORE_API void updateGlobalWin32ControlsTheme(const WId windowId, const bool dark);
#endif // Q_OS_WINDOWS

} // namespace Utils

FRAMELESSHELPER_END_NAMESPACE
