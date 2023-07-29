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

#pragma once

#include <FramelessHelper/Core/framelesshelpercore_global.h>
#include <optional>

FRAMELESSHELPER_BEGIN_NAMESPACE

struct SystemParameters;
class FramelessManager;

class FRAMELESSHELPER_CORE_API FramelessManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessManager)
    Q_DISABLE_COPY_MOVE(FramelessManagerPrivate)

public:
    explicit FramelessManagerPrivate(FramelessManager *q);
    ~FramelessManagerPrivate() override;

    Q_NODISCARD static FramelessManagerPrivate *get(FramelessManager *pub);
    Q_NODISCARD static const FramelessManagerPrivate *get(const FramelessManager *pub);

    static void initializeIconFont();
    Q_NODISCARD static QFont getIconFont();

    Q_NODISCARD Global::SystemTheme systemTheme() const;
    Q_NODISCARD QColor systemAccentColor() const;
    Q_NODISCARD QString wallpaper() const;
    Q_NODISCARD Global::WallpaperAspectStyle wallpaperAspectStyle() const;

    static void addWindow(const SystemParameters *params);
    static void removeWindow(const WId windowId);

    Q_INVOKABLE void notifySystemThemeHasChangedOrNot();
    Q_INVOKABLE void notifyWallpaperHasChangedOrNot();

    Q_NODISCARD static bool usePureQtImplementation();

    void setOverrideTheme(const Global::SystemTheme theme);
    Q_NODISCARD bool isThemeOverrided() const;

private:
    void initialize();

private:
    FramelessManager *q_ptr = nullptr;
    Global::SystemTheme m_systemTheme = Global::SystemTheme::Unknown;
    std::optional<Global::SystemTheme> m_overrideTheme = std::nullopt;
    QColor m_accentColor = {};
#ifdef Q_OS_WINDOWS
    Global::DwmColorizationArea m_colorizationArea = Global::DwmColorizationArea::None;
#endif
    QString m_wallpaper = {};
    Global::WallpaperAspectStyle m_wallpaperAspectStyle = Global::WallpaperAspectStyle::Fill;
};

FRAMELESSHELPER_END_NAMESPACE
