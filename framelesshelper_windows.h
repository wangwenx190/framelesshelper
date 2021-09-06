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

#ifndef NTDDI_WIN10_19H1
#define NTDDI_WIN10_19H1 (0x0A000007)
#endif

#ifdef WINVER
#undef WINVER
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define WINVER NTDDI_WIN10_19H1
#define _WIN32_WINNT NTDDI_WIN10_19H1

#include <QtCore/qt_windows.h>
#include <shellapi.h>
#include <dwmapi.h>

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION (0x00AE)
#endif

#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME (0x00AF)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef IsMinimized
#define IsMinimized(window) (IsIconic(window) != FALSE)
#endif

#ifndef IsMaximized
#define IsMaximized(window) (IsZoomed(window) != FALSE)
#endif

[[maybe_unused]] constexpr int kAutoHideTaskbarThickness = 2; // The thickness of an auto-hide taskbar in pixels

[[maybe_unused]] constexpr char kDwmRegistryKey[] = R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)";
[[maybe_unused]] constexpr char kPersonalizeRegistryKey[] = R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

[[maybe_unused]] constexpr int kDefaultResizeBorderThicknessClassic = 4;
[[maybe_unused]] constexpr int kDefaultResizeBorderThicknessAero = 8;
[[maybe_unused]] constexpr int kDefaultCaptionHeight = 23;

[[maybe_unused]] constexpr WORD _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;
