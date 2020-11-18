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

#include "widget.h"
#include "../../framelesshelper.h"
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

Q_GLOBAL_STATIC(FramelessHelper, framelessHelper)

ContentsWidget::ContentsWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
}

void ContentsWidget::setShouldDrawWindowBorder(const bool val)
{
    m_bShouldDrawWindowBorder = val;
    update();
}

bool ContentsWidget::getShouldDrawWindowBorder() const
{
    return m_bShouldDrawWindowBorder;
}

void ContentsWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (m_bShouldDrawWindowBorder) {
        QPainter painter(this);
        painter.save();
        painter.setPen({Qt::black, 1.5});
        painter.drawLine(0, 0, width(), 0);
        painter.drawLine(0, height(), width(), height());
        painter.drawLine(0, 0, 0, height());
        painter.drawLine(width(), 0, width(), height());
        painter.restore();
    }
}

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    titleBarHeight = framelessHelper()->getTitleBarHeight();
    createWinId();
    setupUi();
    initBackgroundWindow();
    installEventFilter(this);
}

bool Widget::isNormal() const
{
    return (!isMinimized() && !isMaximized() && !isFullScreen());
}

void Widget::setupUi()
{
    contentsWidget = new ContentsWidget(this);
    contentsWidget->setObjectName(QLatin1String("contentsWidget"));
    contentsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const QSize systemButtonSize = {qRound(titleBarHeight * 1.5), titleBarHeight};
    const QSize systemIconSize = {17, 17};
    titleBarWidget = new QWidget(contentsWidget);
    titleBarWidget->setObjectName(QLatin1String("titleBarWidget"));
    titleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleBarWidget->setFixedHeight(titleBarHeight);
    windowIconButton = new QPushButton(this);
    windowIconButton->setObjectName(QLatin1String("windowIconButton"));
    windowIconButton->setFixedSize(systemIconSize);
    windowIconButton->setIconSize(systemIconSize);
    connect(this, &Widget::windowIconChanged, windowIconButton, &QPushButton::setIcon);
    windowTitleLabel = new QLabel(this);
    QFont f = font();
    f.setPointSizeF(8.63);
    windowTitleLabel->setFont(f);
    connect(this, &Widget::windowTitleChanged, windowTitleLabel, &QLabel::setText);
    minimizeButton = new QPushButton(this);
    minimizeButton->setObjectName(QLatin1String("minimizeButton"));
    minimizeButton->setFixedSize(systemButtonSize);
    minimizeButton->setIconSize(systemButtonSize);
    minimizeButton->setIcon(QIcon(QLatin1String(":/images/button_minimize_black.svg")));
    connect(minimizeButton, &QPushButton::clicked, this, &Widget::showMinimized);
    maximizeButton = new QPushButton(this);
    maximizeButton->setObjectName(QLatin1String("maximizeButton"));
    maximizeButton->setFixedSize(systemButtonSize);
    maximizeButton->setIconSize(systemButtonSize);
    maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_maximize_black.svg")));
    connect(maximizeButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    closeButton = new QPushButton(this);
    closeButton->setObjectName(QLatin1String("closeButton"));
    closeButton->setFixedSize(systemButtonSize);
    closeButton->setIconSize(systemButtonSize);
    closeButton->setIcon(QIcon(QLatin1String(":/images/button_close_black.svg")));
    connect(closeButton, &QPushButton::clicked, this, &Widget::close);
    const auto titleBarWidgetLayout = new QHBoxLayout(titleBarWidget);
    titleBarWidgetLayout->setSpacing(0);
    titleBarWidgetLayout->setContentsMargins(0, 0, 0, 0);
    titleBarWidgetLayout->addSpacerItem(
        new QSpacerItem(7, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    titleBarWidgetLayout->addWidget(windowIconButton);
    titleBarWidgetLayout->addSpacerItem(
        new QSpacerItem(3, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    titleBarWidgetLayout->addWidget(windowTitleLabel);
    titleBarWidgetLayout->addStretch();
    titleBarWidgetLayout->addWidget(minimizeButton);
    titleBarWidgetLayout->addWidget(maximizeButton);
    titleBarWidgetLayout->addWidget(closeButton);
    titleBarWidget->setLayout(titleBarWidgetLayout);
    const auto contentsWidgetLayout = new QVBoxLayout(contentsWidget);
    contentsWidgetLayout->setSpacing(0);
    contentsWidgetLayout->setContentsMargins(1, 1, 1, 0);
    contentsWidgetLayout->addWidget(titleBarWidget);
    contentsWidgetLayout->addStretch();
    contentsWidget->setLayout(contentsWidgetLayout);
    const auto backgroundWindowLayout = new QVBoxLayout(this);
    backgroundWindowLayout->setSpacing(0);
    backgroundWindowLayout->addWidget(contentsWidget);
    setLayout(backgroundWindowLayout);
    setStyleSheet(QLatin1String(R"(
#contentsWidget {
  background-color: #f0f0f0;
}

#titleBarWidget {
  background-color: white;
}

#windowIconButton, #minimizeButton, #maximizeButton, #closeButton {
  border-style: none;
  background-color: transparent;
}

#minimizeButton:hover, #maximizeButton:hover {
  background-color: #80c7c7c7;
}

#minimizeButton:pressed, #maximizeButton:pressed {
  background-color: #80808080;
}

#closeButton:hover {
  background-color: #e81123;
}

#closeButton:pressed {
  background-color: #8c0a15;
}
)"));
}

