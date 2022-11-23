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

#include <windows.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cwctype>
#include <algorithm>
#include <clocale>

#ifndef SM_CYPADDEDBORDER
#  define SM_CYPADDEDBORDER SM_CXPADDEDBORDER
#endif

#define DECLARE_DPI_ENTRY(DPR) \
    UINT(std::round(double(USER_DEFAULT_SCREEN_DPI) * double(DPR))), double(DPR)

static constexpr const wchar_t WINDOW_CLASS_NAME[] = L"org.wangwenx190.DpiTester.WindowClass\0";
static constexpr const wchar_t DEFAULT_STRUCT_NAME[] = L"SYSTEM_METRIC";
static constexpr const wchar_t DEFAULT_VARIABLE_NAME[] = L"SYSTEM_METRIC_TABLE";
static constexpr const wchar_t HASH_STD_NAME[] = L"std::unordered_map";
static constexpr const wchar_t HASH_QT_NAME[] = L"QHash";

static constexpr const wchar_t OPT_STRUCT_NAME[] = L"struct-name";
static constexpr const wchar_t OPT_VARIABLE_NAME[] = L"variable-name";
static constexpr const wchar_t OPT_ENABLE_QT[] = L"enable-qt";

static const std::unordered_map<std::wstring, int> SYSTEM_METRIC_TABLE = {
    {L"SM_CXSCREEN", SM_CXSCREEN},
    {L"SM_CYSCREEN", SM_CYSCREEN},
    {L"SM_CXVSCROLL", SM_CXVSCROLL},
    {L"SM_CYHSCROLL", SM_CYHSCROLL},
    {L"SM_CYCAPTION", SM_CYCAPTION},
    {L"SM_CXBORDER", SM_CXBORDER},
    {L"SM_CYBORDER", SM_CYBORDER},
    {L"SM_CXDLGFRAME", SM_CXDLGFRAME},
    {L"SM_CYDLGFRAME", SM_CYDLGFRAME},
    {L"SM_CYVTHUMB", SM_CYVTHUMB},
    {L"SM_CXHTHUMB", SM_CXHTHUMB},
    {L"SM_CXICON", SM_CXICON},
    {L"SM_CYICON", SM_CYICON},
    {L"SM_CXCURSOR", SM_CXCURSOR},
    {L"SM_CYCURSOR", SM_CYCURSOR},
    {L"SM_CYMENU", SM_CYMENU},
    {L"SM_CXFULLSCREEN", SM_CXFULLSCREEN},
    {L"SM_CYFULLSCREEN", SM_CYFULLSCREEN},
    {L"SM_CYKANJIWINDOW", SM_CYKANJIWINDOW},
    {L"SM_MOUSEPRESENT", SM_MOUSEPRESENT},
    {L"SM_CYVSCROLL", SM_CYVSCROLL},
    {L"SM_CXHSCROLL", SM_CXHSCROLL},
    {L"SM_DEBUG", SM_DEBUG},
    {L"SM_SWAPBUTTON", SM_SWAPBUTTON},
    {L"SM_RESERVED1", SM_RESERVED1},
    {L"SM_RESERVED2", SM_RESERVED2},
    {L"SM_RESERVED3", SM_RESERVED3},
    {L"SM_RESERVED4", SM_RESERVED4},
    {L"SM_CXMIN", SM_CXMIN},
    {L"SM_CYMIN", SM_CYMIN},
    {L"SM_CXSIZE", SM_CXSIZE},
    {L"SM_CYSIZE", SM_CYSIZE},
    {L"SM_CXFRAME", SM_CXFRAME},
    {L"SM_CYFRAME", SM_CYFRAME},
    {L"SM_CXMINTRACK", SM_CXMINTRACK},
    {L"SM_CYMINTRACK", SM_CYMINTRACK},
    {L"SM_CXDOUBLECLK", SM_CXDOUBLECLK},
    {L"SM_CYDOUBLECLK", SM_CYDOUBLECLK},
    {L"SM_CXICONSPACING", SM_CXICONSPACING},
    {L"SM_CYICONSPACING", SM_CYICONSPACING},
    {L"SM_MENUDROPALIGNMENT", SM_MENUDROPALIGNMENT},
    {L"SM_PENWINDOWS", SM_PENWINDOWS},
    {L"SM_DBCSENABLED", SM_DBCSENABLED},
    {L"SM_CMOUSEBUTTONS", SM_CMOUSEBUTTONS},
    {L"SM_CXFIXEDFRAME", SM_CXFIXEDFRAME},
    {L"SM_CYFIXEDFRAME", SM_CYFIXEDFRAME},
    {L"SM_CXSIZEFRAME", SM_CXSIZEFRAME},
    {L"SM_CYSIZEFRAME", SM_CYSIZEFRAME},
    {L"SM_SECURE", SM_SECURE},
    {L"SM_CXEDGE", SM_CXEDGE},
    {L"SM_CYEDGE", SM_CYEDGE},
    {L"SM_CXMINSPACING", SM_CXMINSPACING},
    {L"SM_CYMINSPACING", SM_CYMINSPACING},
    {L"SM_CXSMICON", SM_CXSMICON},
    {L"SM_CYSMICON", SM_CYSMICON},
    {L"SM_CYSMCAPTION", SM_CYSMCAPTION},
    {L"SM_CXSMSIZE", SM_CXSMSIZE},
    {L"SM_CYSMSIZE", SM_CYSMSIZE},
    {L"SM_CXMENUSIZE", SM_CXMENUSIZE},
    {L"SM_CYMENUSIZE", SM_CYMENUSIZE},
    {L"SM_ARRANGE", SM_ARRANGE},
    {L"SM_CXMINIMIZED", SM_CXMINIMIZED},
    {L"SM_CYMINIMIZED", SM_CYMINIMIZED},
    {L"SM_CXMAXTRACK", SM_CXMAXTRACK},
    {L"SM_CYMAXTRACK", SM_CYMAXTRACK},
    {L"SM_CXMAXIMIZED", SM_CXMAXIMIZED},
    {L"SM_CYMAXIMIZED", SM_CYMAXIMIZED},
    {L"SM_NETWORK", SM_NETWORK},
    {L"SM_CLEANBOOT", SM_CLEANBOOT},
    {L"SM_CXDRAG", SM_CXDRAG},
    {L"SM_CYDRAG", SM_CYDRAG},
    {L"SM_SHOWSOUNDS", SM_SHOWSOUNDS},
    {L"SM_CXMENUCHECK", SM_CXMENUCHECK},
    {L"SM_CYMENUCHECK", SM_CYMENUCHECK},
    {L"SM_SLOWMACHINE", SM_SLOWMACHINE},
    {L"SM_MIDEASTENABLED", SM_MIDEASTENABLED},
    {L"SM_MOUSEWHEELPRESENT", SM_MOUSEWHEELPRESENT},
    {L"SM_XVIRTUALSCREEN", SM_XVIRTUALSCREEN},
    {L"SM_YVIRTUALSCREEN", SM_YVIRTUALSCREEN},
    {L"SM_CXVIRTUALSCREEN", SM_CXVIRTUALSCREEN},
    {L"SM_CYVIRTUALSCREEN", SM_CYVIRTUALSCREEN},
    {L"SM_CMONITORS", SM_CMONITORS},
    {L"SM_SAMEDISPLAYFORMAT", SM_SAMEDISPLAYFORMAT},
    {L"SM_IMMENABLED", SM_IMMENABLED},
    {L"SM_CXFOCUSBORDER", SM_CXFOCUSBORDER},
    {L"SM_CYFOCUSBORDER", SM_CYFOCUSBORDER},
    {L"SM_TABLETPC", SM_TABLETPC},
    {L"SM_MEDIACENTER", SM_MEDIACENTER},
    {L"SM_STARTER", SM_STARTER},
    {L"SM_SERVERR2", SM_SERVERR2},
    {L"SM_MOUSEHORIZONTALWHEELPRESENT", SM_MOUSEHORIZONTALWHEELPRESENT},
    {L"SM_CXPADDEDBORDER", SM_CXPADDEDBORDER},
    {L"SM_CYPADDEDBORDER", SM_CYPADDEDBORDER},
    {L"SM_DIGITIZER", SM_DIGITIZER},
    {L"SM_MAXIMUMTOUCHES", SM_MAXIMUMTOUCHES},
    {L"SM_CMETRICS", SM_CMETRICS},
    {L"SM_REMOTESESSION", SM_REMOTESESSION},
    {L"SM_SHUTTINGDOWN", SM_SHUTTINGDOWN},
    {L"SM_REMOTECONTROL", SM_REMOTECONTROL},
    {L"SM_CARETBLINKINGENABLED", SM_CARETBLINKINGENABLED},
    {L"SM_CONVERTIBLESLATEMODE", SM_CONVERTIBLESLATEMODE},
    {L"SM_SYSTEMDOCKED", SM_SYSTEMDOCKED}
};

