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

#include "framelesshelperwidgets_global.h"
#include "framelessdialog.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

class WidgetsSharedHelper;

class FRAMELESSHELPER_WIDGETS_API FramelessDialogPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessDialog)
    Q_DISABLE_COPY_MOVE(FramelessDialogPrivate)

public:
    explicit FramelessDialogPrivate(FramelessDialog *q);
    ~FramelessDialogPrivate() override;

    Q_NODISCARD static FramelessDialogPrivate *get(FramelessDialog *pub);
    Q_NODISCARD static const FramelessDialogPrivate *get(const FramelessDialog *pub);

    Q_NODISCARD WidgetsSharedHelper *widgetsSharedHelper() const;

private:
    void initialize();

private:
    QPointer<FramelessDialog> q_ptr = nullptr;
    QScopedPointer<WidgetsSharedHelper> m_helper;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessDialogPrivate))
