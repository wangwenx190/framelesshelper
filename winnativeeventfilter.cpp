/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "winnativeeventfilter.h"

#include <d2d1.h>
#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QSettings>
#include <QWindow>
#include <QtMath>
#include <qpa/qplatformwindow.h>
#include <qt_windows.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QOperatingSystemVersion>
#else
#include <QSysInfo>
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <qpa/qplatformnativeinterface.h>
#else
#include <qpa/qplatformwindow_p.h>
#endif
#ifdef WNEF_LINK_SYSLIB
#include <dwmapi.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <windowsx.h>
#endif

Q_DECLARE_METATYPE(QMargins)

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#define qAsConst(i) std::as_const(i)
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
// Only available since Windows Vista
#define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER 92
#endif

#ifndef WM_NCUAHDRAWCAPTION
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWCAPTION 0x00AE
#endif

#ifndef WM_NCUAHDRAWFRAME
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWFRAME 0x00AF
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
// Only available since Windows Vista
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif

#ifndef WM_DPICHANGED
// Only available since Windows 8.1
#define WM_DPICHANGED 0x02E0
#endif

#ifndef ABM_GETAUTOHIDEBAREX
// Only available since Windows 8.1
#define ABM_GETAUTOHIDEBAREX 0x0000000b
#endif

#ifndef WNEF_GENERATE_WINAPI
#define WNEF_GENERATE_WINAPI(funcName, resultType, ...) \
    using _WNEF_WINAPI_##funcName = resultType(WINAPI *)(__VA_ARGS__); \
    _WNEF_WINAPI_##funcName m_lp##funcName = nullptr;
#endif

