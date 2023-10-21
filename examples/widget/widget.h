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

#include <FramelessHelper/Widgets/framelesswidget.h>

FRAMELESSHELPER_REQUIRE_CONFIG(window)

QT_BEGIN_NAMESPACE
class QLabel;
class QShortcut;
QT_END_NAMESPACE

#if FRAMELESSHELPER_CONFIG(titlebar)
FRAMELESSHELPER_BEGIN_NAMESPACE
class StandardTitleBar;
FRAMELESSHELPER_END_NAMESPACE
#endif

class Widget : public FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessWidget)
{
    Q_OBJECT
    Q_DISABLE_COPY(Widget)

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void waitReady();

protected:
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void initialize();

private Q_SLOTS:
    void updateStyleSheet();

private:
#if FRAMELESSHELPER_CONFIG(titlebar)
    FRAMELESSHELPER_PREPEND_NAMESPACE(StandardTitleBar) *m_titleBar = nullptr;
#endif
    QLabel *m_clockLabel = nullptr;
    QLabel *m_compilerInfoLabel = nullptr;
    QLabel *m_commitInfoLabel = nullptr;
    QShortcut *m_fullScreenShortcut = nullptr;
    QShortcut *m_cancelShortcut = nullptr;
    int m_timerId = -1;
};
