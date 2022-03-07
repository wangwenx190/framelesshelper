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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <sdkddkver.h>

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif

#ifndef NTDDI_WIN10_CO
#define NTDDI_WIN10_CO 0x0A00000B
#endif

#ifdef WINVER
#undef WINVER
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define NTDDI_VERSION NTDDI_WIN10_CO

#include <QtCore/qt_windows.h>
#include <shellapi.h>
#include <dwmapi.h>

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION (0x00AE)
#endif

#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME (0x00AF)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) (static_cast<int>(static_cast<short>(LOWORD(lp))))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) (static_cast<int>(static_cast<short>(HIWORD(lp))))
#endif

#ifndef IsMinimized
#define IsMinimized(window) (IsIconic(window) != FALSE)
#endif

#ifndef IsMaximized
#define IsMaximized(window) (IsZoomed(window) != FALSE)
#endif

struct flh_timecaps_tag
{
    UINT wPeriodMin; // minimum period supported
    UINT wPeriodMax; // maximum period supported
};
using flh_TIMECAPS = flh_timecaps_tag;
using flh_PTIMECAPS = flh_timecaps_tag *;
using flh_NPTIMECAPS = flh_timecaps_tag * NEAR;
using flh_LPTIMECAPS = flh_timecaps_tag * FAR;

[[maybe_unused]] static constexpr const UINT kAutoHideTaskbarThickness = 2; // The thickness of an auto-hide taskbar in pixels

[[maybe_unused]] static constexpr const char kDwmRegistryKey[] = R"(Software\Microsoft\Windows\DWM)";
[[maybe_unused]] static constexpr const char kPersonalizeRegistryKey[] = R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

[[maybe_unused]] static constexpr const UINT kDefaultResizeBorderThicknessClassic = 4;
[[maybe_unused]] static constexpr const UINT kDefaultResizeBorderThicknessAero = 8;
[[maybe_unused]] static constexpr const UINT kDefaultCaptionHeight = 23;

[[maybe_unused]] static constexpr const DWORD _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;