#ifndef WNEF_RESOLVE_ERROR
#ifdef _DEBUG
#define WNEF_RESOLVE_ERROR(funcName) Q_ASSERT(m_lp##funcName);
#else
#define WNEF_RESOLVE_ERROR(funcName) \
    if (!m_lp##funcName) { \
        qFatal("Failed to resolve symbol: " #funcName); \
    }
#endif
#endif

#ifndef WNEF_RESOLVE_WINAPI
#define WNEF_RESOLVE_WINAPI(libName, funcName) \
    if (!m_lp##funcName) { \
        m_lp##funcName = reinterpret_cast<_WNEF_WINAPI_##funcName>( \
            QLibrary::resolve(QString::fromUtf8(#libName), #funcName)); \
        WNEF_RESOLVE_ERROR(funcName) \
    }
#endif

#ifndef WNEF_RESOLVE_WINAPI2
#define WNEF_RESOLVE_WINAPI2(libName, funcName, ordinal) \
    if (!m_lp##funcName) { \
        m_lp##funcName = reinterpret_cast<_WNEF_WINAPI_##funcName>( \
            QLibrary::resolve(QString::fromUtf8(#libName), MAKEINTRESOURCEA(ordinal))); \
        WNEF_RESOLVE_ERROR(funcName) \
    }
#endif

#ifndef WNEF_EXECUTE_WINAPI
#ifdef WNEF_LINK_SYSLIB
#define WNEF_EXECUTE_WINAPI(funcName, ...) funcName(__VA_ARGS__);
#else
#define WNEF_EXECUTE_WINAPI(funcName, ...) \
    if (coreData()->m_lp##funcName) { \
        coreData()->m_lp##funcName(__VA_ARGS__); \
    }
#endif
#endif

#ifndef WNEF_EXECUTE_WINAPI_RETURN
#ifdef WNEF_LINK_SYSLIB
#define WNEF_EXECUTE_WINAPI_RETURN(funcName, defVal, ...) funcName(__VA_ARGS__)
#else
#define WNEF_EXECUTE_WINAPI_RETURN(funcName, defVal, ...) \
    (coreData()->m_lp##funcName ? coreData()->m_lp##funcName(__VA_ARGS__) : defVal)
#endif
#endif

namespace {

enum : WORD { DwmwaUseImmersiveDarkMode = 20, DwmwaUseImmersiveDarkModeBefore20h1 = 19 };

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB {
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_ALLOW_NCPAINT = 4,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_ACCENT_POLICY = 19,
    WCA_USEDARKMODECOLORS = 26
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using ACCENT_STATE = enum _ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

using ACCENT_POLICY = struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

using IMMERSIVE_HC_CACHE_MODE = enum _IMMERSIVE_HC_CACHE_MODE {
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

using PREFERRED_APP_MODE = enum _PREFERRED_APP_MODE {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

bool isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8;
#endif
}

bool isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1;
#endif
}

bool isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

bool isWin10OrGreater(const int ver)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current()
           >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, ver);
#else
    Q_UNUSED(ver)
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

#ifndef WNEF_LINK_SYSLIB

// All the following enums, structs and function prototypes are copied from
// Windows 10 SDK directly, without any modifications.

#ifdef IsMinimized
#undef IsMinimized
#endif

#ifdef IsMaximized
#undef IsMaximized
#endif

// Only available since Windows 2000
#define IsMinimized(h) WNEF_EXECUTE_WINAPI_RETURN(IsIconic, FALSE, h)

// Only available since Windows 2000
#define IsMaximized(h) WNEF_EXECUTE_WINAPI_RETURN(IsZoomed, FALSE, h)

#ifndef GET_X_LPARAM
// Only available since Windows 2000
#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
// Only available since Windows 2000
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))
#endif

#ifndef GetStockBrush
// Only available since Windows 2000
#define GetStockBrush(i) \
    (reinterpret_cast<HBRUSH>(WNEF_EXECUTE_WINAPI_RETURN(GetStockObject, 0, i)))
#endif

#ifndef ABM_GETSTATE
// Only available since Windows XP
#define ABM_GETSTATE 0x00000004
#endif

#ifndef ABM_GETTASKBARPOS
// Only available since Windows XP
#define ABM_GETTASKBARPOS 0x00000005
#endif

#ifndef ABS_AUTOHIDE
// Only available since Windows XP
#define ABS_AUTOHIDE 0x0000001
#endif

#ifndef ABE_LEFT
// Only available since Windows XP
#define ABE_LEFT 0
#endif

#ifndef ABE_TOP
// Only available since Windows XP
#define ABE_TOP 1
#endif

#ifndef ABE_RIGHT
// Only available since Windows XP
#define ABE_RIGHT 2
#endif

#ifndef ABE_BOTTOM
// Only available since Windows XP
#define ABE_BOTTOM 3
#endif

#ifndef BPPF_ERASE
// Only available since Windows Vista
#define BPPF_ERASE 0x0001
#endif

#ifndef BPPF_NOCLIP
// Only available since Windows Vista
#define BPPF_NOCLIP 0x0002
#endif

#ifndef DWM_BB_ENABLE
// Only available since Windows Vista
#define DWM_BB_ENABLE 0x00000001
#endif

#ifndef DWM_BB_BLURREGION
// Only available since Windows Vista
#define DWM_BB_BLURREGION 0x00000002
#endif

#ifndef DWM_BB_TRANSITIONONMAXIMIZED
// Only available since Windows Vista
#define DWM_BB_TRANSITIONONMAXIMIZED 0x00000004
#endif

using HPAINTBUFFER = HANDLE;

using MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE { MDT_EFFECTIVE_DPI = 0 };

using DWMNCRENDERINGPOLICY = enum _DWMNCRENDERINGPOLICY { DWMNCRP_ENABLED = 2 };

using DWMWINDOWATTRIBUTE = enum _DWMWINDOWATTRIBUTE {
    DWMWA_NCRENDERING_POLICY = 2,
    DWMWA_EXTENDED_FRAME_BOUNDS = 9
};

using MARGINS = struct _MARGINS
{
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
};

using APPBARDATA = struct _APPBARDATA
{
    DWORD cbSize;
    HWND hWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
    LPARAM lParam;
};

using PROCESS_DPI_AWARENESS = enum _PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
};

using BP_BUFFERFORMAT = enum _BP_BUFFERFORMAT { BPBF_TOPDOWNDIB = 2 };

using BLENDFUNCTION = struct _BLENDFUNCTION
{
    BYTE BlendOp;
    BYTE BlendFlags;
    BYTE SourceConstantAlpha;
    BYTE AlphaFormat;
};

using BP_PAINTPARAMS = struct _BP_PAINTPARAMS
{
    DWORD cbSize;
    DWORD dwFlags;
    CONST RECT *prcExclude;
    CONST BLENDFUNCTION *pBlendFunction;
};

using DWM_BLURBEHIND = struct _DWM_BLURBEHIND
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
};

#endif // WNEF_LINK_SYSLIB

// Internal data structure.
using WNEF_CORE_DATA = struct _WNEF_CORE_DATA
{
    _WNEF_CORE_DATA() { ResolveWin32APIs(); }
    ~_WNEF_CORE_DATA() = default;

    // These functions are undocumented APIs so we have to
    // load them dynamically unconditionally.
    WNEF_GENERATE_WINAPI(GetWindowCompositionAttribute, BOOL, HWND, WINDOWCOMPOSITIONATTRIBDATA *)
    WNEF_GENERATE_WINAPI(SetWindowCompositionAttribute, BOOL, HWND, WINDOWCOMPOSITIONATTRIBDATA *)
    WNEF_GENERATE_WINAPI(ShouldAppsUseDarkMode, BOOL)
    WNEF_GENERATE_WINAPI(AllowDarkModeForWindow, BOOL, HWND, BOOL)
    WNEF_GENERATE_WINAPI(AllowDarkModeForApp, BOOL, BOOL)
    WNEF_GENERATE_WINAPI(IsDarkModeAllowedForWindow, BOOL, HWND)
    WNEF_GENERATE_WINAPI(GetIsImmersiveColorUsingHighContrast, BOOL, IMMERSIVE_HC_CACHE_MODE)
    WNEF_GENERATE_WINAPI(RefreshImmersiveColorPolicyState, VOID)
    WNEF_GENERATE_WINAPI(ShouldSystemUseDarkMode, BOOL)
    WNEF_GENERATE_WINAPI(SetPreferredAppMode, PREFERRED_APP_MODE, PREFERRED_APP_MODE)
    WNEF_GENERATE_WINAPI(IsDarkModeAllowedForApp, BOOL)

#ifndef WNEF_LINK_SYSLIB
    // Some of the following functions are not used by this code anymore,
    // but we don't remove them completely because we may still need them later.
    WNEF_GENERATE_WINAPI(DwmEnableBlurBehindWindow, HRESULT, HWND, CONST DWM_BLURBEHIND *)
    WNEF_GENERATE_WINAPI(DwmExtendFrameIntoClientArea, HRESULT, HWND, CONST MARGINS *)
    WNEF_GENERATE_WINAPI(DwmIsCompositionEnabled, HRESULT, BOOL *)
    WNEF_GENERATE_WINAPI(DwmSetWindowAttribute, HRESULT, HWND, DWORD, LPCVOID, DWORD)
    WNEF_GENERATE_WINAPI(SHAppBarMessage, UINT_PTR, DWORD, APPBARDATA *)
    WNEF_GENERATE_WINAPI(GetDeviceCaps, int, HDC, int)
    WNEF_GENERATE_WINAPI(DefWindowProcW, LRESULT, HWND, UINT, WPARAM, LPARAM)
    WNEF_GENERATE_WINAPI(SetLayeredWindowAttributes, BOOL, HWND, COLORREF, BYTE, DWORD)
    WNEF_GENERATE_WINAPI(MoveWindow, BOOL, HWND, int, int, int, int, BOOL)
    WNEF_GENERATE_WINAPI(IsZoomed, BOOL, HWND)
    WNEF_GENERATE_WINAPI(IsIconic, BOOL, HWND)
    WNEF_GENERATE_WINAPI(GetSystemMetrics, int, int)
    WNEF_GENERATE_WINAPI(GetDC, HDC, HWND)
    WNEF_GENERATE_WINAPI(ReleaseDC, int, HWND, HDC)
    WNEF_GENERATE_WINAPI(RedrawWindow, BOOL, HWND, CONST RECT *, HRGN, UINT)
    WNEF_GENERATE_WINAPI(GetClientRect, BOOL, HWND, LPRECT)
    WNEF_GENERATE_WINAPI(GetWindowRect, BOOL, HWND, LPRECT)
    WNEF_GENERATE_WINAPI(ScreenToClient, BOOL, HWND, LPPOINT)
    WNEF_GENERATE_WINAPI(EqualRect, BOOL, CONST RECT *, CONST RECT *)
#ifdef Q_PROCESSOR_X86_64
    WNEF_GENERATE_WINAPI(GetWindowLongPtrW, LONG_PTR, HWND, int)
    WNEF_GENERATE_WINAPI(SetWindowLongPtrW, LONG_PTR, HWND, int, LONG_PTR)
    WNEF_GENERATE_WINAPI(GetClassLongPtrW, ULONG_PTR, HWND, int)
    WNEF_GENERATE_WINAPI(SetClassLongPtrW, ULONG_PTR, HWND, int, LONG_PTR)
#else // Q_PROCESSOR_X86_64
#ifdef LONG_PTR
#undef LONG_PTR
#endif
#define LONG_PTR LONG
    WNEF_GENERATE_WINAPI(GetWindowLongW, LONG_PTR, HWND, int)
    WNEF_GENERATE_WINAPI(SetWindowLongW, LONG_PTR, HWND, int, LONG_PTR)
#define m_lpGetWindowLongPtrW m_lpGetWindowLongW
#define m_lpSetWindowLongPtrW m_lpSetWindowLongW
#ifdef GWLP_USERDATA
#undef GWLP_USERDATA
#endif
#define GWLP_USERDATA GWL_USERDATA
    WNEF_GENERATE_WINAPI(GetClassLongW, DWORD, HWND, int)
    WNEF_GENERATE_WINAPI(SetClassLongW, DWORD, HWND, int, LONG_PTR)
#define m_lpGetClassLongPtrW m_lpGetClassLongW
#define m_lpSetClassLongPtrW m_lpSetClassLongW
#ifdef GCLP_HBRBACKGROUND
#undef GCLP_HBRBACKGROUND
#endif
#define GCLP_HBRBACKGROUND GCL_HBRBACKGROUND
#endif // Q_PROCESSOR_X86_64
    WNEF_GENERATE_WINAPI(FindWindowW, HWND, LPCWSTR, LPCWSTR)
    WNEF_GENERATE_WINAPI(MonitorFromWindow, HMONITOR, HWND, DWORD)
    WNEF_GENERATE_WINAPI(GetMonitorInfoW, BOOL, HMONITOR, LPMONITORINFO)
    WNEF_GENERATE_WINAPI(GetAncestor, HWND, HWND, UINT)
    WNEF_GENERATE_WINAPI(GetDesktopWindow, HWND)
    WNEF_GENERATE_WINAPI(SendMessageW, LRESULT, HWND, UINT, WPARAM, LPARAM)
    WNEF_GENERATE_WINAPI(SetWindowPos, BOOL, HWND, HWND, int, int, int, int, UINT)
    WNEF_GENERATE_WINAPI(UpdateWindow, BOOL, HWND)
    WNEF_GENERATE_WINAPI(InvalidateRect, BOOL, HWND, CONST RECT *, BOOL)
    WNEF_GENERATE_WINAPI(SetWindowRgn, int, HWND, HRGN, BOOL)
    WNEF_GENERATE_WINAPI(IsWindow, BOOL, HWND)
    WNEF_GENERATE_WINAPI(GetWindowInfo, BOOL, HWND, LPWINDOWINFO)
    WNEF_GENERATE_WINAPI(CreateSolidBrush, HBRUSH, COLORREF)
    WNEF_GENERATE_WINAPI(FillRect, int, HDC, CONST RECT *, HBRUSH)
    WNEF_GENERATE_WINAPI(DeleteObject, BOOL, HGDIOBJ)
    WNEF_GENERATE_WINAPI(IsThemeActive, BOOL)
    WNEF_GENERATE_WINAPI(BeginPaint, HDC, HWND, LPPAINTSTRUCT)
    WNEF_GENERATE_WINAPI(EndPaint, BOOL, HWND, CONST PAINTSTRUCT *)
    WNEF_GENERATE_WINAPI(GetCurrentProcess, HANDLE)
    WNEF_GENERATE_WINAPI(IsProcessDPIAware, BOOL)
    WNEF_GENERATE_WINAPI(
        D2D1CreateFactory, HRESULT, D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS *, void **)
    WNEF_GENERATE_WINAPI(AdjustWindowRectEx, BOOL, LPRECT, DWORD, BOOL, DWORD)
    WNEF_GENERATE_WINAPI(DwmDefWindowProc, BOOL, HWND, UINT, WPARAM, LPARAM, LRESULT *)
    WNEF_GENERATE_WINAPI(DwmGetWindowAttribute, HRESULT, HWND, DWORD, PVOID, DWORD)
    WNEF_GENERATE_WINAPI(GetStockObject, HGDIOBJ, int)
    WNEF_GENERATE_WINAPI(BufferedPaintSetAlpha, HRESULT, HPAINTBUFFER, CONST RECT *, BYTE)
    WNEF_GENERATE_WINAPI(EndBufferedPaint, HRESULT, HPAINTBUFFER, BOOL)
    WNEF_GENERATE_WINAPI(BeginBufferedPaint,
                         HPAINTBUFFER,
                         HDC,
                         CONST RECT *,
                         BP_BUFFERFORMAT,
                         BP_PAINTPARAMS *,
                         HDC *)
    WNEF_GENERATE_WINAPI(CreateRectRgnIndirect, HRGN, CONST RECT *)
    WNEF_GENERATE_WINAPI(GetDCEx, HDC, HWND, HRGN, DWORD)
    WNEF_GENERATE_WINAPI(GetWindowDC, HDC, HWND)
    WNEF_GENERATE_WINAPI(OffsetRect, BOOL, LPRECT, int, int)
    WNEF_GENERATE_WINAPI(GetSystemMenu, HMENU, HWND, BOOL)
    WNEF_GENERATE_WINAPI(SetMenuItemInfoW, BOOL, HMENU, UINT, BOOL, LPCMENUITEMINFOW)
    WNEF_GENERATE_WINAPI(TrackPopupMenu, BOOL, HMENU, UINT, int, int, int, HWND, CONST RECT *)
    WNEF_GENERATE_WINAPI(PostMessageW, BOOL, HWND, UINT, WPARAM, LPARAM)
    WNEF_GENERATE_WINAPI(GetMessagePos, DWORD)
    WNEF_GENERATE_WINAPI(SystemParametersInfoW, BOOL, UINT, UINT, PVOID, UINT)
    WNEF_GENERATE_WINAPI(DwmGetColorizationColor, HRESULT, DWORD *, BOOL *)

#endif // WNEF_LINK_SYSLIB

    void loadUndocumentedAPIs()
    {
        static bool resolved = false;
        if (resolved) {
            // Don't resolve twice.
            return;
        }
        resolved = true;
        // Available since Windows 7
        WNEF_RESOLVE_WINAPI(User32, GetWindowCompositionAttribute)
        WNEF_RESOLVE_WINAPI(User32, SetWindowCompositionAttribute)
        // Available since Windows 10, version 1809 (10.0.17763)
        if (isWin10OrGreater(17763)) {
            WNEF_RESOLVE_WINAPI2(UxTheme, ShouldAppsUseDarkMode, 132)
            WNEF_RESOLVE_WINAPI2(UxTheme, AllowDarkModeForWindow, 133)
            WNEF_RESOLVE_WINAPI2(UxTheme, AllowDarkModeForApp, 135)
            WNEF_RESOLVE_WINAPI2(UxTheme, RefreshImmersiveColorPolicyState, 104)
            WNEF_RESOLVE_WINAPI2(UxTheme, IsDarkModeAllowedForWindow, 137)
            WNEF_RESOLVE_WINAPI2(UxTheme, GetIsImmersiveColorUsingHighContrast, 106)
        }
        // Available since Windows 10, version 1903 (10.0.18362)
        if (isWin10OrGreater(18362)) {
            WNEF_RESOLVE_WINAPI2(UxTheme, ShouldSystemUseDarkMode, 138)
            WNEF_RESOLVE_WINAPI2(UxTheme, SetPreferredAppMode, 135)
            WNEF_RESOLVE_WINAPI2(UxTheme, IsDarkModeAllowedForApp, 139)
        }
    }

    // These functions were introduced in Win10 1607 or later (mostly),
    // so we always load them dynamically.
    WNEF_GENERATE_WINAPI(GetDpiForMonitor, HRESULT, HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *)
    WNEF_GENERATE_WINAPI(GetProcessDpiAwareness, HRESULT, HANDLE, PROCESS_DPI_AWARENESS *)
    WNEF_GENERATE_WINAPI(GetSystemDpiForProcess, UINT, HANDLE)
    WNEF_GENERATE_WINAPI(GetDpiForWindow, UINT, HWND)
    WNEF_GENERATE_WINAPI(GetDpiForSystem, UINT)
    WNEF_GENERATE_WINAPI(GetSystemMetricsForDpi, int, int, UINT)
    WNEF_GENERATE_WINAPI(AdjustWindowRectExForDpi, BOOL, LPRECT, DWORD, BOOL, DWORD, UINT)

    void loadDPIFunctions()
    {
        static bool resolved = false;
        if (resolved) {
            // Don't resolve twice.
            return;
        }
        resolved = true;
        // Available since Windows 8.1
        if (isWin8Point1OrGreater()) {
            WNEF_RESOLVE_WINAPI(SHCore, GetDpiForMonitor)
            WNEF_RESOLVE_WINAPI(SHCore, GetProcessDpiAwareness)
        }
        // Available since Windows 10, version 1607 (10.0.14393)
        if (isWin10OrGreater(14393)) {
            WNEF_RESOLVE_WINAPI(User32, GetDpiForWindow)
            WNEF_RESOLVE_WINAPI(User32, GetDpiForSystem)
            WNEF_RESOLVE_WINAPI(User32, GetSystemMetricsForDpi)
            WNEF_RESOLVE_WINAPI(User32, AdjustWindowRectExForDpi)
        }
        // Available since Windows 10, version 1803 (10.0.17134)
        if (isWin10OrGreater(17134)) {
            WNEF_RESOLVE_WINAPI(User32, GetSystemDpiForProcess)
        }
    }

#ifdef WNEF_LINK_SYSLIB

    void ResolveWin32APIs()
    {
        static bool resolved = false;
        if (resolved) {
            // Don't resolve twice.
            return;
        }
        resolved = true;
        loadUndocumentedAPIs();
        loadDPIFunctions();
    }

#else // WNEF_LINK_SYSLIB

    // Some APIs are not available on old systems, so we will load them
    // dynamically at run-time to get maximum compatibility.
    void ResolveWin32APIs()
    {
        static bool resolved = false;
        if (resolved) {
            // Don't resolve twice.
            return;
        }
        resolved = true;
        // Available since Windows 2000.
        WNEF_RESOLVE_WINAPI(User32, SystemParametersInfoW)
        WNEF_RESOLVE_WINAPI(User32, GetMessagePos)
        WNEF_RESOLVE_WINAPI(User32, GetSystemMenu)
        WNEF_RESOLVE_WINAPI(User32, SetMenuItemInfoW)
        WNEF_RESOLVE_WINAPI(User32, TrackPopupMenu)
        WNEF_RESOLVE_WINAPI(User32, PostMessageW)
        WNEF_RESOLVE_WINAPI(User32, OffsetRect)
        WNEF_RESOLVE_WINAPI(User32, GetWindowDC)
        WNEF_RESOLVE_WINAPI(User32, GetDCEx)
        WNEF_RESOLVE_WINAPI(User32, AdjustWindowRectEx)
        WNEF_RESOLVE_WINAPI(User32, EndPaint)
        WNEF_RESOLVE_WINAPI(User32, BeginPaint)
        WNEF_RESOLVE_WINAPI(User32, FillRect)
        WNEF_RESOLVE_WINAPI(User32, GetWindowInfo)
        WNEF_RESOLVE_WINAPI(User32, IsWindow)
        WNEF_RESOLVE_WINAPI(User32, SetWindowRgn)
        WNEF_RESOLVE_WINAPI(User32, InvalidateRect)
        WNEF_RESOLVE_WINAPI(User32, UpdateWindow)
        WNEF_RESOLVE_WINAPI(User32, SetWindowPos)
        WNEF_RESOLVE_WINAPI(User32, SendMessageW)
        WNEF_RESOLVE_WINAPI(User32, GetDesktopWindow)
        WNEF_RESOLVE_WINAPI(User32, GetAncestor)
        WNEF_RESOLVE_WINAPI(User32, DefWindowProcW)
        WNEF_RESOLVE_WINAPI(User32, SetLayeredWindowAttributes)
        WNEF_RESOLVE_WINAPI(User32, MoveWindow)
        WNEF_RESOLVE_WINAPI(User32, IsZoomed)
        WNEF_RESOLVE_WINAPI(User32, IsIconic)
        WNEF_RESOLVE_WINAPI(User32, GetSystemMetrics)
        WNEF_RESOLVE_WINAPI(User32, GetDC)
        WNEF_RESOLVE_WINAPI(User32, ReleaseDC)
        WNEF_RESOLVE_WINAPI(User32, RedrawWindow)
        WNEF_RESOLVE_WINAPI(User32, GetClientRect)
        WNEF_RESOLVE_WINAPI(User32, GetWindowRect)
        WNEF_RESOLVE_WINAPI(User32, ScreenToClient)
        WNEF_RESOLVE_WINAPI(User32, EqualRect)
#ifdef Q_PROCESSOR_X86_64
        // These functions only exist in 64 bit User32.dll
        WNEF_RESOLVE_WINAPI(User32, GetWindowLongPtrW)
        WNEF_RESOLVE_WINAPI(User32, SetWindowLongPtrW)
        WNEF_RESOLVE_WINAPI(User32, GetClassLongPtrW)
        WNEF_RESOLVE_WINAPI(User32, SetClassLongPtrW)
#else // Q_PROCESSOR_X86_64
        WNEF_RESOLVE_WINAPI(User32, GetWindowLongW)
        WNEF_RESOLVE_WINAPI(User32, SetWindowLongW)
        WNEF_RESOLVE_WINAPI(User32, GetClassLongW)
        WNEF_RESOLVE_WINAPI(User32, SetClassLongW)
#endif // Q_PROCESSOR_X86_64
        WNEF_RESOLVE_WINAPI(User32, FindWindowW)
        WNEF_RESOLVE_WINAPI(User32, MonitorFromWindow)
        WNEF_RESOLVE_WINAPI(User32, GetMonitorInfoW)
        WNEF_RESOLVE_WINAPI(Gdi32, GetDeviceCaps)
        WNEF_RESOLVE_WINAPI(Gdi32, CreateSolidBrush)
        WNEF_RESOLVE_WINAPI(Gdi32, DeleteObject)
        WNEF_RESOLVE_WINAPI(Gdi32, GetStockObject)
        WNEF_RESOLVE_WINAPI(Gdi32, CreateRectRgnIndirect)
        // Available since Windows XP.
        WNEF_RESOLVE_WINAPI(Shell32, SHAppBarMessage)
        WNEF_RESOLVE_WINAPI(Kernel32, GetCurrentProcess)
        // Available since Windows Vista.
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmGetColorizationColor)
        WNEF_RESOLVE_WINAPI(User32, IsProcessDPIAware)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmGetWindowAttribute)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmIsCompositionEnabled)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmExtendFrameIntoClientArea)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmSetWindowAttribute)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmDefWindowProc)
        WNEF_RESOLVE_WINAPI(Dwmapi, DwmEnableBlurBehindWindow)
        WNEF_RESOLVE_WINAPI(UxTheme, IsThemeActive)
        WNEF_RESOLVE_WINAPI(UxTheme, BufferedPaintSetAlpha)
        WNEF_RESOLVE_WINAPI(UxTheme, EndBufferedPaint)
        WNEF_RESOLVE_WINAPI(UxTheme, BeginBufferedPaint)
        // Available since Windows 7.
        WNEF_RESOLVE_WINAPI(D2D1, D2D1CreateFactory)
        loadUndocumentedAPIs();
        loadDPIFunctions();
    }

