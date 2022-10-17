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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfileiconprovider.h>
#include <Utils>
#include <StandardTitleBar>
#include <StandardSystemButton>
#include <FramelessWidgetsHelper>
#include "../shared/settings.h"

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)
FRAMELESSHELPER_STRING_CONSTANT(State)

MainWindow::MainWindow(QWidget *parent, const Qt::WindowFlags flags) : FramelessMainWindow(parent, flags)
{
    initialize();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    Settings::set({}, kGeometry, saveGeometry());
    Settings::set({}, kState, saveState());
    FramelessMainWindow::closeEvent(event);
}

void MainWindow::initialize()
{
    m_titleBar.reset(new StandardTitleBar(this));
    m_titleBar->setTitleLabelAlignment(Qt::AlignCenter);
    m_mainWindow.reset(new Ui::MainWindow);
    m_mainWindow->setupUi(this);

    QMenuBar * const mb = menuBar();
    mb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mb->setStyleSheet(FRAMELESSHELPER_STRING_LITERAL(R"(
QMenuBar {
    background-color: transparent;
}

QMenuBar::item {
    background: transparent;
}

QMenuBar::item:selected {
    background: #a8a8a8;
}

QMenuBar::item:pressed {
    background: #888888;
}
    )"));
    const auto titleBarLayout = static_cast<QHBoxLayout *>(m_titleBar->layout());
    titleBarLayout->insertWidget(0, mb);

    // setMenuWidget(): make the menu widget become the first row of the window.
    setMenuWidget(m_titleBar.data());

    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(m_titleBar.data());
    helper->setSystemButton(m_titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(m_titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(m_titleBar->closeButton(), SystemButtonType::Close);
    helper->setHitTestVisible(mb); // IMPORTANT!
    connect(helper, &FramelessWidgetsHelper::ready, this, [this, helper](){
        const QByteArray geoData = Settings::get({}, kGeometry);
        const QByteArray stateData = Settings::get({}, kState);
        if (geoData.isEmpty()) {
            helper->moveWindowToDesktopCenter();
        } else {
            restoreGeometry(geoData);
        }
        if (!stateData.isEmpty()) {
            restoreState(stateData);
        }
    });

    setWindowTitle(tr("FramelessHelper demo application - Qt MainWindow"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));
}
