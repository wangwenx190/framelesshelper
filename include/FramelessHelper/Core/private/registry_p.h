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

QT_BEGIN_NAMESPACE
class QWinRegistryKey;
class QSettings;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class FRAMELESSHELPER_CORE_API Registry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Registry)

public:
    explicit Registry(const Global::RegistryRootKey root, const QString &key, QObject *parent = nullptr);
    ~Registry() override;

    Q_NODISCARD Global::RegistryRootKey rootKey() const;
    Q_NODISCARD QString subKey() const;

    Q_NODISCARD bool isValid() const;

    Q_NODISCARD QVariant value(const QString &name) const;

private:
    Global::RegistryRootKey m_rootKey = Global::RegistryRootKey::CurrentUser;
    QString m_subKey = {};
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QScopedPointer<QWinRegistryKey> m_registry;
#else
    QScopedPointer<QSettings> m_settings;
    bool m_valid = false;
#endif
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(Registry))