#endif // WNEF_LINK_SYSLIB

    QScopedPointer<WinNativeEventFilter> m_instance;
};

} // namespace

Q_GLOBAL_STATIC(WNEF_CORE_DATA, coreData)

namespace {

const quint32 m_defaultDotsPerInch = USER_DEFAULT_SCREEN_DPI;

const qreal m_defaultDevicePixelRatio = 1.0;

const char envVarUseNativeTitleBar[] = "WNEF_USE_NATIVE_TITLE_BAR";
const char envVarPreserveWindowFrame[] = "WNEF_PRESERVE_WINDOW_FRAME";
const char envVarForceWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char envVarForceAcrylic[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";

bool shouldUseNativeTitleBar()
{
    return qEnvironmentVariableIsSet(envVarUseNativeTitleBar);
}

bool shouldHaveWindowFrame()
{
    if (shouldUseNativeTitleBar()) {
        // We have to use the original window frame unconditionally if we
        // want to use the native title bar.
        return true;
    }
    const bool should = qEnvironmentVariableIsSet(envVarPreserveWindowFrame);
    const bool force = qEnvironmentVariableIsSet(envVarForceWindowFrame);
    if (should || force) {
        if (force) {
            return true;
        }
        if (should) {
            // If you preserve the window frame on Win7~8.1,
            // the window will have a terrible appearance.
            return isWin10OrGreater();
        }
    }
    return false;
}

bool forceEnableAcrylicOnWin10()
{
    return qEnvironmentVariableIsSet(envVarForceAcrylic);
}

bool isDwmCompositionEnabled()
{
    // Since Win8, DWM composition is always enabled and can't be disabled.
    // In other words, DwmIsCompositionEnabled will always return TRUE on
    // systems newer than Win7.
    BOOL enabled = FALSE;
    return SUCCEEDED(WNEF_EXECUTE_WINAPI_RETURN(DwmIsCompositionEnabled, E_FAIL, &enabled))
           && enabled;
}

QWindow *findWindow(const HWND handle)
{
    Q_ASSERT(handle);
    const auto wid = reinterpret_cast<WId>(handle);
    const QWindowList windows = QGuiApplication::topLevelWindows();
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == wid) {
                return window;
            }
        }
    }
    return nullptr;
}

