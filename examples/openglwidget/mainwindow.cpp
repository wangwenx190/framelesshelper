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
#include "glwidget.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfileiconprovider.h>
#include <FramelessWidgetsHelper>
#include <StandardTitleBar>
#include <StandardSystemButton>
#include "../shared/settings.h"

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)

MainWindow::MainWindow(QWidget *parent) : FramelessWidget(parent)
{
    initialize();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    Settings::set({}, kGeometry, saveGeometry());
    FramelessWidget::closeEvent(event);
}

void MainWindow::initialize()
{
    setWindowTitle(tr("FramelessHelper demo application - QOpenGLWidget"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));
    resize(800, 600);
    m_titleBar = new StandardTitleBar(this);
    m_titleBar->setWindowIconVisible(true);
    m_glWidget = new GLWidget(this);
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_glWidget);
    setLayout(mainLayout);

    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(m_titleBar);
    helper->setSystemButton(m_titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(m_titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(m_titleBar->closeButton(), SystemButtonType::Close);
    connect(helper, &FramelessWidgetsHelper::ready, this, [this, helper](){
        const QByteArray data = Settings::get({}, kGeometry);
        if (data.isEmpty()) {
            helper->moveWindowToDesktopCenter();
        } else {
            restoreGeometry(data);
        }
    });
}