static const struct {
    const UINT DotsPerInch = 0;
    const double DevicePixelRatio = 0.0;
} DPI_TABLE[] = {
    DECLARE_DPI_ENTRY(1.00), // 100%
    DECLARE_DPI_ENTRY(1.20), // 120%
    DECLARE_DPI_ENTRY(1.25), // 125%
    DECLARE_DPI_ENTRY(1.40), // 140%
    DECLARE_DPI_ENTRY(1.50), // 150%
    DECLARE_DPI_ENTRY(1.60), // 160%
    DECLARE_DPI_ENTRY(1.75), // 175%
    DECLARE_DPI_ENTRY(1.80), // 180%
    DECLARE_DPI_ENTRY(2.00), // 200%
    DECLARE_DPI_ENTRY(2.25), // 225%
    DECLARE_DPI_ENTRY(2.50), // 250%
    DECLARE_DPI_ENTRY(3.00), // 300%
    DECLARE_DPI_ENTRY(3.50), // 350%
    DECLARE_DPI_ENTRY(4.00), // 400%
    DECLARE_DPI_ENTRY(4.50), // 450%
    DECLARE_DPI_ENTRY(5.00)  // 500%
};
static const auto DPI_COUNT = int(std::size(DPI_TABLE));

struct HiDPI
{
    static inline decltype(&::GetDpiForWindow) GetDpiForWindowPtr = nullptr;
    static inline decltype(&::GetSystemMetricsForDpi) GetSystemMetricsForDpiPtr = nullptr;
    static inline decltype(&::GetDpiForMonitor) GetDpiForMonitorPtr = nullptr;

