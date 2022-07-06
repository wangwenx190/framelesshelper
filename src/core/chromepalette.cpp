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

#include "chromepalette.h"
#include "chromepalette_p.h"
#include "framelessmanager.h"
#include "utils.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

ChromePalettePrivate::ChromePalettePrivate(ChromePalette *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    connect(FramelessManager::instance(),
        &FramelessManager::systemThemeChanged, this, &ChromePalettePrivate::refresh);
    refresh();
}

ChromePalettePrivate::~ChromePalettePrivate() = default;

ChromePalettePrivate *ChromePalettePrivate::get(ChromePalette *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

const ChromePalettePrivate *ChromePalettePrivate::get(const ChromePalette *q)
{
    Q_ASSERT(q);
    if (!q) {
        return nullptr;
    }
    return q->d_func();
}

void ChromePalettePrivate::refresh()
{
    const bool colorized = Utils::isTitleBarColorized();
    const bool dark = Utils::shouldAppsUseDarkMode();
    titleBarActiveBackgroundColor_sys = [colorized, dark]() -> QColor {
        if (colorized) {
#ifdef Q_OS_WINDOWS
            return Utils::getDwmColorizationColor();
#elif defined(Q_OS_LINUX)
            return Utils::getWmThemeColor();
#elif defined(Q_OS_MACOS)
            return Utils::getControlsAccentColor();
#else
            return {};
#endif
        } else {
            return (dark ? kDefaultBlackColor : kDefaultWhiteColor);
        }
    }();
    titleBarInactiveBackgroundColor_sys = (dark ? kDefaultSystemDarkColor : kDefaultWhiteColor);
    titleBarActiveForegroundColor_sys = [this, dark, colorized]() -> QColor {
        if (dark || colorized) {
            // Calculate the most appropriate foreground color, based on the
            // current background color.
            const qreal grayF = (
                (0.299 * titleBarActiveBackgroundColor_sys.redF()) +
                (0.587 * titleBarActiveBackgroundColor_sys.greenF()) +
                (0.114 * titleBarActiveBackgroundColor_sys.blueF()));
            if (grayF <= 0.5) {
                return kDefaultWhiteColor;
            }
        }
        return kDefaultBlackColor;
    }();
    titleBarInactiveForegroundColor_sys = kDefaultDarkGrayColor;
    chromeButtonNormalColor_sys = kDefaultTransparentColor;
    chromeButtonHoverColor_sys =
        Utils::calculateSystemButtonBackgroundColor(SystemButtonType::Minimize, ButtonState::Hovered);
    chromeButtonPressColor_sys =
        Utils::calculateSystemButtonBackgroundColor(SystemButtonType::Minimize, ButtonState::Pressed);
    closeButtonNormalColor_sys = kDefaultTransparentColor;
    closeButtonHoverColor_sys =
        Utils::calculateSystemButtonBackgroundColor(SystemButtonType::Close, ButtonState::Hovered);
    closeButtonPressColor_sys =
        Utils::calculateSystemButtonBackgroundColor(SystemButtonType::Close, ButtonState::Pressed);
    Q_Q(ChromePalette);
    Q_EMIT q->titleBarActiveBackgroundColorChanged();
    Q_EMIT q->titleBarInactiveBackgroundColorChanged();
    Q_EMIT q->titleBarActiveForegroundColorChanged();
    Q_EMIT q->titleBarInactiveForegroundColorChanged();
    Q_EMIT q->chromeButtonNormalColorChanged();
    Q_EMIT q->chromeButtonHoverColorChanged();
    Q_EMIT q->chromeButtonPressColorChanged();
    Q_EMIT q->closeButtonNormalColorChanged();
    Q_EMIT q->closeButtonHoverColorChanged();
    Q_EMIT q->closeButtonPressColorChanged();
    Q_EMIT q->titleBarColorChanged();
    Q_EMIT q->chromeButtonColorChanged();
}

ChromePalette::ChromePalette(QObject *parent) :
    QObject(parent), d_ptr(new ChromePalettePrivate(this))
{
}

ChromePalette::~ChromePalette() = default;

QColor ChromePalette::titleBarActiveBackgroundColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::TitleBarActiveBackgroundColor) {
        return d->titleBarActiveBackgroundColor;
    }
    return d->titleBarActiveBackgroundColor_sys;
}

QColor ChromePalette::titleBarInactiveBackgroundColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::TitleBarInactiveBackgroundColor) {
        return d->titleBarInactiveBackgroundColor;
    }
    return d->titleBarInactiveBackgroundColor_sys;
}

QColor ChromePalette::titleBarActiveForegroundColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::TitleBarActiveForegroundColor) {
        return d->titleBarActiveForegroundColor;
    }
    return d->titleBarActiveForegroundColor_sys;
}

QColor ChromePalette::titleBarInactiveForegroundColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::TitleBarInactiveForegroundColor) {
        return d->titleBarInactiveForegroundColor;
    }
    return d->titleBarInactiveForegroundColor_sys;
}

QColor ChromePalette::chromeButtonNormalColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::ChromeButtonNormalColor) {
        return d->chromeButtonNormalColor;
    }
    return d->chromeButtonNormalColor_sys;
}

QColor ChromePalette::chromeButtonHoverColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::ChromeButtonHoverColor) {
        return d->chromeButtonHoverColor;
    }
    return d->chromeButtonHoverColor_sys;
}

QColor ChromePalette::chromeButtonPressColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::ChromeButtonPressColor) {
        return d->chromeButtonPressColor;
    }
    return d->chromeButtonPressColor_sys;
}

