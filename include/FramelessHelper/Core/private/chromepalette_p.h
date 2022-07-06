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
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class ChromePalette;

class FRAMELESSHELPER_CORE_API ChromePalettePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ChromePalette)
    Q_DISABLE_COPY_MOVE(ChromePalettePrivate)

public:
    enum class MaskFlag
    {
        TitleBarActiveBackgroundColor = 0x00000001,
        TitleBarInactiveBackgroundColor = 0x00000002,
        TitleBarActiveForegroundColor = 0x00000004,
        TitleBarInactiveForegroundColor = 0x00000008,
        ChromeButtonNormalColor = 0x00000010,
        ChromeButtonHoverColor = 0x00000020,
        ChromeButtonPressColor = 0x00000040,
        CloseButtonNormalColor = 0x00000080,
        CloseButtonHoverColor = 0x00000100,
        CloseButtonPressColor = 0x00000200
    };
    Q_ENUM(MaskFlag)
    Q_DECLARE_FLAGS(Mask, MaskFlag)
    Q_FLAG(Mask)

    explicit ChromePalettePrivate(ChromePalette *q);
    ~ChromePalettePrivate() override;

    Q_NODISCARD static ChromePalettePrivate *get(ChromePalette *q);
    Q_NODISCARD static const ChromePalettePrivate *get(const ChromePalette *q);

public Q_SLOTS:
    void refresh();

private:
    QPointer<ChromePalette> q_ptr = nullptr;
    Mask mask = {};
    // System-defined ones:
    QColor titleBarActiveBackgroundColor_sys = {};
    QColor titleBarInactiveBackgroundColor_sys = {};
    QColor titleBarActiveForegroundColor_sys = {};
    QColor titleBarInactiveForegroundColor_sys = {};
    QColor chromeButtonNormalColor_sys = {};
    QColor chromeButtonHoverColor_sys = {};
    QColor chromeButtonPressColor_sys = {};
    QColor closeButtonNormalColor_sys = {};
    QColor closeButtonHoverColor_sys = {};
    QColor closeButtonPressColor_sys = {};
    // User-defined ones:
    QColor titleBarActiveBackgroundColor = {};
    QColor titleBarInactiveBackgroundColor = {};
    QColor titleBarActiveForegroundColor = {};
    QColor titleBarInactiveForegroundColor = {};
    QColor chromeButtonNormalColor = {};
    QColor chromeButtonHoverColor = {};
    QColor chromeButtonPressColor = {};
    QColor closeButtonNormalColor = {};
    QColor closeButtonHoverColor = {};
    QColor closeButtonPressColor = {};
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ChromePalettePrivate::Mask)

FRAMELESSHELPER_END_NAMESPACE
