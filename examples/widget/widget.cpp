/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtCore/qdatetime.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/qguiapplication.h>
#include "../../utilities.h"
#include "../../framelesswindowsmanager.h"

Widget::Widget(QWidget *parent) : QtAcrylicWidget(parent)
{
    createWinId();
    setupUi();
    startTimer(500);
}

Widget::~Widget() = default;

void Widget::moveToDesktopCenter()
{
    const QSize ss = Utilities::getScreenGeometry(nullptr).size();
    const int newX = (ss.width() - width()) / 2;
    const int newY = (ss.height() - height()) / 2;
    move(newX, newY);
}

void Widget::showEvent(QShowEvent *event)
{
    QtAcrylicWidget::showEvent(event);
    static bool inited = false;
    if (!inited) {
        FramelessWindowsManager::addWindow(windowHandle());
        setAcrylicEnabled(true);
        inited = true;
    }
}

void Widget::timerEvent(QTimerEvent *event)
{
    QtAcrylicWidget::timerEvent(event);
    m_label->setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
}

void Widget::changeEvent(QEvent *event)
{
    QtAcrylicWidget::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized() || isFullScreen()) {
            layout()->setContentsMargins(0, 0, 0, 0);
            m_maximizeButton->setIcon(QIcon{QStringLiteral(":/images/button_restore_black.svg")});
        } else if (!isMinimized()) {
            layout()->setContentsMargins(1, 1, 1, 1);
            m_maximizeButton->setIcon(QIcon{QStringLiteral(":/images/button_maximize_black.svg")});
        }
    }
}

void Widget::setupUi()
{
    const QWindow *win = windowHandle();
    const int titleBarHeight = Utilities::getSystemMetric(win, Utilities::SystemMetric::TitleBarHeight, false);
    const QSize systemButtonSize = {qRound(titleBarHeight * 1.5), titleBarHeight};
    m_minimizeButton = new QPushButton(this);
    m_minimizeButton->setObjectName(QStringLiteral("MinimizeButton"));
    m_minimizeButton->setFixedSize(systemButtonSize);
    m_minimizeButton->setIcon(QIcon{QStringLiteral(":/images/button_minimize_black.svg")});
    m_minimizeButton->setIconSize(systemButtonSize);
    connect(m_minimizeButton, &QPushButton::clicked, this, &Widget::showMinimized);
    m_maximizeButton = new QPushButton(this);
    m_maximizeButton->setObjectName(QStringLiteral("MaximizeButton"));
    m_maximizeButton->setFixedSize(systemButtonSize);
    m_maximizeButton->setIcon(QIcon{QStringLiteral(":/images/button_maximize_black.svg")});
    m_maximizeButton->setIconSize(systemButtonSize);
    connect(m_maximizeButton, &QPushButton::clicked, this, [this](){
        if (isMaximized() || isFullScreen()) {
            showNormal();
            m_maximizeButton->setIcon(QIcon{QStringLiteral(":/images/button_maximize_black.svg")});
        } else {
            showMaximized();
            m_maximizeButton->setIcon(QIcon{QStringLiteral(":/images/button_restore_black.svg")});
        }
    });
    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName(QStringLiteral("CloseButton"));
    m_closeButton->setFixedSize(systemButtonSize);
    m_closeButton->setIcon(QIcon{QStringLiteral(":/images/button_close_black.svg")});
    m_closeButton->setIconSize(systemButtonSize);
    connect(m_closeButton, &QPushButton::clicked, this, &Widget::close);
    FramelessWindowsManager::addIgnoreObject(win, m_minimizeButton);
    FramelessWindowsManager::addIgnoreObject(win, m_maximizeButton);
    FramelessWindowsManager::addIgnoreObject(win, m_closeButton);
    const auto systemButtonLayout = new QHBoxLayout;
    systemButtonLayout->setContentsMargins(0, 0, 0, 0);
    systemButtonLayout->setSpacing(0);
    systemButtonLayout->addStretch();
    systemButtonLayout->addWidget(m_minimizeButton);
    systemButtonLayout->addWidget(m_maximizeButton);
    systemButtonLayout->addWidget(m_closeButton);
    m_label = new QLabel(this);
    QFont font = QGuiApplication::font();
    font.setBold(true);
    font.setPointSize(70);
    m_label->setFont(font);
    m_label->setFrameShape(QFrame::NoFrame);
    const auto contentLayout = new QHBoxLayout;
    contentLayout->addStretch();
    contentLayout->addWidget(m_label);
    contentLayout->addStretch();
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(systemButtonLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);
    setStyleSheet(QStringLiteral(R"(
#MinimizeButton, #MaximizeButton, #CloseButton {
  border-style: none;
  background-color: transparent;
}

#MinimizeButton:hover, #MaximizeButton:hover {
  background-color: #80c7c7c7;
}

#MinimizeButton:pressed, #MaximizeButton:pressed {
  background-color: #80808080;
}

#CloseButton:hover {
  background-color: #e81123;
}

#CloseButton:pressed {
  background-color: #8c0a15;
}
)"));
}
