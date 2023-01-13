/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#include "quicksettings.h"
#include "../shared/settings.h"

FRAMELESSHELPER_STRING_CONSTANT(Geometry)
FRAMELESSHELPER_STRING_CONSTANT(DevicePixelRatio)

extern template void Settings::set<QRect>(const QString &, const QString &, const QRect &);
extern template void Settings::set<qreal>(const QString &, const QString &, const qreal &);

extern template QRect Settings::get<QRect>(const QString &, const QString &);
extern template qreal Settings::get<qreal>(const QString &, const QString &);

QuickSettings::QuickSettings(QObject *parent) : QObject(parent)
{
}

QuickSettings::~QuickSettings() = default;

void QuickSettings::saveGeometry(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const QString objName = window->objectName();
    Settings::set(objName, kGeometry, window->geometry());
    Settings::set(objName, kDevicePixelRatio, window->devicePixelRatio());
}

bool QuickSettings::restoreGeometry(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const QString objName = window->objectName();
    const auto savedGeometry = Settings::get<QRect>(objName, kGeometry);
    if (!savedGeometry.isValid()) {
        return false;
    }
    const auto savedDpr = Settings::get<qreal>(objName, kDevicePixelRatio);
    // Qt doesn't support dpr < 1.
    const qreal oldDpr = std::max(savedDpr, qreal(1));
    const qreal scale = (window->devicePixelRatio() / oldDpr);
    window->setGeometry({savedGeometry.topLeft() * scale, savedGeometry.size() * scale});
    return true;
}
