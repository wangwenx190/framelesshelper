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

namespace WindowsVersionHelper
{

[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin2KOrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWinXPOrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWinXP64OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWinVistaOrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWinVistaSP1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWinVistaSP2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin7OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin7SP1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8Point1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin8Point1Update1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10TH1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10TH2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10RS1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10RS2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10RS3OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10RS4OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin10RS5OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1019H1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1019H2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1020H1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1020H2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin21H1OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin21H2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin11OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1121H2OrGreater();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWin1122H2OrGreater();

} // namespace WindowsVersionHelper

FRAMELESSHELPER_END_NAMESPACE
