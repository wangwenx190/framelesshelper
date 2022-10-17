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
#include <QtQuick/qquickitem.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuickMicaMaterial)

class QuickMicaMaterialPrivate;

class FRAMELESSHELPER_QUICK_API QuickMicaMaterial : public QQuickItem
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(MicaMaterial)
#endif
    Q_DISABLE_COPY_MOVE(QuickMicaMaterial)
    Q_DECLARE_PRIVATE(QuickMicaMaterial)

public:
    explicit QuickMicaMaterial(QQuickItem *parent = nullptr);
    ~QuickMicaMaterial() override;

protected:
    void itemChange(const ItemChange change, const ItemChangeData &value) override;
    [[nodiscard]] QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *data) override;
    void classBegin() override;
    void componentComplete() override;

private:
    QScopedPointer<QuickMicaMaterialPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(QuickMicaMaterial))
QML_DECLARE_TYPE(FRAMELESSHELPER_PREPEND_NAMESPACE(QuickMicaMaterial))
