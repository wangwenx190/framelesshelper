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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qshortcut.h>
#else
#  include <QtWidgets/qshortcut.h>
#endif
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfileiconprovider.h>
#include <FramelessManager>
#include <Utils>
#include <FramelessWidgetsHelper>
#include <StandardTitleBar>
#include <StandardSystemButton>
#include "../shared/settings.h"

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)

Widget::Widget(QWidget *parent) : FramelessWidget(parent)
{
    initialize();
    startTimer(500);
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged, this, &Widget::updateStyleSheet);
}

Widget::~Widget() = default;

void Widget::timerEvent(QTimerEvent *event)
{
    FramelessWidget::timerEvent(event);
    if (m_clockLabel) {
        m_clockLabel->setText(QTime::currentTime().toString(FRAMELESSHELPER_STRING_LITERAL("hh:mm:ss")));
    }
}

void Widget::closeEvent(QCloseEvent *event)
{
    Settings::set(objectName(), kGeometry, saveGeometry());
    FramelessWidget::closeEvent(event);
}

void Widget::initialize()
{
    setWindowTitle(tr("FramelessHelper demo application - Qt Widgets"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));
    resize(800, 600);
    m_titleBar.reset(new StandardTitleBar(this));
    m_titleBar->setWindowIconVisible(true);
    m_clockLabel.reset(new QLabel(this));
    m_clockLabel->setFrameShape(QFrame::NoFrame);
    QFont clockFont = font();
    clockFont.setBold(true);
    clockFont.setPointSize(70);
    m_clockLabel->setFont(clockFont);
    const auto contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_clockLabel.data());
    contentLayout->addStretch();
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_titleBar.data());
    mainLayout->addLayout(contentLayout);
    setLayout(mainLayout);
    updateStyleSheet();

    m_cancelShortcut.reset(new QShortcut(this));
    m_cancelShortcut->setKey(FRAMELESSHELPER_STRING_LITERAL("ESC"));
    connect(m_cancelShortcut.data(), &QShortcut::activated, this, [this](){
        if (isFullScreen()) {
            Q_EMIT m_fullScreenShortcut->activated();
        } else {
            close();
        }
    });

    m_fullScreenShortcut.reset(new QShortcut(this));
    m_fullScreenShortcut->setKey(FRAMELESSHELPER_STRING_LITERAL("ALT+RETURN"));
    connect(m_fullScreenShortcut.data(), &QShortcut::activated, this, [this](){
        if (isFullScreen()) {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        } else {
            showFullScreen();
        }
    });

    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(m_titleBar.data());
    helper->setSystemButton(m_titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(m_titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(m_titleBar->closeButton(), SystemButtonType::Close);
    connect(helper, &FramelessWidgetsHelper::ready, this, [this, helper](){
        const QByteArray data = Settings::get(objectName(), kGeometry);
        if (data.isEmpty()) {
            helper->moveWindowToDesktopCenter();
        } else {
            restoreGeometry(data);
        }
    });
}

void Widget::updateStyleSheet()
{
    const bool dark = Utils::shouldAppsUseDarkMode();
    const QColor clockLabelTextColor = (dark ? kDefaultWhiteColor : kDefaultBlackColor);
    m_clockLabel->setStyleSheet(FRAMELESSHELPER_STRING_LITERAL("background-color: transparent; color: %1;")
                 .arg(clockLabelTextColor.name()));
    if (FramelessWidgetsHelper::get(this)->isBlurBehindWindowEnabled()) {
        setStyleSheet(FRAMELESSHELPER_STRING_LITERAL("background-color: transparent;"));
    } else {
        const QColor windowBackgroundColor = (dark ? kDefaultSystemDarkColor : kDefaultSystemLightColor);
        setStyleSheet(FRAMELESSHELPER_STRING_LITERAL("background-color: %1;").arg(windowBackgroundColor.name()));
    }
    update();
}
