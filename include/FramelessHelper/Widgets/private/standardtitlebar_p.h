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
#include "standardtitlebar.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE

class StandardSystemButton;
class ChromePalette;

class FRAMELESSHELPER_WIDGETS_API StandardTitleBarPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(StandardTitleBar)
    Q_DISABLE_COPY_MOVE(StandardTitleBarPrivate)

public:
    explicit StandardTitleBarPrivate(StandardTitleBar *q);
    ~StandardTitleBarPrivate() override;

    Q_NODISCARD static StandardTitleBarPrivate *get(StandardTitleBar *pub);
    Q_NODISCARD static const StandardTitleBarPrivate *get(const StandardTitleBar *pub);

    Q_NODISCARD Qt::Alignment titleLabelAlignment() const;
    void setTitleLabelAlignment(const Qt::Alignment value);

    Q_NODISCARD bool isExtended() const;
    void setExtended(const bool value);

    Q_NODISCARD bool isHideWhenClose() const;
    void setHideWhenClose(const bool value);

    Q_NODISCARD ChromePalette *chromePalette() const;

    void paintTitleBar(QPaintEvent *event);

    Q_NODISCARD bool titleLabelVisible() const;
    void setTitleLabelVisible(const bool value);

    Q_NODISCARD QSize windowIconSize() const;
    void setWindowIconSize(const QSize &value);

    Q_NODISCARD bool windowIconVisible() const;
    void setWindowIconVisible(const bool value);

    Q_NODISCARD QFont titleFont() const;
    void setTitleFont(const QFont &value);

    Q_NODISCARD bool mouseEventHandler(QMouseEvent *event);

    Q_NODISCARD QRect windowIconRect() const;
    Q_NODISCARD bool windowIconVisible_real() const;
    Q_NODISCARD bool isInTitleBarIconArea(const QPoint &pos) const;

public Q_SLOTS:
    void updateMaximizeButton();
    void updateTitleBarColor();
    void updateChromeButtonColor();
    void retranslateUi();

protected:
    Q_NODISCARD bool eventFilter(QObject *object, QEvent *event) override;

private:
    void initialize();

private:
    QPointer<StandardTitleBar> q_ptr = nullptr;
    QScopedPointer<StandardSystemButton> m_minimizeButton;
    QScopedPointer<StandardSystemButton> m_maximizeButton;
    QScopedPointer<StandardSystemButton> m_closeButton;
    QPointer<QWidget> m_window = nullptr;
    bool m_extended = false;
    Qt::Alignment m_labelAlignment = {};
    bool m_hideWhenClose = false;
    QScopedPointer<ChromePalette> m_chromePalette;
    bool m_titleLabelVisible = true;
    std::optional<QSize> m_windowIconSize = std::nullopt;
    bool m_windowIconVisible = false;
    std::optional<QFont> m_titleFont = std::nullopt;
    bool m_closeTriggered = false;
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(StandardTitleBarPrivate))
