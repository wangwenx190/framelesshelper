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

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessWindowsManager;

class FRAMELESSHELPER_CORE_API FramelessWindowsManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessWindowsManager)
    Q_DISABLE_COPY_MOVE(FramelessWindowsManagerPrivate)

public:
    explicit FramelessWindowsManagerPrivate(FramelessWindowsManager *q);
    ~FramelessWindowsManagerPrivate() override;

    Q_NODISCARD static FramelessWindowsManagerPrivate *get(FramelessWindowsManager *pub);
    Q_NODISCARD static const FramelessWindowsManagerPrivate *get(const FramelessWindowsManager *pub);

    Q_NODISCARD static bool usePureQtImplementation();
    Q_NODISCARD Global::SystemTheme systemTheme() const;
    Q_NODISCARD QColor systemAccentColor() const;

    static void addWindow(const Global::SystemParameters &params);
    Q_INVOKABLE void notifySystemThemeHasChangedOrNot();

private:
    void initialize();

private:
    FramelessWindowsManager *q_ptr = nullptr;
    Global::SystemTheme m_systemTheme = Global::SystemTheme::Unknown;
    QColor m_accentColor = {};
#ifdef Q_OS_WINDOWS
    Global::DwmColorizationArea m_colorizationArea = Global::DwmColorizationArea::None_;
#endif
};

FRAMELESSHELPER_END_NAMESPACE