    static inline HWND FakeWindow = nullptr;

    static void Initialize();
    static void Cleanup();

private:
    static inline bool inited = false;
    static inline HMODULE user32 = nullptr;
    static inline HMODULE shcore = nullptr;
};

void HiDPI::Initialize()
{
    if (inited) {
        return;
    }
    inited = true;

    user32 = LoadLibraryExW(L"user32", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (user32) {
        GetDpiForWindowPtr = reinterpret_cast<decltype(GetDpiForWindowPtr)>(GetProcAddress(user32, "GetDpiForWindow"));
        GetSystemMetricsForDpiPtr = reinterpret_cast<decltype(GetSystemMetricsForDpiPtr)>(GetProcAddress(user32, "GetSystemMetricsForDpi"));
    } else {
        std::wcerr << L"Failed to retrieve the handle of the USER32.DLL." << std::endl;
    }

    shcore = LoadLibraryExW(L"shcore", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (shcore) {
        GetDpiForMonitorPtr = reinterpret_cast<decltype(GetDpiForMonitorPtr)>(GetProcAddress(shcore, "GetDpiForMonitor"));
    } else {
        std::wcerr << L"Failed to retrieve the handle of the SHCORE.DLL." << std::endl;
    }

    if (const HINSTANCE instance = GetModuleHandleW(nullptr)) {
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.lpfnWndProc = DefWindowProcW;
        wcex.hInstance = instance;
        wcex.lpszClassName = WINDOW_CLASS_NAME;
        if (RegisterClassExW(&wcex) != INVALID_ATOM) {
            FakeWindow = CreateWindowExW(0, WINDOW_CLASS_NAME, nullptr,
                WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
            if (!FakeWindow) {
                std::wcerr << L"Failed to create the window." << std::endl;
            }
        } else {
            std::wcerr << L"Failed to register the window class." << std::endl;
        }
    } else {
        std::wcerr << L"Failed to retrieve the handle of the current instance." << std::endl;
    }
}

void HiDPI::Cleanup()
{
    if (!inited) {
        return;
    }
    inited = false;

    GetDpiForWindowPtr = nullptr;
    GetSystemMetricsForDpiPtr = nullptr;
    GetDpiForMonitorPtr = nullptr;

    if (FakeWindow) {
        DestroyWindow(FakeWindow);
        UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandleW(nullptr));
        FakeWindow = nullptr;
    }

    if (shcore) {
        FreeLibrary(shcore);
        shcore = nullptr;
    }

    if (user32) {
        FreeLibrary(user32);
        user32 = nullptr;
    }
}

[[nodiscard]] static inline UINT GetCurrentDPIForPrimaryScreen()
{
    HiDPI::Initialize();
    if (const HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY)) {
        if (HiDPI::GetDpiForMonitorPtr) {
            UINT dpiX = 0, dpiY = 0;
            const HRESULT hr = HiDPI::GetDpiForMonitorPtr(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (SUCCEEDED(hr) && (dpiX > 0) && (dpiY > 0)) {
                return dpiX;
            } else {
                std::wcerr << L"GetDpiForMonitor() failed." << std::endl;
            }
        }
        MONITORINFOEXW monitorInfo;
        SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
        monitorInfo.cbSize = sizeof(monitorInfo);
        GetMonitorInfoW(hMonitor, &monitorInfo);
        if (const HDC hdc = CreateDCW(monitorInfo.szDevice, monitorInfo.szDevice, nullptr, nullptr)) {
            const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            DeleteDC(hdc);
            if ((dpiX > 0) && (dpiY > 0)) {
                return dpiX;
            } else {
                std::wcerr << L"Failed to retrieve the primary screen's DPI." << std::endl;
            }
        } else {
            std::wcerr << L"Failed to create DC for the primary monitor." << std::endl;
        }
    } else {
        std::wcerr << L"Failed to retrieve the primary monitor." << std::endl;
    }
    if (const HDC hdc = GetDC(nullptr)) {
        const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(nullptr, hdc);
        if ((dpiX > 0) && (dpiY > 0)) {
            return dpiX;
        } else {
            std::wcerr << L"Failed to retrieve the primary screen's DPI." << std::endl;
        }
    } else {
        std::wcerr << L"Failed to retrieve the primary screen's DC." << std::endl;
    }
    return 0;
}

[[nodiscard]] static inline UINT GetCurrentDPIForWindow()
{
    HiDPI::Initialize();
    if (HiDPI::GetDpiForWindowPtr) {
        return HiDPI::GetDpiForWindowPtr(HiDPI::FakeWindow);
    }
    return 0;
}

[[nodiscard]] static inline UINT GetCurrentDPI()
{
    UINT dpi = GetCurrentDPIForWindow();
    if (dpi == 0) {
        std::wcerr << L"Failed to retrieve the DPI from the window, trying to fetch from the primary screen instead..." << std::endl;
        dpi = GetCurrentDPIForPrimaryScreen();
        if (dpi == 0) {
            std::wcerr << L"Failed to retrieve the DPI from the primary screen, using the default DPI value instead..." << std::endl;
            dpi = USER_DEFAULT_SCREEN_DPI;
        }
    }
    return dpi;
}

[[nodiscard]] static inline int GetSystemMetricsForDpi2(const int index, const UINT dpi)
{
    HiDPI::Initialize();
    if (HiDPI::GetSystemMetricsForDpiPtr) {
        return HiDPI::GetSystemMetricsForDpiPtr(index, dpi);
    }
    const double dpr = double(GetCurrentDPI()) / double(USER_DEFAULT_SCREEN_DPI);
    const double result = double(GetSystemMetrics(index)) / dpr;
    return int(std::round(result));
}

[[nodiscard]] static inline std::wstring to_lower(std::wstring str)
{
    if (str.empty()) {
        return {};
    }
    std::transform(str.begin(), str.end(), str.begin(), std::towlower);
    return str;
}

EXTERN_C int WINAPI wmain(int argc, wchar_t *argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
    HiDPI::Initialize();
    std::unordered_map<std::wstring, std::wstring> options = {};
    std::vector<std::wstring> metrics = {};
    if (argc > 1) {
        bool skip = false;
        for (int i = 1; i != argc; ++i) {
            if (skip) {
                skip = false;
                continue;
            }
            const std::wstring arg = argv[i];
            if (arg.starts_with(L"SM_CX") || arg.starts_with(L"SM_CY")) {
                if (SYSTEM_METRIC_TABLE.contains(arg)) {
                    if (std::find(std::begin(metrics), std::end(metrics), arg) == std::end(metrics)) {
                        metrics.push_back(arg);
                    } else {
                        std::wcerr << L"Duplicated system metric value: " << arg << std::endl;
                    }
                } else {
                    std::wcerr << L"Unrecognized system metric value: " << arg << std::endl;
                }
            } else if (arg.starts_with(L'/') || arg.starts_with(L"--")) {
                const int length = arg.starts_with(L'/') ? 1 : 2;
                std::wstring option = std::wstring(arg).erase(0, length);
                std::wstring param = {};
                const std::wstring::size_type pos = option.find_first_of(L'=');
                if (pos == std::wstring::npos) {
                    const int index = i + 1;
                    if (index < argc) {
                        skip = true;
                        param = argv[index];
                    }
                } else {
                    param = option.substr(pos + 1);
                    option = option.substr(0, pos);
                }
                if (options.contains(option)) {
                    std::wcerr << L"Duplicated option: " << option << std::endl;
                } else {
                    options.insert({option, param});
                }
            } else {
                std::wcerr << L"Unrecognized parameter: " << arg << std::endl;
            }
        }
    }
    if (metrics.empty()) {
        for (auto &&[key, value] : std::as_const(SYSTEM_METRIC_TABLE)) {
            if (key.starts_with(L"SM_CX") || key.starts_with(L"SM_CY")) {
                metrics.push_back(key);
            }
        }
    }
    const auto metrics_count = int(metrics.size());
    const auto struct_name = [&options]() -> std::wstring {
        if (options.contains(OPT_STRUCT_NAME)) {
            const std::wstring name = options.at(OPT_STRUCT_NAME);
            if (!name.empty()) {
                return name;
            }
        }
        return DEFAULT_STRUCT_NAME;
    }();
    const auto variable_name = [&options]() -> std::wstring {
        if (options.contains(OPT_VARIABLE_NAME)) {
            const std::wstring name = options.at(OPT_VARIABLE_NAME);
            if (!name.empty()) {
                return name;
            }
        }
        return DEFAULT_VARIABLE_NAME;
    }();
    const bool enable_qt = options.contains(OPT_ENABLE_QT);
    const std::wstring hash_name = enable_qt ? HASH_QT_NAME : HASH_STD_NAME;
    std::wstring text = {};
    text += L"struct " + struct_name + L"\n{\n";
    for (int i = 0; i != DPI_COUNT; ++i) {
        const auto entry = DPI_TABLE[i];
        const auto percent = int(std::round(entry.DevicePixelRatio * double(100)));
        text += L"    int DPI_" + std::to_wstring(entry.DotsPerInch) + L" = 0;";
        text += L" // " + std::to_wstring(percent) + L"%. The scale factor for the device is "
                + std::to_wstring(entry.DevicePixelRatio) + L"x.\n";
        if (i == (DPI_COUNT - 1)) {
            text += L"};\n";
        }
    }
    text += L'\n';
    text += L"static const " + hash_name + L"<int, " + struct_name + L"> " + variable_name + L" =\n{\n";
    for (int i = 0; i != metrics_count; ++i) {
        const std::wstring sm = metrics.at(i);
        text += L"    {" + sm + L", {";
        for (int j = 0; j != DPI_COUNT; ++j) {
            const int index = SYSTEM_METRIC_TABLE.at(sm);
            const int value = GetSystemMetricsForDpi2(index, DPI_TABLE[j].DotsPerInch);
            text += std::to_wstring(value);
            if (j == (DPI_COUNT - 1)) {
                text += L"}}";
            } else {
                text += L", ";
            }
        }
        if (i != (metrics_count - 1)) {
            text += L',';
        }
        text += L'\n';
    }
    text += L"};";
    std::wcout << text << std::endl;
    HiDPI::Cleanup();
    return 0;
}
