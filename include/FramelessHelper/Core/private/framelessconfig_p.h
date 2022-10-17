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
#include <QtCore/qvariant.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_CORE_API FramelessConfig : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FramelessConfig)

public:
    explicit FramelessConfig(QObject *parent = nullptr);
    ~FramelessConfig() override;

    Q_NODISCARD static FramelessConfig *instance();

    void reload(const bool force = false);

    void set(const Global::Option option, const bool on = true);
    Q_NODISCARD bool isSet(const Global::Option option) const;

    static void setLoadFromEnvironmentVariablesDisabled(const bool on = true);
    static void setLoadFromConfigurationFileDisabled(const bool on = true);

    Q_NODISCARD QVariant setInternal(const QString &key, const QVariant &value);
    Q_NODISCARD QVariant getInternal(const QString &key) const;
    template<typename T>
    Q_NODISCARD std::optional<T> getInternal(const QString &key) const
    {
        const QVariant var = getInternal(key);
        if (var.isValid()) {
            return qvariant_cast<T>(var);
        }
        return std::nullopt;
    }
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessConfig))
