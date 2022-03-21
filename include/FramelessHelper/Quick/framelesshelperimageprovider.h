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

#include "framelesshelperquick_global.h"
#include <QtQuick/qquickimageprovider.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_QUICK_API FramelessHelperImageProvider : public QQuickImageProvider
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    Q_OBJECT
#endif
    Q_DISABLE_COPY_MOVE(FramelessHelperImageProvider)

public:
    explicit FramelessHelperImageProvider();
    ~FramelessHelperImageProvider() override;

    Q_NODISCARD QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

FRAMELESSHELPER_END_NAMESPACE
