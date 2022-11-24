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

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcUtilsCommon)
#ifdef Q_OS_WINDOWS
  Q_DECLARE_LOGGING_CATEGORY(lcUtilsWin)
#elif defined(Q_OS_LINUX)
  Q_DECLARE_LOGGING_CATEGORY(lcUtilsLinux)
#elif defined(Q_OS_MACOS)
  Q_DECLARE_LOGGING_CATEGORY(lcUtilsMac)
#endif

namespace Utils
{

[[nodiscard]] FRAMELESSHELPER_CORE_API
    Qt::CursorShape calculateCursorShape(const QWindow *window, const QPoint &pos);
[[nodiscard]] FRAMELESSHELPER_CORE_API
    Qt::Edges calculateWindowEdges(const QWindow *window, const QPoint &pos);
FRAMELESSHELPER_CORE_API void startSystemMove(QWindow *window, const QPoint &globalPos);
FRAMELESSHELPER_CORE_API void startSystemResize(QWindow *window, const Qt::Edges edges, const QPoint &globalPos);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getSystemButtonIconCode(const Global::SystemButtonType button);
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
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isTitleBarColorized();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool
    setBlurBehindWindowEnabled(const WId windowId, const Global::BlurMode mode, const QColor &color);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getWallpaperFilePath();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::WallpaperAspectStyle getWallpaperAspectStyle();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isBlurBehindWindowSupported();
FRAMELESSHELPER_CORE_API void registerThemeChangeNotification();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getFrameBorderColor(const bool active);
[[nodiscard]] FRAMELESSHELPER_CORE_API qreal roundScaleFactor(const qreal factor);

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowsVersionOrGreater(const Global::WindowsVersion version);
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
    const bool selectFirstEntry,
    const Global::IsWindowFixedSizeCallback &isWindowFixedSize);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getDwmColorizationColor();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::DwmColorizationArea getDwmColorizationArea();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isHighContrastModeEnabled();
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getPrimaryScreenDpi(const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getWindowDpi(const WId windowId, const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThicknessForDpi
    (const bool horizontal, const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThickness(const WId windowId,
                                                                        const bool horizontal,
                                                                        const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionBarHeightForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionBarHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeightForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThicknessForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThickness(const WId windowId,
                                                                       const bool scaled);
FRAMELESSHELPER_CORE_API void maybeFixupQtInternals(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowFrameBorderVisible();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFrameBorderColorized();
FRAMELESSHELPER_CORE_API void installSystemMenuHook(
    const WId windowId,
    const Global::IsWindowFixedSizeCallback &isWindowFixedSize,
    const Global::IsInsideTitleBarDraggableAreaCallback &isInTitleBarArea,
    const Global::GetWindowDevicePixelRatioCallback &getDevicePixelRatio);
FRAMELESSHELPER_CORE_API void uninstallSystemMenuHook(const WId windowId);
FRAMELESSHELPER_CORE_API void setAeroSnappingEnabled(const WId windowId, const bool enable);
FRAMELESSHELPER_CORE_API void tryToEnableHighestDpiAwarenessLevel();
FRAMELESSHELPER_CORE_API void updateGlobalWin32ControlsTheme(const WId windowId, const bool dark);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_windows();
FRAMELESSHELPER_CORE_API void setCornerStyleForWindow(const WId windowId, const Global::WindowCornerStyle style);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getDwmAccentColor();
FRAMELESSHELPER_CORE_API void hideOriginalTitleBarElements
    (const WId windowId, const bool disable = true);
FRAMELESSHELPER_CORE_API void setQtDarkModeAwareEnabled(const bool enable);
FRAMELESSHELPER_CORE_API void refreshWin32ThemeResources(const WId windowId, const bool dark);
FRAMELESSHELPER_CORE_API void enableNonClientAreaDpiScalingForWindow(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API
    Global::DpiAwareness getDpiAwarenessForCurrentProcess(bool *highest = nullptr);
FRAMELESSHELPER_CORE_API void fixupChildWindowsDpiMessage(const WId windowId);
FRAMELESSHELPER_CORE_API void fixupDialogsDpiScaling();
FRAMELESSHELPER_CORE_API void setDarkModeAllowedForApp(const bool allow = true);
FRAMELESSHELPER_CORE_API void bringWindowToFront(const WId windowId);
#endif // Q_OS_WINDOWS

#ifdef Q_OS_LINUX
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_linux();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getWmThemeColor();
#endif // Q_OS_LINUX

#ifdef Q_OS_MACOS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_macos();
FRAMELESSHELPER_CORE_API void setSystemTitleBarVisible(const WId windowId, const bool visible);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getControlsAccentColor();
FRAMELESSHELPER_CORE_API void removeWindowProxy(const WId windowId);
#endif // Q_OS_MACOS
} // namespace Utils

FRAMELESSHELPER_END_NAMESPACE
