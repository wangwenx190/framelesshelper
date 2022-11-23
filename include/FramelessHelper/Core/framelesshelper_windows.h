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

#ifndef NTDDI_WIN10_NI
#  define NTDDI_WIN10_NI 0x0A00000C
#endif

#ifndef WINVER
#  define WINVER _WIN32_WINNT_WIN10
#endif

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif

#ifndef NTDDI_VERSION
#  define NTDDI_VERSION NTDDI_WIN10_NI
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

#ifndef WM_GETDPISCALEDSIZE
#  define WM_GETDPISCALEDSIZE (0x02E4)
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

DECLARE_HANDLE(_DPI_AWARENESS_CONTEXT);

#ifndef _DPI_AWARENESS_CONTEXT_UNAWARE
#  define _DPI_AWARENESS_CONTEXT_UNAWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-1))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
#  define _DPI_AWARENESS_CONTEXT_SYSTEM_AWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-2))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
#  define _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-3))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#  define _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-4))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED
#  define _DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-5))
#endif

#ifndef HKEY_CLASSES_ROOT
#  define HKEY_CLASSES_ROOT (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000000))))
#endif

#ifndef HKEY_CURRENT_USER
#  define HKEY_CURRENT_USER (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000001))))
#endif

#ifndef HKEY_LOCAL_MACHINE
#  define HKEY_LOCAL_MACHINE (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000002))))
#endif

#ifndef HKEY_USERS
#  define HKEY_USERS (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000003))))
#endif

#ifndef HKEY_PERFORMANCE_DATA
#  define HKEY_PERFORMANCE_DATA (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000004))))
#endif

#ifndef HKEY_CURRENT_CONFIG
#  define HKEY_CURRENT_CONFIG (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000005))))
#endif

#ifndef HKEY_DYN_DATA
#  define HKEY_DYN_DATA (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000006))))
#endif

#ifndef HKEY_CURRENT_USER_LOCAL_SETTINGS
#  define HKEY_CURRENT_USER_LOCAL_SETTINGS (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000007))))
#endif

#ifndef HKEY_PERFORMANCE_TEXT
#  define HKEY_PERFORMANCE_TEXT (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000050))))
#endif

#ifndef HKEY_PERFORMANCE_NLSTEXT
#  define HKEY_PERFORMANCE_NLSTEXT (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(static_cast<LONG>(0x80000060))))
#endif

#ifndef _STATUS_SUCCESS
#  define _STATUS_SUCCESS (static_cast<_NTSTATUS>(0x00000000L))
#endif

#ifndef WTNCA_NODRAWCAPTION
#  define WTNCA_NODRAWCAPTION (0x00000001) // don't draw the window caption
#endif

#ifndef WTNCA_NODRAWICON
#  define WTNCA_NODRAWICON (0x00000002) // don't draw the system icon
#endif

#ifndef WTNCA_NOSYSMENU
#  define WTNCA_NOSYSMENU (0x00000004) // don't expose the system menu icon functionality
#endif

#ifndef WTNCA_NOMIRRORHELP
#  define WTNCA_NOMIRRORHELP (0x00000008) // don't mirror the question mark, even in RTL layout
#endif

#ifndef WTNCA_VALIDBITS
#  define WTNCA_VALIDBITS (WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON | WTNCA_NOSYSMENU | WTNCA_NOMIRRORHELP)
#endif

#ifndef EXTERN_C
#  define EXTERN_C extern "C"
#endif

#ifndef EXTERN_C_START
#  define EXTERN_C_START EXTERN_C {
#endif

#ifndef EXTERN_C_END
#  define EXTERN_C_END }
#endif

using _NTSTATUS = LONG;

using _MMRESULT = UINT;
using _TIMECAPS = struct _TIMECAPS
{
    UINT wPeriodMin; // minimum period supported
    UINT wPeriodMax; // maximum period supported
};
using _PTIMECAPS = _TIMECAPS *;

using _PROCESS_DPI_AWARENESS = enum _PROCESS_DPI_AWARENESS
{
    _PROCESS_DPI_UNAWARE = 0,
    _PROCESS_SYSTEM_DPI_AWARE = 1,
    _PROCESS_PER_MONITOR_DPI_AWARE = 2,
    _PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3,
    _PROCESS_DPI_UNAWARE_GDISCALED = 4
};

using _MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE
{
    _MDT_EFFECTIVE_DPI = 0,
    _MDT_ANGULAR_DPI = 1,
    _MDT_RAW_DPI = 2,
    _MDT_DEFAULT = _MDT_EFFECTIVE_DPI
};

using _DEVICE_SCALE_FACTOR = enum _DEVICE_SCALE_FACTOR
{
    _DEVICE_SCALE_FACTOR_INVALID = 0,
    _SCALE_100_PERCENT = 100,
    _SCALE_120_PERCENT = 120,
    _SCALE_125_PERCENT = 125,
    _SCALE_140_PERCENT = 140,
    _SCALE_150_PERCENT = 150,
    _SCALE_160_PERCENT = 160,
    _SCALE_175_PERCENT = 175,
    _SCALE_180_PERCENT = 180,
    _SCALE_200_PERCENT = 200,
    _SCALE_225_PERCENT = 225,
    _SCALE_250_PERCENT = 250,
    _SCALE_300_PERCENT = 300,
    _SCALE_350_PERCENT = 350,
    _SCALE_400_PERCENT = 400,
    _SCALE_450_PERCENT = 450,
    _SCALE_500_PERCENT = 500
};

