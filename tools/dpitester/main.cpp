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
#include <iostream>
#include <string>
#include <map>
#include <vector>

#ifndef SM_CYPADDEDBORDER
#  define SM_CYPADDEDBORDER SM_CXPADDEDBORDER
#endif

static const std::map<std::wstring, int> SYSTEM_METRIC_TABLE = {
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

static const int DPI_TABLE[] = {
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.0)), // 100%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.2)), // 120%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.25)), // 125%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.4)), // 140%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.5)), // 150%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.6)), // 160%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.75)), // 175%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 1.8)), // 180%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 2.0)), // 200%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 2.25)), // 225%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 2.5)), // 250%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 3.0)), // 300%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 3.5)), // 350%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 4.0)), // 400%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 4.5)), // 450%
    int(std::round(USER_DEFAULT_SCREEN_DPI * 5.0)) // 500%
};
static const auto DPI_COUNT = int(std::size(DPI_TABLE));

EXTERN_C int WINAPI wmain(int argc, wchar_t *argv[])
{
    std::map<std::wstring, std::wstring> options = {};
    std::vector<std::wstring> metrics = {};
    if (argc > 1) {
        for (int i = 1; i != argc; ++i) {
            const std::wstring arg = argv[i];
            if (arg.starts_with(L"SM_CX") || arg.starts_with(L"SM_CY")) {
                if (SYSTEM_METRIC_TABLE.contains(arg)) {
                    if (std::find(std::begin(metrics), std::end(metrics), arg) == std::end(metrics)) {
                        metrics.push_back(arg);
                    }
                } else {
                    std::wcerr << L"Unrecognized system metric value: " << arg << std::endl;
                }
            } else if (arg.starts_with(L'/') || arg.starts_with(L"--")) {
                const int length = arg.starts_with(L'/') ? 1 : 2;
                const std::wstring option = std::wstring(arg).erase(0, length);
                if (options.contains(option)) {
                    std::wcerr << L"Duplicated option: " << option << std::endl;
                } else {
                    const std::wstring param = [&option, i, argc, argv]() -> std::wstring {
                        const std::wstring::size_type pos = option.find_first_of(L'=');
                        if (pos == std::wstring::npos) {
                            const int index = i + 1;
                            if (index < argc) {
                                return argv[index];
                            }
                            return {};
                        }
                        return option.substr(pos, option.length() - pos - 1);
                    }();
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
    std::wstring text = L"// DPI: ";
    for (int i = 0; i != DPI_COUNT; ++i) {
        text += std::to_wstring(DPI_TABLE[i]);
        if (i == (DPI_COUNT - 1)) {
            text += L'\n';
        } else {
            text += L", ";
        }
    }
    text += L"static const QHash<int, SYSTEM_METRIC> g_systemMetricsTable = {\n";
    for (int i = 0; i != metrics_count; ++i) {
        const std::wstring sm = metrics.at(i);
        text += L"    {" + sm + L", {";
        for (int j = 0; j != DPI_COUNT; ++j) {
            const int index = SYSTEM_METRIC_TABLE.at(sm);
            const int value = GetSystemMetricsForDpi(index, DPI_TABLE[j]);
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
    return 0;
}
