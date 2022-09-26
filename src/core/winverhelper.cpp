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

#include "winverhelper_p.h"
#include "utils.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

namespace WindowsVersionHelper
{

bool isWin2KOrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_2000);
    return result;
}

bool isWinXPOrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_XP);
    return result;
}

bool isWinXP64OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_XP_64);
    return result;
}

bool isWinVistaOrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_Vista);
    return result;
}

bool isWinVistaSP1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_Vista_SP1);
    return result;
}

bool isWinVistaSP2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_Vista_SP2);
    return result;
}

bool isWin7OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_7);
    return result;
}

bool isWin7SP1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_7_SP1);
    return result;
}

bool isWin8OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_8);
    return result;
}

bool isWin8Point1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_8_1);
    return result;
}

bool isWin8Point1Update1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_8_1_Update1);
    return result;
}

bool isWin10OrGreater()
{
    return isWin10TH1OrGreater();
}

bool isWin10TH1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1507);
    return result;
}

bool isWin10TH2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1511);
    return result;
}

bool isWin10RS1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1607);
    return result;
}

bool isWin10RS2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1703);
    return result;
}

bool isWin10RS3OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1709);
    return result;
}

bool isWin10RS4OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1803);
    return result;
}

bool isWin10RS5OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1809);
    return result;
}

bool isWin1019H1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1903);
    return result;
}

bool isWin1019H2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_1909);
    return result;
}

bool isWin1020H1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_2004);
    return result;
}

bool isWin1020H2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_20H2);
    return result;
}

bool isWin21H1OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_21H1);
    return result;
}

bool isWin21H2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_10_21H2);
    return result;
}

bool isWin11OrGreater()
{
    return isWin1121H2OrGreater();
}

bool isWin1121H2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
    return result;
}

bool isWin1122H2OrGreater()
{
    static const bool result = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_22H2);
    return result;
}

} // namespace WindowsVersionHelper

FRAMELESSHELPER_END_NAMESPACE
