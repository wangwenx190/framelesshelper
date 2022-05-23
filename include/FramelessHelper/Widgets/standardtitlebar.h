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
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class StandardTitleBarPrivate;
class StandardSystemButton;

class FRAMELESSHELPER_WIDGETS_API StandardTitleBar : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StandardTitleBar)
    Q_DISABLE_COPY_MOVE(StandardTitleBar)
    Q_PROPERTY(Qt::Alignment titleLabelAlignment READ titleLabelAlignment WRITE setTitleLabelAlignment NOTIFY titleLabelAlignmentChanged FINAL)
    Q_PROPERTY(QLabel* titleLabel READ titleLabel CONSTANT FINAL)
    Q_PROPERTY(StandardSystemButton* minimizeButton READ minimizeButton CONSTANT FINAL)
    Q_PROPERTY(StandardSystemButton* maximizeButton READ maximizeButton CONSTANT FINAL)
    Q_PROPERTY(StandardSystemButton* closeButton READ closeButton CONSTANT FINAL)
    Q_PROPERTY(bool extended READ isExtended WRITE setExtended NOTIFY extendedChanged FINAL)
    Q_PROPERTY(bool useAlternativeBackground READ isUsingAlternativeBackground WRITE setUseAlternativeBackground NOTIFY useAlternativeBackgroundChanged FINAL)
    Q_PROPERTY(bool hideWhenClose READ isHideWhenClose WRITE setHideWhenClose NOTIFY hideWhenCloseChanged FINAL)

public:
    explicit StandardTitleBar(QWidget *parent = nullptr);
    ~StandardTitleBar() override;

    Q_NODISCARD Qt::Alignment titleLabelAlignment() const;
    void setTitleLabelAlignment(const Qt::Alignment value);

    Q_NODISCARD QLabel *titleLabel() const;
    Q_NODISCARD StandardSystemButton *minimizeButton() const;
    Q_NODISCARD StandardSystemButton *maximizeButton() const;
    Q_NODISCARD StandardSystemButton *closeButton() const;

    Q_NODISCARD bool isExtended() const;
    void setExtended(const bool value);

    Q_NODISCARD bool isUsingAlternativeBackground() const;
    void setUseAlternativeBackground(const bool value);

    Q_NODISCARD bool isHideWhenClose() const;
    void setHideWhenClose(const bool value);

protected:
    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void extendedChanged();
    void titleLabelAlignmentChanged();
    void useAlternativeBackgroundChanged();
    void hideWhenCloseChanged();

private:
    QScopedPointer<StandardTitleBarPrivate> d_ptr;
};

FRAMELESSHELPER_END_NAMESPACE
