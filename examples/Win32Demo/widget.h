/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
QT_FORWARD_DECLARE_CLASS(Widget)
}
QT_END_NAMESPACE

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

class Widget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Widget)

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    bool isNormaled() const;

    bool shouldDrawBorder(const bool ignoreWindowState = false) const;
    bool shouldDrawThemedBorder(const bool ignoreWindowState = false) const;
    bool shouldDrawThemedTitleBar() const;

    QColor activeBorderColor() const;
    QColor inactiveBorderColor() const;
    QColor borderColor() const;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;

private:
    void updateTitleBar();
    void initWindow();

private:
    Ui::Widget *ui = nullptr;
    bool m_bIsWin10OrGreater = false, m_bIsWin101803OrGreater = false, m_bExtendToTitleBar = false,
         m_bShowColorDialog = false;
    const QColor m_cDefaultActiveBorderColor = {"#707070"} /*Qt::darkGray*/;
    const QColor m_cDefaultInactiveBorderColor = {"#aaaaaa"} /*Qt::gray*/;
    QColor m_cThemeColor = Qt::white;
};
