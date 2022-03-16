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

#include "widget.h"
#include <QtCore/qdatetime.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <utils.h>

FRAMELESSHELPER_USE_NAMESPACE

Widget::Widget(QWidget *parent) : FramelessWidget(parent)
{
    setupUi();
    connect(this, &Widget::systemThemeChanged, this, &Widget::updateStyleSheet);
    startTimer(500);
}

Widget::~Widget() = default;

void Widget::timerEvent(QTimerEvent *event)
{
    FramelessWidget::timerEvent(event);
    if (m_clockLabel) {
        m_clockLabel->setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    }
}

void Widget::setupUi()
{
    setWindowTitle(tr("Hello, World!"));
    resize(800, 600);
    m_clockLabel = new QLabel(this);
    m_clockLabel->setFrameShape(QFrame::NoFrame);
    QFont clockFont = font();
    clockFont.setBold(true);
    clockFont.setPointSize(70);
    m_clockLabel->setFont(clockFont);
    const auto widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const auto contentLayout = new QHBoxLayout(widget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_clockLabel);
    contentLayout->addStretch();
    widget->setLayout(contentLayout);
    setContentWidget(widget);
    updateStyleSheet();
}

void Widget::updateStyleSheet()
{
    const bool dark = Utils::shouldAppsUseDarkMode();
    const QColor clockLabelTextColor = (dark ? Qt::white : Qt::black);
    const QColor widgetBackgroundColor = (dark ? kDefaultSystemDarkColor : kDefaultSystemLightColor);
    m_clockLabel->setStyleSheet(QStringLiteral("color: %1;").arg(clockLabelTextColor.name()));
    setStyleSheet(QStringLiteral("background-color: %1;").arg(widgetBackgroundColor.name()));
    update();
}
