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

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINRT_LEAN_AND_MEAN
#  define WINRT_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#  define UNICODE
#endif

#ifndef _UNICODE
#  define _UNICODE
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#  define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <sdkddkver.h>

#ifndef _WIN32_WINNT_WIN10
#  define _WIN32_WINNT_WIN10 0x0A00
#endif

#ifndef NTDDI_WIN10_CO
#  define NTDDI_WIN10_CO 0x0A00000B
#endif

#ifndef WINVER
#  define WINVER _WIN32_WINNT_WIN10
#endif

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif

#ifndef NTDDI_VERSION
#  define NTDDI_VERSION NTDDI_WIN10_CO
#endif

#include <windows.h>
#include <uxtheme.h>
#include <shellapi.h>
#include <dwmapi.h>

#ifndef WM_NCUAHDRAWCAPTION
#  define WM_NCUAHDRAWCAPTION (0x00AE)
#endif

#ifndef WM_NCUAHDRAWFRAME
#  define WM_NCUAHDRAWFRAME (0x00AF)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#  define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#  define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#  define WM_DPICHANGED (0x02E0)
#endif

#ifndef SM_CXPADDEDBORDER
#  define SM_CXPADDEDBORDER (92)
#endif

#ifndef SM_CYPADDEDBORDER
#  define SM_CYPADDEDBORDER SM_CXPADDEDBORDER
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#  define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef GET_X_LPARAM
#  define GET_X_LPARAM(lp) (static_cast<int>(static_cast<short>(LOWORD(lp))))
#endif

#ifndef GET_Y_LPARAM
#  define GET_Y_LPARAM(lp) (static_cast<int>(static_cast<short>(HIWORD(lp))))
#endif

#ifndef IsMinimized
#  define IsMinimized(hwnd) (IsIconic(hwnd) != FALSE)
#endif

#ifndef IsMaximized
#  define IsMaximized(hwnd) (IsZoomed(hwnd) != FALSE)
#endif

#ifndef MMSYSERR_NOERROR
#  define MMSYSERR_NOERROR (0)
#endif

#ifndef TIMERR_NOERROR
#  define TIMERR_NOERROR (0)
#endif

using MMRESULT = UINT;

using TIMECAPS = struct timecaps_tag
{
    UINT wPeriodMin; // minimum period supported
    UINT wPeriodMax; // maximum period supported
};
using PTIMECAPS = TIMECAPS *;
using NPTIMECAPS = TIMECAPS NEAR *;
using LPTIMECAPS = TIMECAPS FAR *;

using PROCESS_DPI_AWARENESS = enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2,
    PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3
};

using MONITOR_DPI_TYPE = enum MONITOR_DPI_TYPE
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
};

using _DWM_WINDOW_CORNER_PREFERENCE = enum _DWM_WINDOW_CORNER_PREFERENCE
{
    _DWMWCP_DEFAULT = 0,
    _DWMWCP_DONOTROUND = 1,
    _DWMWCP_ROUND = 2,
    _DWMWCP_ROUNDSMALL = 3
};

EXTERN_C MMRESULT WINAPI
timeGetDevCaps(
    _Out_writes_bytes_(cbtc) LPTIMECAPS ptc,
    _In_ UINT cbtc
);

EXTERN_C MMRESULT WINAPI
timeBeginPeriod(
    _In_ UINT uPeriod
);

EXTERN_C MMRESULT WINAPI
timeEndPeriod(
    _In_ UINT uPeriod
);

EXTERN_C HRESULT WINAPI
SetProcessDpiAwareness(
    _In_ PROCESS_DPI_AWARENESS value
);

EXTERN_C HRESULT WINAPI
GetDpiForMonitor(
    _In_ HMONITOR hMonitor,
    _In_ MONITOR_DPI_TYPE dpiType,
    _Out_ UINT *dpiX,
    _Out_ UINT *dpiY
);

[[maybe_unused]] static constexpr const int kAutoHideTaskBarThickness = 2; // The thickness of an auto-hide taskbar in pixels.

[[maybe_unused]] static constexpr const wchar_t kDwmRegistryKey[] = LR"(Software\Microsoft\Windows\DWM)";
[[maybe_unused]] static constexpr const wchar_t kPersonalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
[[maybe_unused]] static constexpr const wchar_t kThemeSettingChangeEventName[] = L"ImmersiveColorSet";
[[maybe_unused]] static constexpr const wchar_t kDwmColorKeyName[] = L"ColorPrevalence";
[[maybe_unused]] static constexpr const wchar_t kSystemDarkThemeResourceName[] = L"DarkMode_Explorer";
[[maybe_unused]] static constexpr const wchar_t kSystemLightThemeResourceName[] = L"Explorer";

[[maybe_unused]] static constexpr const DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
[[maybe_unused]] static constexpr const DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
[[maybe_unused]] static constexpr const DWORD _DWMWA_WINDOW_CORNER_PREFERENCE = 33;
[[maybe_unused]] static constexpr const DWORD _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;
