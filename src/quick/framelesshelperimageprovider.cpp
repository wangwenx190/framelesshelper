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

#include "framelesshelperimageprovider.h"
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

[[nodiscard]] static inline SystemTheme strToTheme(const QString &str)
{
    Q_ASSERT(!str.isEmpty());
    if (str.isEmpty()) {
        return SystemTheme::Unknown;
    }
    if (str.compare(QStringLiteral("light"), Qt::CaseInsensitive) == 0) {
        return SystemTheme::Light;
    }
    if (str.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0) {
        return SystemTheme::Dark;
    }
    if (str.compare(QStringLiteral("highcontrast"), Qt::CaseInsensitive) == 0) {
        return SystemTheme::HighContrast;
    }
    return SystemTheme::Unknown;
}

[[nodiscard]] static inline SystemButtonType strToButton(const QString &str)
{
    Q_ASSERT(!str.isEmpty());
    if (str.isEmpty()) {
        return SystemButtonType::Unknown;
    }
    if (str.compare(QStringLiteral("windowicon"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::WindowIcon;
    }
    if (str.compare(QStringLiteral("help"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::Help;
    }
    if (str.compare(QStringLiteral("minimize"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::Minimize;
    }
    if (str.compare(QStringLiteral("maximize"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::Maximize;
    }
    if (str.compare(QStringLiteral("restore"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::Restore;
    }
    if (str.compare(QStringLiteral("close"), Qt::CaseInsensitive) == 0) {
        return SystemButtonType::Close;
    }
    return SystemButtonType::Unknown;
}

FramelessHelperImageProvider::FramelessHelperImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

FramelessHelperImageProvider::~FramelessHelperImageProvider() = default;

QPixmap FramelessHelperImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_ASSERT(!id.isEmpty());
    if (id.isEmpty()) {
        return {};
    }
    const QStringList params = id.split(u'/', Qt::SkipEmptyParts, Qt::CaseInsensitive);
    Q_ASSERT(!params.isEmpty());
    if (params.isEmpty()) {
        return {};
    }
    Q_ASSERT(params.count() >= 2);
    if (params.count() < 2) {
        return {};
    }
    const SystemTheme theme = strToTheme(params.at(0));
    const SystemButtonType button = strToButton(params.at(1));
    const QVariant pixmapVar = Utils::getSystemButtonIconResource(button, theme, ResourceType::Pixmap);
    if (!pixmapVar.isValid()) {
        return {};
    }
    if (static_cast<QMetaType::Type>(pixmapVar.userType()) != QMetaType::QPixmap) {
        return {};
    }
    if (size) {
        *size = kDefaultSystemButtonIconSize;
    }
    const auto pixmap = qvariant_cast<QPixmap>(pixmapVar);
    if (!requestedSize.isEmpty() && (pixmap.size() != requestedSize)) {
        return pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return pixmap;
}

FRAMELESSHELPER_END_NAMESPACE
