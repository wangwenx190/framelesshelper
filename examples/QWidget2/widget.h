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

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

class QGraphicsDropShadowEffect;
class QPushButton;
class QLabel;

class ContentsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ContentsWidget)

public:
    explicit ContentsWidget(QWidget *parent = nullptr);
    ~ContentsWidget() override = default;

    void setShouldDrawWindowBorder(const bool val);
    bool getShouldDrawWindowBorder() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_bShouldDrawWindowBorder = true;
};

class Widget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Widget)

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override = default;

    bool isNormal() const;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void setupUi();
    void initBackgroundWindow();
    void setFrameShadowEnabled(const bool enable = true);
    void setFrameShadowActive(const bool active = true);

private:
    int titleBarHeight = 30;
    ContentsWidget *contentsWidget = nullptr;
    QGraphicsDropShadowEffect *shadowEffect = nullptr;
    QWidget *titleBarWidget = nullptr;
    QPushButton *windowIconButton = nullptr;
    QLabel *windowTitleLabel = nullptr;
    QPushButton *minimizeButton = nullptr;
    QPushButton *maximizeButton = nullptr;
    QPushButton *closeButton = nullptr;
};