using _DPI_AWARENESS = enum _DPI_AWARENESS
{
    _DPI_AWARENESS_INVALID = -1,
    _DPI_AWARENESS_UNAWARE = 0,
    _DPI_AWARENESS_SYSTEM_AWARE = 1,
    _DPI_AWARENESS_PER_MONITOR_AWARE = 2,
    _DPI_AWARENESS_PER_MONITOR_AWARE_V2 = 3,
    _DPI_AWARENESS_UNAWARE_GDISCALED = 4
};

using _DWMWINDOWATTRIBUTE = enum _DWMWINDOWATTRIBUTE
{
    _DWMWA_USE_HOSTBACKDROPBRUSH = 17, // [set] BOOL, Allows the use of host backdrop brushes for the window.
    _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19, // Undocumented, the same with DWMWA_USE_IMMERSIVE_DARK_MODE, but available on systems before Win10 20H1.
    _DWMWA_USE_IMMERSIVE_DARK_MODE = 20, // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
    _DWMWA_WINDOW_CORNER_PREFERENCE = 33, // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
    _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37, // [get] UINT, width of the visible border around a thick frame window
    _DWMWA_SYSTEMBACKDROP_TYPE = 38, // [get, set] SYSTEMBACKDROP_TYPE, Controls the system-drawn backdrop material of a window, including behind the non-client area.
    _DWMWA_MICA_EFFECT = 1029 // Undocumented, use this value to enable Mica material on Win11 21H2. You should use DWMWA_SYSTEMBACKDROP_TYPE instead on Win11 22H2 and newer.
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
    _DWMSBT_TABBEDWINDOW = 4 /* Mica Alt */ // Draw the backdrop material effect corresponding to a window with a tabbed title bar.
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

using WINDOWCOMPOSITIONATTRIBDATA = struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    DWORD cbData;
};
using PWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA *;

using _WINDOWTHEMEATTRIBUTETYPE = enum _WINDOWTHEMEATTRIBUTETYPE
{
    _WTA_NONCLIENT = 1
};

using WTA_OPTIONS2 = struct WTA_OPTIONS2
{
    DWORD dwFlags; // Values for each style option specified in the bitmask.
    DWORD dwMask; // Bitmask for flags that are changing.
};
using PWTA_OPTIONS2 = WTA_OPTIONS2 *;

using IMMERSIVE_HC_CACHE_MODE = enum IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE = 0,
    IHCM_REFRESH = 1
};

using PREFERRED_APP_MODE = enum PREFERRED_APP_MODE
{
    PAM_DEFAULT = 0, // Default behavior on systems before Win10 1809. It indicates the application doesn't support dark mode at all.
    PAM_AUTO = 1, // Available since Win10 1809, let system decide whether to enable dark mode or not.
    PAM_DARK = 2, // Available since Win10 1903, force dark mode regardless of the system theme.
    PAM_LIGHT = 3, // Available since Win10 1903, force light mode regardless of the system theme.
    PAM_MAX = 4
};

using GetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, PWINDOWCOMPOSITIONATTRIBDATA);
using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, PWINDOWCOMPOSITIONATTRIBDATA);
// Win10 1809 (10.0.17763)
using ShouldAppsUseDarkModePtr = BOOL(WINAPI *)(VOID); // Ordinal 132
using AllowDarkModeForWindowPtr = BOOL(WINAPI *)(HWND, BOOL); // Ordinal 133
using AllowDarkModeForAppPtr = BOOL(WINAPI *)(BOOL); // Ordinal 135
using FlushMenuThemesPtr = VOID(WINAPI *)(VOID); // Ordinal 136
using RefreshImmersiveColorPolicyStatePtr = VOID(WINAPI *)(VOID); // Ordinal 104
using IsDarkModeAllowedForWindowPtr = BOOL(WINAPI *)(HWND); // Ordinal 137
using GetIsImmersiveColorUsingHighContrastPtr = BOOL(WINAPI *)(IMMERSIVE_HC_CACHE_MODE); // Ordinal 106
using OpenNcThemeDataPtr = HTHEME(WINAPI *)(HWND, LPCWSTR); // Ordinal 49
// Win10 1903 (10.0.18362)
using ShouldSystemUseDarkModePtr = BOOL(WINAPI *)(VOID); // Ordinal 138
using SetPreferredAppModePtr = PREFERRED_APP_MODE(WINAPI *)(PREFERRED_APP_MODE); // Ordinal 135
using IsDarkModeAllowedForAppPtr = BOOL(WINAPI *)(VOID); // Ordinal 139

[[maybe_unused]] inline constexpr const int kAutoHideTaskBarThickness = 2; // The thickness of an auto-hide taskbar in pixels.
[[maybe_unused]] inline constexpr const wchar_t kDwmRegistryKey[] = LR"(Software\Microsoft\Windows\DWM)";
[[maybe_unused]] inline constexpr const wchar_t kPersonalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
[[maybe_unused]] inline constexpr const wchar_t kThemeSettingChangeEventName[] = L"ImmersiveColorSet";
[[maybe_unused]] inline constexpr const wchar_t kDwmColorKeyName[] = L"ColorPrevalence";
[[maybe_unused]] inline constexpr const wchar_t kSystemDarkThemeResourceName[] = L"DarkMode_Explorer";
[[maybe_unused]] inline constexpr const wchar_t kSystemLightThemeResourceName[] = L"Explorer";
[[maybe_unused]] inline constexpr const wchar_t kDesktopRegistryKey[] = LR"(Control Panel\Desktop)";
[[maybe_unused]] inline constexpr const wchar_t kDarkModePropertyName[] = L"UseImmersiveDarkModeColors";
