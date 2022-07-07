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

#ifndef WS_EX_NOREDIRECTIONBITMAP
#  define WS_EX_NOREDIRECTIONBITMAP (0x00200000L)
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
#  define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef _DPI_AWARENESS_CONTEXTS_
#  define _DPI_AWARENESS_CONTEXTS_
   DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#  define DPI_AWARENESS_CONTEXT_UNAWARE ((DPI_AWARENESS_CONTEXT)-1)
#  define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE ((DPI_AWARENESS_CONTEXT)-2)
#  define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((DPI_AWARENESS_CONTEXT)-3)
#  define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#  define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED ((DPI_AWARENESS_CONTEXT)-5)
#endif

#ifndef STATUS_SUCCESS
#  define STATUS_SUCCESS (static_cast<NTSTATUS>(0x00000000L))
#endif

using NTSTATUS = LONG;

#ifndef WINMMAPI

#define WINMMAPI EXTERN_C DECLSPEC_IMPORT

using MMRESULT = UINT;

using TIMECAPS = struct TIMECAPS
{
    UINT wPeriodMin; // minimum period supported
    UINT wPeriodMax; // maximum period supported
};
using PTIMECAPS = TIMECAPS *;
using NPTIMECAPS = TIMECAPS NEAR *;
using LPTIMECAPS = TIMECAPS FAR *;

#endif

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

using _DWMWINDOWATTRIBUTE = enum _DWMWINDOWATTRIBUTE
{
    _DWMWA_USE_HOSTBACKDROPBRUSH = 17, // [set] BOOL, Allows the use of host backdrop brushes for the window.
    _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19, // Undocumented
    _DWMWA_USE_IMMERSIVE_DARK_MODE = 20, // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
    _DWMWA_WINDOW_CORNER_PREFERENCE = 33, // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
    _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37, // [get] UINT, width of the visible border around a thick frame window
    _DWMWA_SYSTEMBACKDROP_TYPE = 38, // [get, set] SYSTEMBACKDROP_TYPE, Controls the system-drawn backdrop material of a window, including behind the non-client area.
    _DWMWA_MICA_EFFECT = 1029 // Undocumented
};

using _DWM_WINDOW_CORNER_PREFERENCE = enum _DWM_WINDOW_CORNER_PREFERENCE
{
    _DWMWCP_DEFAULT = 0, // Let the system decide whether or not to round window corners
    _DWMWCP_DONOTROUND = 1, // Never round window corners
    _DWMWCP_ROUND = 2, // Round the corners if appropriate
    _DWMWCP_ROUNDSMALL = 3 // Round the corners if appropriate, with a small radius
};

using _DWM_SYSTEMBACKDROP_TYPE = enum _DWM_SYSTEMBACKDROP_TYPE
{
    _DWMSBT_AUTO = 0, // [Default] Let DWM automatically decide the system-drawn backdrop for this window.
    _DWMSBT_NONE = 1, // Do not draw any system backdrop.
    _DWMSBT_MAINWINDOW = 2, /* Mica */ // Draw the backdrop material effect corresponding to a long-lived window.
    _DWMSBT_TRANSIENTWINDOW = 3, /* Acrylic */ // Draw the backdrop material effect corresponding to a transient window.
    _DWMSBT_TABBEDWINDOW = 4 /* Aero */ // Draw the backdrop material effect corresponding to a window with a tabbed title bar.
};

using WINDOWCOMPOSITIONATTRIB = enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_CORNER_STYLE = 27,
    WCA_PART_COLOR = 28,
    WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
    WCA_LAST = 30
};

using ACCENT_STATE = enum ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3, // Traditional DWM blur
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_USE_HOST_BACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6 // Using this value will remove the window background
};

using ACCENT_POLICY = struct ACCENT_POLICY
{
    ACCENT_STATE State;
    DWORD Flags;
    DWORD GradientColor; // #AABBGGRR
    DWORD AnimationId;
};
using PACCENT_POLICY = ACCENT_POLICY *;
using NPACCENT_POLICY = ACCENT_POLICY NEAR *;
using LPACCENT_POLICY = ACCENT_POLICY FAR *;

using WINDOWCOMPOSITIONATTRIBDATA = struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};
using PWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA *;
using NPWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA NEAR *;
using LPWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA FAR *;

using GetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, PWINDOWCOMPOSITIONATTRIBDATA);
using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, PWINDOWCOMPOSITIONATTRIBDATA);

WINMMAPI MMRESULT WINAPI
timeGetDevCaps(
    _Out_writes_bytes_(cbtc) LPTIMECAPS ptc,
    _In_ UINT cbtc
);

WINMMAPI MMRESULT WINAPI
timeBeginPeriod(
    _In_ UINT uPeriod
);

WINMMAPI MMRESULT WINAPI
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

WINUSERAPI int WINAPI
GetSystemMetricsForDpi(
    _In_ int nIndex,
    _In_ UINT dpi
);

WINUSERAPI UINT WINAPI
GetDpiForWindow(
    _In_ HWND hwnd
);

WINUSERAPI UINT WINAPI
GetDpiForSystem(
    VOID
);

WINUSERAPI UINT WINAPI
GetSystemDpiForProcess(
    _In_ HANDLE hProcess
);

WINUSERAPI BOOL WINAPI
SetProcessDpiAwarenessContext(
    _In_ DPI_AWARENESS_CONTEXT value
);

WINUSERAPI BOOL WINAPI
SetProcessDPIAware(
    VOID
);

[[maybe_unused]] static constexpr const int kAutoHideTaskBarThickness = 2; // The thickness of an auto-hide taskbar in pixels.

[[maybe_unused]] static constexpr const wchar_t kDwmRegistryKey[] = LR"(Software\Microsoft\Windows\DWM)";
[[maybe_unused]] static constexpr const wchar_t kPersonalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
[[maybe_unused]] static constexpr const wchar_t kThemeSettingChangeEventName[] = L"ImmersiveColorSet";
[[maybe_unused]] static constexpr const wchar_t kDwmColorKeyName[] = L"ColorPrevalence";
[[maybe_unused]] static constexpr const wchar_t kSystemDarkThemeResourceName[] = L"DarkMode_Explorer";
[[maybe_unused]] static constexpr const wchar_t kSystemLightThemeResourceName[] = L"Explorer";