void triggerFrameChange(const QWindow *window)
{
    Q_ASSERT(window);
    WNEF_EXECUTE_WINAPI(SetWindowPos,
                        reinterpret_cast<HWND>(window->winId()),
                        nullptr,
                        0,
                        0,
                        0,
                        0,
                        SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER
                            | SWP_NOOWNERZORDER)
}

void updateFrameMargins(const QWindow *window, bool reset)
{
    Q_ASSERT(window);
    MARGINS margins = {0, 0, 0, 0};
    if (!reset) {
        margins.cyTopHeight = 1;
    }
    WNEF_EXECUTE_WINAPI(DwmExtendFrameIntoClientArea,
                        reinterpret_cast<HWND>(window->winId()),
                        &margins)
}

// The standard values of border width, border height and title bar height
// when DPI is 96.
const int m_defaultBorderWidth = 8, m_defaultBorderHeight = 8, m_defaultTitleBarHeight = 31;

// The thickness of an auto-hide taskbar in pixels.
const int kAutoHideTaskbarThicknessPx = 2;
const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

const QString g_sDwmRegistryKey = QString::fromUtf8(
    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)");
const QString g_sPersonalizeRegistryKey = QString::fromUtf8(
    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)");

const char m_framelessMode[] = "_WNEF_FRAMELESS_MODE_ENABLED";
const char m_borderWidth[] = "_WNEF_WINDOW_BORDER_WIDTH";
const char m_borderHeight[] = "_WNEF_WINDOW_BORDER_HEIGHT";
const char m_titleBarHeight[] = "_WNEF_TITLE_BAR_HEIGHT";
const char m_ignoredObjects[] = "_WNEF_TITLE_BAR_IGNORED_OBJECTS";

void setup()
{
    if (coreData()->m_instance.isNull()) {
        coreData()->m_instance.reset(new WinNativeEventFilter);
        qApp->installNativeEventFilter(coreData()->m_instance.get());
    }
}

void installHelper(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    window->setProperty(m_framelessMode, enable);
    const int tbh = enable ? WinNativeEventFilter::getSystemMetric(
                        window, WinNativeEventFilter::SystemMetric::TitleBarHeight, true, true)
                           : 0;
    const QMargins margins = {0, -tbh, 0, 0};
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow,
                                                                      QString::fromUtf8(
                                                                          "WindowsCustomMargins"),
                                                                      QVariant::fromValue(margins));
    }
#else
    auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
        window->handle());
    if (platformWindow) {
        platformWindow->setCustomMargins(margins);
    }
#endif
    updateFrameMargins(window, !enable);
    triggerFrameChange(window);
}

} // namespace

WinNativeEventFilter::WinNativeEventFilter() = default;

WinNativeEventFilter::~WinNativeEventFilter()
{
    if (!coreData()->m_instance.isNull()) {
        qApp->removeNativeEventFilter(coreData()->m_instance.get());
    }
}

void WinNativeEventFilter::addFramelessWindow(QWindow *window)
{
    Q_ASSERT(window);
    setup();
    installHelper(window, true);
}

bool WinNativeEventFilter::isWindowFrameless(const QWindow *window)
{
    Q_ASSERT(window);
    return window->property(m_framelessMode).toBool();
}