void Widget::initBackgroundWindow()
{
    QWindow *win = windowHandle();
    framelessHelper()->removeWindowFrame(win);
    framelessHelper()->addIgnoreObject(win, windowIconButton);
    framelessHelper()->addIgnoreObject(win, minimizeButton);
    framelessHelper()->addIgnoreObject(win, maximizeButton);
    framelessHelper()->addIgnoreObject(win, closeButton);
    framelessHelper()->setTitleBarHeight(titleBarHeight + framelessHelper()->getBorderHeight());
    //setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    contentsWidget->setGraphicsEffect(shadowEffect);
    setFrameShadowEnabled();
    setFrameShadowActive();
}

void Widget::setFrameShadowEnabled(const bool enable)
{
    if (enable) {
        const int bw = framelessHelper()->getBorderWidth();
        const int bh = framelessHelper()->getBorderHeight();
        layout()->setContentsMargins(bw, bh, bw, bh);
        shadowEffect->setEnabled(true);
    } else {
        shadowEffect->setEnabled(false);
        layout()->setContentsMargins(0, 0, 0, 0);
    }
}

void Widget::setFrameShadowActive(const bool active)
{
    if (active) {
        shadowEffect->setColor({0, 0, 0, 80});
        shadowEffect->setBlurRadius(25);
    } else {
        shadowEffect->setColor({0, 0, 0, 60});
        shadowEffect->setBlurRadius(20);
    }
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (object == this) {
        switch (event->type()) {
        case QEvent::WindowStateChange: {
            const bool normal = isNormal();
            contentsWidget->setShouldDrawWindowBorder(normal);
            setFrameShadowEnabled(normal);
            framelessHelper()->setTitleBarHeight(
                titleBarHeight + (normal ? framelessHelper()->getBorderHeight() : 0));
            const QLatin1String maxIconPath = QLatin1String{":/images/button_maximize_black.svg"};
            const QLatin1String restoreIconPath = QLatin1String{
                ":/images/button_restore_black.svg"};
            maximizeButton->setIcon(QIcon(normal ? maxIconPath : restoreIconPath));
        } break;
        case QEvent::WindowActivate: {
            if (isNormal()) {
                setFrameShadowActive();
            }
        } break;
        case QEvent::WindowDeactivate: {
            if (isNormal()) {
                setFrameShadowActive(false);
            }
        } break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(object, event);
}