QColor ChromePalette::closeButtonNormalColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::CloseButtonNormalColor) {
        return d->closeButtonNormalColor;
    }
    return d->closeButtonNormalColor_sys;
}

QColor ChromePalette::closeButtonHoverColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::CloseButtonHoverColor) {
        return d->closeButtonHoverColor;
    }
    return d->closeButtonHoverColor_sys;
}

QColor ChromePalette::closeButtonPressColor() const
{
    Q_D(const ChromePalette);
    if (d->mask & ChromePalettePrivate::MaskFlag::CloseButtonPressColor) {
        return d->closeButtonPressColor;
    }
    return d->closeButtonPressColor_sys;
}

void ChromePalette::setTitleBarActiveBackgroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->titleBarActiveBackgroundColor == value) {
        return;
    }
    d->titleBarActiveBackgroundColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::TitleBarActiveBackgroundColor;
    Q_EMIT titleBarActiveBackgroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::resetTitleBarActiveBackgroundColor()
{
    Q_D(ChromePalette);
    d->titleBarActiveBackgroundColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::TitleBarActiveBackgroundColor);
    Q_EMIT titleBarActiveBackgroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::setTitleBarInactiveBackgroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->titleBarInactiveBackgroundColor == value) {
        return;
    }
    d->titleBarInactiveBackgroundColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::TitleBarInactiveBackgroundColor;
    Q_EMIT titleBarInactiveBackgroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::resetTitleBarInactiveBackgroundColor()
{
    Q_D(ChromePalette);
    d->titleBarInactiveBackgroundColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::TitleBarInactiveBackgroundColor);
    Q_EMIT titleBarInactiveBackgroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::setTitleBarActiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->titleBarActiveForegroundColor == value) {
        return;
    }
    d->titleBarActiveForegroundColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::TitleBarActiveForegroundColor;
    Q_EMIT titleBarActiveForegroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::resetTitleBarActiveForegroundColor()
{
    Q_D(ChromePalette);
    d->titleBarActiveForegroundColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::TitleBarActiveForegroundColor);
    Q_EMIT titleBarActiveForegroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::setTitleBarInactiveForegroundColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->titleBarInactiveForegroundColor == value) {
        return;
    }
    d->titleBarInactiveForegroundColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::TitleBarInactiveForegroundColor;
    Q_EMIT titleBarInactiveForegroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::resetTitleBarInactiveForegroundColor()
{
    Q_D(ChromePalette);
    d->titleBarInactiveForegroundColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::TitleBarInactiveForegroundColor);
    Q_EMIT titleBarInactiveForegroundColorChanged();
    Q_EMIT titleBarColorChanged();
}

void ChromePalette::setChromeButtonNormalColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->chromeButtonNormalColor == value) {
        return;
    }
    d->chromeButtonNormalColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::ChromeButtonNormalColor;
    Q_EMIT chromeButtonNormalColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetChromeButtonNormalColor()
{
    Q_D(ChromePalette);
    d->chromeButtonNormalColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::ChromeButtonNormalColor);
    Q_EMIT chromeButtonNormalColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::setChromeButtonHoverColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->chromeButtonHoverColor == value) {
        return;
    }
    d->chromeButtonHoverColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::ChromeButtonHoverColor;
    Q_EMIT chromeButtonHoverColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetChromeButtonHoverColor()
{
    Q_D(ChromePalette);
    d->chromeButtonHoverColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::ChromeButtonHoverColor);
    Q_EMIT chromeButtonHoverColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::setChromeButtonPressColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->chromeButtonPressColor == value) {
        return;
    }
    d->chromeButtonPressColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::ChromeButtonPressColor;
    Q_EMIT chromeButtonPressColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetChromeButtonPressColor()
{
    Q_D(ChromePalette);
    d->chromeButtonPressColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::ChromeButtonPressColor);
    Q_EMIT chromeButtonPressColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::setCloseButtonNormalColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->closeButtonNormalColor == value) {
        return;
    }
    d->closeButtonNormalColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::CloseButtonNormalColor;
    Q_EMIT closeButtonNormalColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetCloseButtonNormalColor()
{
    Q_D(ChromePalette);
    d->closeButtonNormalColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::CloseButtonNormalColor);
    Q_EMIT closeButtonNormalColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::setCloseButtonHoverColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->closeButtonHoverColor == value) {
        return;
    }
    d->closeButtonHoverColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::CloseButtonHoverColor;
    Q_EMIT closeButtonHoverColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetCloseButtonHoverColor()
{
    Q_D(ChromePalette);
    d->closeButtonHoverColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::CloseButtonHoverColor);
    Q_EMIT closeButtonHoverColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::setCloseButtonPressColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    Q_D(ChromePalette);
    if (d->closeButtonPressColor == value) {
        return;
    }
    d->closeButtonPressColor = value;
    d->mask |= ChromePalettePrivate::MaskFlag::CloseButtonPressColor;
    Q_EMIT closeButtonPressColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

void ChromePalette::resetCloseButtonPressColor()
{
    Q_D(ChromePalette);
    d->closeButtonPressColor = {};
    d->mask &= ~ChromePalettePrivate::Mask(ChromePalettePrivate::MaskFlag::CloseButtonPressColor);
    Q_EMIT closeButtonPressColorChanged();
    Q_EMIT chromeButtonColorChanged();
}

FRAMELESSHELPER_END_NAMESPACE