void WinNativeEventFilter::removeFramelessWindow(QWindow *window)
{
    Q_ASSERT(window);
    installHelper(window, false);
}

void WinNativeEventFilter::setIgnoredObjects(QWindow *window, const QObjectList &objects)
{
    Q_ASSERT(window);
    window->setProperty(m_ignoredObjects, QVariant::fromValue(objects));
}

QObjectList WinNativeEventFilter::getIgnoredObjects(const QWindow *window)
{
    Q_ASSERT(window);
    return qvariant_cast<QObjectList>(window->property(m_ignoredObjects));
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool WinNativeEventFilter::nativeEventFilter(const QByteArray &eventType,
                                             void *message,
                                             qintptr *result)
#else
bool WinNativeEventFilter::nativeEventFilter(const QByteArray &eventType,
                                             void *message,
                                             long *result)
#endif
{
    // "result" can't be null in theory and I don't see any projects check
    // this, everyone is assuming it will never be null, including Microsoft,
    // but according to Lucas, frameless applications crashed on many Win7
    // machines because it's null. The temporary solution is also strange:
    // upgrade drivers or switch to the basic theme.
    if (!result) {
        return false;
    }
    // The example code in Qt's documentation has this check. I don't know
    // whether we really need this check or not, but adding this check won't
    // bring us harm anyway.
    if (eventType != "windows_generic_MSG") {
        return false;
    }
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    // Work-around a bug caused by typo which only exists in Qt 5.11.1
    const auto msg = *reinterpret_cast<MSG **>(message);
#else
    const auto msg = static_cast<LPMSG>(message);
#endif
    if (!msg || (msg && !msg->hwnd)) {
        // Why sometimes the window handle is null? Is it designed to be?
        // Anyway, we should skip it in this case.
        return false;
    }
    const QWindow *window = findWindow(msg->hwnd);
    if (!window || (window && !window->property(m_framelessMode).toBool())) {
        return false;
    }
    switch (msg->message) {
    case WM_NCCALCSIZE: {
        // Windows是根据这个消息的返回值来设置窗口的客户区（窗口中真正显示的内容）
        // 和非客户区（标题栏、窗口边框、菜单栏和状态栏等Windows系统自行提供的部分
        // ，不过对于Qt来说，除了标题栏和窗口边框，非客户区基本也都是自绘的）的范
        // 围的，lParam里存放的就是新客户区的几何区域，默认是整个窗口的大小，正常
        // 的程序需要修改这个参数，告知系统窗口的客户区和非客户区的范围（一般来说可
        // 以完全交给Windows，让其自行处理，使用默认的客户区和非客户区），因此如果
        // 我们不修改lParam，就可以使客户区充满整个窗口，从而去掉标题栏和窗口边框
        // （因为这些东西都被客户区给盖住了。但边框阴影也会因此而丢失，不过我们会使
        // 用其他方式将其带回，请参考其他消息的处理，此处不过多提及）。但有个情况要
        // 特别注意，那就是窗口最大化后，窗口的实际尺寸会比屏幕的尺寸大一点，从而使
        // 用户看不到窗口的边界，这样用户就不能在窗口最大化后调整窗口的大小了（虽然
        // 这个做法听起来特别奇怪，但Windows确实就是这样做的），因此如果我们要自行
        // 处理窗口的非客户区，就要在窗口最大化后，将窗口边框的宽度和高度（一般是相
        // 等的）从客户区裁剪掉，否则我们窗口所显示的内容就会超出屏幕边界，显示不全。
        // 如果用户开启了任务栏自动隐藏，在窗口最大化后，还要考虑任务栏的位置。因为
        // 如果窗口最大化后，其尺寸和屏幕尺寸相等（因为任务栏隐藏了，所以窗口最大化
        // 后其实是充满了整个屏幕，变相的全屏了），Windows会认为窗口已经进入全屏的
        // 状态，从而导致自动隐藏的任务栏无法弹出。要避免这个状况，就要使窗口的尺寸
        // 小于屏幕尺寸。我下面的做法参考了火狐、Chromium和Windows Terminal
        // 如果没有开启任务栏自动隐藏，是不存在这个问题的，所以要先进行判断。
        // 一般情况下，*result设置为0（相当于DefWindowProc的返回值为0）就可以了，
        // 根据MSDN的说法，返回0意为此消息已经被程序自行处理了，让Windows跳过此消
        // 息，否则Windows会添加对此消息的默认处理，对于当前这个消息而言，就意味着
        // 标题栏和窗口边框又会回来，这当然不是我们想要的结果。根据MSDN，当wParam
        // 为FALSE时，只能返回0，但当其为TRUE时，可以返回0，也可以返回一个WVR_常
        // 量。根据Chromium的注释，当存在非客户区时，如果返回WVR_REDRAW会导致子
        // 窗口/子控件出现奇怪的bug（自绘控件错位），并且Lucas在Windows 10
        // 上成功复现，说明这个bug至今都没有解决。我查阅了大量资料，发现唯一的解决
        // 方案就是返回0。但如果不存在非客户区，且wParam为TRUE，最好返回
        // WVR_REDRAW，否则窗口在调整大小可能会产生严重的闪烁现象。
        // 虽然对大多数消息来说，返回0都代表让Windows忽略此消息，但实际上不同消息
        // 能接受的返回值是不一样的，请注意自行查阅MSDN。

        // Sent when the size and position of a window's client area must be
        // calculated. By processing this message, an application can
        // control the content of the window's client area when the size or
        // position of the window changes. If wParam is TRUE, lParam points
        // to an NCCALCSIZE_PARAMS structure that contains information an
        // application can use to calculate the new size and position of the
        // client rectangle. If wParam is FALSE, lParam points to a RECT
        // structure. On entry, the structure contains the proposed window
        // rectangle for the window. On exit, the structure should contain
        // the screen coordinates of the corresponding window client area.
        // The client area is the window's content area, the non-client area
        // is the area which is provided by the system, such as the title
        // bar, the four window borders, the frame shadow, the menu bar, the
        // status bar, the scroll bar, etc. But for Qt, it draws most of the
        // window area (client + non-client) itself. We now know that the
        // title bar and the window frame is in the non-client area and we
        // can set the scope of the client area in this message, so we can
        // remove the title bar and the window frame by let the non-client
        // area be covered by the client area (because we can't really get
        // rid of the non-client area, it will always be there, all we can
        // do is to hide it) , which means we should let the client area's
        // size the same with the whole window's size. So there is no room
        // for the non-client area and then the user won't be able to see it
        // again. But how to achieve this? Very easy, just leave lParam (the
        // re-calculated client area) untouched. But of course you can
        // modify lParam, then the non-client area will be seen and the
        // window borders and the window frame will show up. However, things
        // are quite different when you try to modify the top margin of the
        // client area. DWM will always draw the whole title bar no matter
        // what margin value you set for the top, unless you don't modify it
        // and remove the whole top area (the title bar + the one pixel
        // height window border). This can be confirmed in Windows
        // Terminal's source code, you can also try yourself to verify
        // it. So things will become quite complicated if you want to
        // preserve the four window borders. So we just remove the whole
        // window frame, otherwise the code will become much more complex.

        if (shouldUseNativeTitleBar()) {
            break;
        }

        if (!msg->wParam) {
            *result = 0;
            return true;
        }
        bool nonclient = false;
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam)->rgrc[0]);
        if (shouldHaveWindowFrame()) {
            // Store the original top before the default window proc
            // applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = WNEF_EXECUTE_WINAPI_RETURN(DefWindowProcW,
                                                           0,
                                                           msg->hwnd,
                                                           WM_NCCALCSIZE,
                                                           msg->wParam,
                                                           msg->lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the
            // default frame was applied.
            clientRect->top = originalTop;
        }
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (IsMaximized(msg->hwnd) && !(window->windowState() & Qt::WindowFullScreen)) {
            // Windows automatically adds a standard width border to all
            // sides when a window is maximized. We have to remove it
            // otherwise the content of our window will be cut-off from
            // the screen.
            // The value of border width and border height should be
            // identical in most cases, when the scale factor is 1.0, it
            // should be eight pixels.
            const int bh = getSystemMetric(window, SystemMetric::BorderHeight, true);
            clientRect->top += bh;
            if (!shouldHaveWindowFrame()) {
                clientRect->bottom -= bh;
                const int bw = getSystemMetric(window, SystemMetric::BorderWidth, true);
                clientRect->left += bw;
                clientRect->right -= bw;
            }
            nonclient = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (IsMaximized(msg->hwnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = WNEF_EXECUTE_WINAPI_RETURN(SHAppBarMessage,
                                                                 0,
                                                                 ABM_GETSTATE,
                                                                 &abd);
            // First, check if we have an auto-hide taskbar at all:
            if (taskbarState & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to ABM_GETAUTOHIDEBAREX only exists from Win8.1,
                // we have to use another way to judge this if we are
                // running on Windows 7 or Windows 8.
                if (isWin8Point1OrGreater()) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    const HMONITOR monitor = WNEF_EXECUTE_WINAPI_RETURN(MonitorFromWindow,
                                                                        nullptr,
                                                                        msg->hwnd,
                                                                        MONITOR_DEFAULTTONEAREST);
                    WNEF_EXECUTE_WINAPI(GetMonitorInfoW, monitor, &monitorInfo)
                    // This helper can be used to determine if there's a
                    // auto-hide taskbar on the given edge of the monitor
                    // we're currently on.
                    const auto hasAutohideTaskbar = [&monitorInfo](const UINT edge) -> bool {
                        APPBARDATA _abd;
                        SecureZeroMemory(&_abd, sizeof(_abd));
                        _abd.cbSize = sizeof(_abd);
                        _abd.uEdge = edge;
                        _abd.rc = monitorInfo.rcMonitor;
                        const auto hTaskbar = reinterpret_cast<HWND>(
                            WNEF_EXECUTE_WINAPI_RETURN(SHAppBarMessage,
                                                       0,
                                                       ABM_GETAUTOHIDEBAREX,
                                                       &_abd));
                        return hTaskbar != nullptr;
                    };
                    top = hasAutohideTaskbar(ABE_TOP);
                    bottom = hasAutohideTaskbar(ABE_BOTTOM);
                    left = hasAutohideTaskbar(ABE_LEFT);
                    right = hasAutohideTaskbar(ABE_RIGHT);
                } else {
                    // The following code is copied from Mozilla Firefox,
                    // with some modifications.
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = WNEF_EXECUTE_WINAPI_RETURN(FindWindowW,
                                                           nullptr,
                                                           L"Shell_TrayWnd",
                                                           nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor
                            = WNEF_EXECUTE_WINAPI_RETURN(MonitorFromWindow,
                                                         nullptr,
                                                         msg->hwnd,
                                                         MONITOR_DEFAULTTONEAREST);
                        const HMONITOR taskbarMonitor
                            = WNEF_EXECUTE_WINAPI_RETURN(MonitorFromWindow,
                                                         nullptr,
                                                         _abd.hWnd,
                                                         MONITOR_DEFAULTTOPRIMARY);
                        if (taskbarMonitor == windowMonitor) {
                            WNEF_EXECUTE_WINAPI(SHAppBarMessage, ABM_GETTASKBARPOS, &_abd)
                            edge = _abd.uEdge;
                        }
                    }
                    top = edge == ABE_TOP;
                    bottom = edge == ABE_BOTTOM;
                    left = edge == ABE_LEFT;
                    right = edge == ABE_RIGHT;
                }
                // If there's a taskbar on any side of the monitor, reduce
                // our size a little bit on that edge.
                // Note to future code archeologists:
                // This doesn't seem to work for fullscreen on the primary
                // display. However, testing a bunch of other apps with
                // fullscreen modes and an auto-hiding taskbar has
                // shown that _none_ of them reveal the taskbar from
                // fullscreen mode. This includes Edge, Firefox, Chrome,
                // Sublime Text, PowerPoint - none seemed to support this.
                // This does however work fine for maximized.
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += kAutoHideTaskbarThicknessPy;
                    nonclient = true;
                } else if (bottom) {
                    clientRect->bottom -= kAutoHideTaskbarThicknessPy;
                    nonclient = true;
                } else if (left) {
                    clientRect->left += kAutoHideTaskbarThicknessPx;
                    nonclient = true;
                } else if (right) {
                    clientRect->right -= kAutoHideTaskbarThicknessPx;
                    nonclient = true;
                }
            }
        }
        // If the window bounds change, we're going to relayout and repaint
        // anyway. Returning WVR_REDRAW avoids an extra paint before that of
        // the old client pixels in the (now wrong) location, and thus makes
        // actions like resizing a window from the left edge look slightly
        // less broken.
        //
        // We cannot return WVR_REDRAW when there is nonclient area, or
        // Windows exhibits bugs where client pixels and child HWNDs are
        // mispositioned by the width/height of the upper-left nonclient
        // area.
        *result = nonclient ? 0 : WVR_REDRAW;
        return true;
    }
    // These undocumented messages are sent to draw themed window
    // borders. Block them to prevent drawing borders over the client
    // area.
    case WM_NCUAHDRAWCAPTION:
    case WM_NCUAHDRAWFRAME: {
        if (shouldHaveWindowFrame()) {
            break;
        } else {
            *result = 0;
            return true;
        }
    }
    case WM_NCPAINT: {
        // 边框阴影处于非客户区的范围，因此如果直接阻止非客户区的绘制，会导致边框阴影丢失

        if (!isDwmCompositionEnabled() && !shouldHaveWindowFrame()) {
            // Only block WM_NCPAINT when DWM composition is disabled. If
            // it's blocked when DWM composition is enabled, the frame
            // shadow won't be drawn.
            *result = 0;
            return true;
        } else {
            break;
        }
    }
    case WM_NCACTIVATE: {
        if (shouldHaveWindowFrame()) {
            break;
        } else {
            if (isDwmCompositionEnabled()) {
                // DefWindowProc won't repaint the window border if lParam
                // (normally a HRGN) is -1. See the following link's "lParam"
                // section:
                // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
                // Don't use "*result = 0" otherwise the window won't respond
                // to the window active state change.
                *result = WNEF_EXECUTE_WINAPI_RETURN(DefWindowProcW,
                                                     0,
                                                     msg->hwnd,
                                                     msg->message,
                                                     msg->wParam,
                                                     -1);
            } else {
                if (static_cast<BOOL>(msg->wParam)) {
                    *result = FALSE;
                } else {
                    *result = TRUE;
                }
            }
            return true;
        }
    }
    case WM_NCHITTEST: {
        // 原生Win32窗口只有顶边是在窗口内部resize的，其余三边都是在窗口
        // 外部进行resize的，其原理是，WS_THICKFRAME这个窗口样式会在窗
        // 口的左、右和底边添加三个透明的resize区域，这三个区域在正常状态
        // 下是完全不可见的，它们由DWM负责绘制和控制。这些区域的宽度等于
        // (SM_CXSIZEFRAME + SM_CXPADDEDBORDER)，高度等于
        // (SM_CYSIZEFRAME + SM_CXPADDEDBORDER)，在100%缩放时，均等
        // 于8像素。它们属于窗口区域的一部分，但不属于客户区，而是属于非客
        // 户区，因此GetWindowRect获取的区域中是包含这三个resize区域的，
        // 而GetClientRect获取的区域是不包含它们的。当把
        // DWMWA_EXTENDED_FRAME_BOUNDS作为参数调用
        // DwmGetWindowAttribute时，也能获取到一个窗口大小，这个大小介
        // 于前面两者之间，暂时不知道这个数据的意义及其作用。我们在
        // WM_NCCALCSIZE消息的处理中，已经把整个窗口都设置为客户区了，也
        // 就是说，我们的窗口已经没有非客户区了，因此那三个透明的resize区
        // 域，此刻也已经成为窗口客户区的一部分了，从而变得不透明了。所以
        // 现在的resize，看起来像是在窗口内部resize，是因为原本透明的地方
        // 现在变得不透明了，实际上，单纯从范围上来看，现在我们resize的地方，
        // 就是普通窗口的边框外部，那三个透明区域的范围。
        // 因此，如果我们把边框完全去掉（就是我们正在做的事情），resize就
        // 会看起来是在内部进行，这个问题通过常规方法非常难以解决。我测试过
        // QQ和钉钉的窗口，它们的窗口就是在外部resize，但实际上它们是通过
        // 把窗口实际的内容，嵌入到一个完全透明的但尺寸要大一圈的窗口中实现
        // 的，虽然看起来效果还行，但在我看来不是正途。而且我之所以能发现，
        // 也是由于这种方法在很多情况下会露馅，比如窗口未响应卡住或贴边的时
        // 候，能明显看到窗口周围多出来一圈边界。我曾经尝试再把那三个区域弄
        // 透明，但无一例外都会破坏DWM绘制的边框阴影，因此只好作罢。

        // As you may have found, if you use this code, the resize areas
        // will be inside the frameless window, however, a normal Win32
        // window can be resized outside of it. Here is the reason: the
        // WS_THICKFRAME window style will cause a window has three
        // transparent areas beside the window's left, right and bottom
        // edge. Their width or height is eight pixels if the window is not
        // scaled. In most cases, they are totally invisible. It's DWM's
        // responsibility to draw and control them. They exist to let the
        // user resize the window, visually outside of it. They are in the
        // window area, but not the client area, so they are in the
        // non-client area actually. But we have turned the whole window
        // area into client area in WM_NCCALCSIZE, so the three transparent
        // resize areas also become a part of the client area and thus they
        // become visible. When we resize the window, it looks like we are
        // resizing inside of it, however, that's because the transparent
        // resize areas are visible now, we ARE resizing outside of the
        // window actually. But I don't know how to make them become
        // transparent again without breaking the frame shadow drawn by DWM.
        // If you really want to solve it, you can try to embed your window
        // into a larger transparent window and draw the frame shadow
        // yourself. As what we have said in WM_NCCALCSIZE, you can only
        // remove the top area of the window, this will let us be able to
        // resize outside of the window and don't need much process in this
        // message, it looks like a perfect plan, however, the top border is
        // missing due to the whole top area is removed, and it's very hard
        // to bring it back because we have to use a trick in WM_PAINT
        // (learned from Windows Terminal), but no matter what we do in
        // WM_PAINT, it will always break the backing store mechanism of Qt,
        // so actually we can't do it. And it's very difficult to do such
        // things in NativeEventFilters as well. What's worse, if we really
        // do this, the four window borders will become white and they look
        // horrible in dark mode. This solution only supports Windows 10
        // because the border width on Win10 is only one pixel, however it's
        // eight pixels on Windows 7 so preserving the three window borders
        // looks terrible on old systems. I'm testing this solution in
        // another branch, if you are interested in it, you can give it a
        // try.

        if (shouldUseNativeTitleBar()) {
            break;
        }

        const auto isInSpecificObjects =
            [](const QPointF &mousePos, const QObjectList &objects, const qreal dpr) -> bool {
            if (objects.isEmpty()) {
                return false;
            }
            for (auto &&object : qAsConst(objects)) {
                if (!object) {
                    continue;
                }
                if (!object->isWidgetType() && !object->inherits("QQuickItem")) {
                    qWarning() << object << "is not a QWidget or QQuickItem!";
                    continue;
                }
                if (!object->property("visible").toBool()) {
                    qDebug() << "Skipping invisible object" << object;
                    continue;
                }
                const auto mapOriginPointToWindow = [](const QObject *obj) -> QPointF {
                    Q_ASSERT(obj);
                    QPointF point = {obj->property("x").toReal(), obj->property("y").toReal()};
                    for (QObject *parent = obj->parent(); parent; parent = parent->parent()) {
                        point += {parent->property("x").toReal(), parent->property("y").toReal()};
                    }
                    return point;
                };
                const QPointF originPoint = mapOriginPointToWindow(object);
                const qreal width = object->property("width").toReal();
                const qreal height = object->property("height").toReal();
                if (QRectF(originPoint.x() * dpr, originPoint.y() * dpr, width * dpr, height * dpr)
                        .contains(mousePos)) {
                    return true;
                }
            }
            return false;
        };
        const qreal dpr = window->devicePixelRatio();
        const QPointF globalMouse = QCursor::pos(window->screen()) * dpr;
        POINT winLocalMouse = {qRound(globalMouse.x()), qRound(globalMouse.y())};
        WNEF_EXECUTE_WINAPI(ScreenToClient, msg->hwnd, &winLocalMouse)
        const QPointF localMouse = {static_cast<qreal>(winLocalMouse.x),
                                    static_cast<qreal>(winLocalMouse.y)};
        const bool isInIgnoreObjects = isInSpecificObjects(globalMouse,
                                                           qvariant_cast<QObjectList>(
                                                               window->property(m_ignoredObjects)),
                                                           dpr);
        const int bh = getSystemMetric(window, SystemMetric::BorderHeight, true);
        const int tbh = getSystemMetric(window, SystemMetric::TitleBarHeight, true);
        const bool isTitleBar = (localMouse.y() <= tbh) && !isInIgnoreObjects;
        const bool isTop = localMouse.y() <= bh;
        if (shouldHaveWindowFrame()) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = WNEF_EXECUTE_WINAPI_RETURN(DefWindowProcW,
                                                                   0,
                                                                   msg->hwnd,
                                                                   WM_NCHITTEST,
                                                                   msg->wParam,
                                                                   msg->lParam);
            if (originalRet != HTCLIENT) {
                *result = originalRet;
                return true;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (!IsMaximized(msg->hwnd) && isTop) {
                *result = HTTOP;
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        } else {
            const auto getHitTestResult =
                [msg, isTitleBar, &localMouse, bh, isTop, window]() -> LRESULT {
                RECT clientRect = {0, 0, 0, 0};
                WNEF_EXECUTE_WINAPI(GetClientRect, msg->hwnd, &clientRect)
                const LONG ww = clientRect.right;
                const LONG wh = clientRect.bottom;
                const int bw = getSystemMetric(window, SystemMetric::BorderWidth, true);
                if (IsMaximized(msg->hwnd)) {
                    if (isTitleBar) {
                        return HTCAPTION;
                    }
                    return HTCLIENT;
                }
                const bool isBottom = (localMouse.y() >= (wh - bh));
                // Make the border a little wider to let the user easy to resize
                // on corners.
                const int factor = (isTop || isBottom) ? 2 : 1;
                const bool isLeft = (localMouse.x() <= (bw * factor));
                const bool isRight = (localMouse.x() >= (ww - (bw * factor)));
                const bool fixedSize = [window] {
                    if (window->flags().testFlag(Qt::MSWindowsFixedSizeDialogHint)) {
                        return true;
                    }
                    const QSize minSize = window->minimumSize();
                    const QSize maxSize = window->maximumSize();
                    if (!minSize.isEmpty() && !maxSize.isEmpty() && minSize == maxSize) {
                        return true;
                    }
                    return false;
                }();
                const auto getBorderValue = [fixedSize](int value) -> int {
                    // HTBORDER: non-resizable window border.
                    return fixedSize ? HTBORDER : value;
                };
                if (isTop) {
                    if (isLeft) {
                        return getBorderValue(HTTOPLEFT);
                    }
                    if (isRight) {
                        return getBorderValue(HTTOPRIGHT);
                    }
                    return getBorderValue(HTTOP);
                }
                if (isBottom) {
                    if (isLeft) {
                        return getBorderValue(HTBOTTOMLEFT);
                    }
                    if (isRight) {
                        return getBorderValue(HTBOTTOMRIGHT);
                    }
                    return getBorderValue(HTBOTTOM);
                }
                if (isLeft) {
                    return getBorderValue(HTLEFT);
                }
                if (isRight) {
                    return getBorderValue(HTRIGHT);
                }
                if (isTitleBar) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            };
            *result = getHitTestResult();
            return true;
        }
    }
    case WM_SETICON:
    case WM_SETTEXT: {
        if (shouldUseNativeTitleBar()) {
            break;
        }

        // Disable painting while these messages are handled to prevent them
        // from drawing a window caption over the client area.
        const auto oldStyle = WNEF_EXECUTE_WINAPI_RETURN(GetWindowLongPtrW, 0, msg->hwnd, GWL_STYLE);
        // Prevent Windows from drawing the default title bar by temporarily
        // toggling the WS_VISIBLE style.
        WNEF_EXECUTE_WINAPI(SetWindowLongPtrW, msg->hwnd, GWL_STYLE, oldStyle & ~WS_VISIBLE)
        triggerFrameChange(window);
        const LRESULT ret = WNEF_EXECUTE_WINAPI_RETURN(DefWindowProcW,
                                                       0,
                                                       msg->hwnd,
                                                       msg->message,
                                                       msg->wParam,
                                                       msg->lParam);
        WNEF_EXECUTE_WINAPI(SetWindowLongPtrW, msg->hwnd, GWL_STYLE, oldStyle)
        triggerFrameChange(window);
        *result = ret;
        return true;
    }
    default:
        break;
    }
    return false;
}

void WinNativeEventFilter::setBorderWidth(QWindow *window, const int bw)
{
    Q_ASSERT(window);
    window->setProperty(m_borderWidth, bw);
}

void WinNativeEventFilter::setBorderHeight(QWindow *window, const int bh)
{
    Q_ASSERT(window);
    window->setProperty(m_borderHeight, bh);
}

void WinNativeEventFilter::setTitleBarHeight(QWindow *window, const int tbh)
{
    Q_ASSERT(window);
    window->setProperty(m_titleBarHeight, tbh);
}

int WinNativeEventFilter::getSystemMetric(const QWindow *window,
                                          const SystemMetric metric,
                                          const bool dpiAware,
                                          const bool forceSystemValue)
{
    Q_ASSERT(window);
    const qreal dpr = dpiAware ? window->devicePixelRatio() : m_defaultDevicePixelRatio;
    const auto getSystemMetricsForWindow = [dpr](const int index, const bool dpiAware) -> int {
        if (coreData()->m_lpGetSystemMetricsForDpi) {
            const quint32 dpi = dpiAware ? qRound(m_defaultDotsPerInch * dpr)
                                         : m_defaultDotsPerInch;
            return coreData()->m_lpGetSystemMetricsForDpi(index, dpi);
        } else {
            const int value = WNEF_EXECUTE_WINAPI_RETURN(GetSystemMetrics, 0, index);
            return dpiAware ? value : qRound(value / dpr);
        }
    };
    int ret = 0;
    switch (metric) {
    case SystemMetric::BorderWidth: {
        const int bw = window->property(m_borderWidth).toInt();
        if ((bw > 0) && !forceSystemValue) {
            ret = qRound(bw * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CXSIZEFRAME, false)
                                      + getSystemMetricsForWindow(SM_CXPADDEDBORDER, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CXSIZEFRAME, true)
                                   + getSystemMetricsForWindow(SM_CXPADDEDBORDER, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(m_defaultBorderWidth * dpr);
        }
    } break;
    case SystemMetric::BorderHeight: {
        const int bh = window->property(m_borderHeight).toInt();
        if ((bh > 0) && !forceSystemValue) {
            ret = qRound(bh * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CYSIZEFRAME, false)
                                      + getSystemMetricsForWindow(SM_CXPADDEDBORDER, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CYSIZEFRAME, true)
                                   + getSystemMetricsForWindow(SM_CXPADDEDBORDER, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(m_defaultBorderHeight * dpr);
        }
    } break;
    case SystemMetric::TitleBarHeight: {
        const int tbh = window->property(m_titleBarHeight).toInt();
        if ((tbh > 0) && !forceSystemValue) {
            // Special case: this is the user defined value,
            // don't change it and just return it untouched.
            return qRound(tbh * dpr);
        } else {
            const int result_nondpi = getSystemMetricsForWindow(SM_CYCAPTION, false);
            const int result_dpi = getSystemMetricsForWindow(SM_CYCAPTION, true);
            const int result = dpiAware ? result_dpi : result_nondpi;
            ret = result > 0 ? result : qRound(m_defaultTitleBarHeight * dpr);
        }
    } break;
    }
    // When dpr = 1.0 (DPI = 96):
    // SM_CXSIZEFRAME = SM_CYSIZEFRAME = 4px
    // SM_CXPADDEDBORDER = 4px
    // SM_CYCAPTION = 23px
    // Border Width = Border Height = SM_C(X|Y)SIZEFRAME + SM_CXPADDEDBORDER = 8px
    // Title Bar Height = Border Height + SM_CYCAPTION = 31px
    // dpr = 1.25 --> Title Bar Height = 38px
    // dpr = 1.5 --> Title Bar Height = 45px
    // dpr = 1.75 --> Title Bar Height = 51px
    ret += (metric == SystemMetric::TitleBarHeight)
               ? getSystemMetric(window, SystemMetric::BorderHeight, dpiAware)
               : 0;
    return ret;
}

bool WinNativeEventFilter::setBlurEffectEnabled(const QWindow *window,
                                                const bool enabled,
                                                const QColor &gradientColor)
{
    Q_ASSERT(window);
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (isWin8OrGreater() && coreData()->m_lpSetWindowCompositionAttribute) {
        ACCENT_POLICY accentPolicy;
        SecureZeroMemory(&accentPolicy, sizeof(accentPolicy));
        WINDOWCOMPOSITIONATTRIBDATA wcaData;
        SecureZeroMemory(&wcaData, sizeof(wcaData));
        wcaData.Attrib = WCA_ACCENT_POLICY;
        wcaData.pvData = &accentPolicy;
        wcaData.cbData = sizeof(accentPolicy);
        if (enabled) {
            if (isWin10OrGreater(17134)) {
                // Windows 10, version 1803 (10.0.17134)
                // It's not allowed to enable the Acrylic effect for Win32
                // applications until Win10 1803.
                if (forceEnableAcrylicOnWin10()) {
                    accentPolicy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                    // The gradient color must be set otherwise it'll look
                    // like a classic blur. Use semi-transparent gradient
                    // color to get better appearance.
                    const QColor color = gradientColor.isValid() ? gradientColor : Qt::white;
                    accentPolicy.GradientColor = qRgba(color.blue(),
                                                       color.green(),
                                                       color.red(),
                                                       color.alpha());
                } else {
                    // Enabling the Acrylic effect for Win32 windows is
                    // very buggy and it's a bug of Windows 10 itself so
                    // it's not fixable from my side.
                    // So here we switch back to use the classic blur as
                    // a workaround.
                    accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
                }
            } else if (isWin10OrGreater(10240)) {
                // Windows 10, version 1507 (10.0.10240)
                // The initial version of Win10.
                accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
            } else {
                // Windows 8 and 8.1.
                accentPolicy.AccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
            }
        } else {
            accentPolicy.AccentState = ACCENT_DISABLED;
        }
        return coreData()->m_lpSetWindowCompositionAttribute(hwnd, &wcaData);
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = enabled ? TRUE : FALSE;
        return SUCCEEDED(
            WNEF_EXECUTE_WINAPI_RETURN(DwmEnableBlurBehindWindow, E_FAIL, hwnd, &dwmBB));
    }
}

bool WinNativeEventFilter::isColorizationEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_sDwmRegistryKey, QSettings::NativeFormat);
    const bool colorPrevalence
        = registry.value(QString::fromUtf8("ColorPrevalence"), 0).toULongLong(&ok) != 0;
    return (ok && colorPrevalence);
}

QColor WinNativeEventFilter::getColorizationColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    return SUCCEEDED(
               WNEF_EXECUTE_WINAPI_RETURN(DwmGetColorizationColor, E_FAIL, &color, &opaqueBlend))
               ? QColor::fromRgba(color)
               : Qt::white;
}

bool WinNativeEventFilter::isLightThemeEnabled()
{
    return !isDarkThemeEnabled();
}

bool WinNativeEventFilter::isDarkThemeEnabled()
{
    return coreData()->m_lpShouldSystemUseDarkMode ? coreData()->m_lpShouldSystemUseDarkMode()
                                                   : false;
}

bool WinNativeEventFilter::isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    return WNEF_EXECUTE_WINAPI_RETURN(SystemParametersInfoW, FALSE, SPI_GETHIGHCONTRAST, 0, &hc, 0)
               ? (hc.dwFlags & HCF_HIGHCONTRASTON)
               : false;
}

bool WinNativeEventFilter::isDarkFrameEnabled(const QWindow *window)
{
    Q_ASSERT(window);
    if (!isWin10OrGreater(17763)) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (WNEF_EXECUTE_WINAPI_RETURN(IsWindow, FALSE, hwnd)) {
        BOOL result = FALSE;
        const bool ok = SUCCEEDED(WNEF_EXECUTE_WINAPI_RETURN(DwmGetWindowAttribute,
                                                             E_FAIL,
                                                             hwnd,
                                                             DwmwaUseImmersiveDarkMode,
                                                             &result,
                                                             sizeof(result)))
                        || SUCCEEDED(WNEF_EXECUTE_WINAPI_RETURN(DwmGetWindowAttribute,
                                                                E_FAIL,
                                                                hwnd,
                                                                DwmwaUseImmersiveDarkModeBefore20h1,
                                                                &result,
                                                                sizeof(result)));
        return (ok && result);
    }
    return false;
}

bool WinNativeEventFilter::isTransparencyEffectEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_sPersonalizeRegistryKey, QSettings::NativeFormat);
    const bool enableTransparency
        = registry.value(QString::fromUtf8("EnableTransparency"), 0).toULongLong(&ok) != 0;
    return (ok && enableTransparency);
}
