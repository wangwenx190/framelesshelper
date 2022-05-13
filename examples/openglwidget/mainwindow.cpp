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
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qboxlayout.h>
#include <FramelessWidgetsHelper>
#include <StandardTitleBar>
#include <StandardSystemButton>

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT2(IniKeyPath, "Window/Geometry")

[[nodiscard]] static inline QSettings *appConfigFile()
{
    const QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    const QString iniFileName = fileInfo.completeBaseName() + FRAMELESSHELPER_STRING_LITERAL(".ini");
    const QString iniFilePath = fileInfo.canonicalPath() + QDir::separator() + iniFileName;
    const auto settings = new QSettings(iniFilePath, QSettings::IniFormat);
    return settings;
}

MainWindow::MainWindow(QWidget *parent) : FramelessWidget(parent)
{
    initialize();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    const QScopedPointer<QSettings> settings(appConfigFile());
    settings->setValue(kIniKeyPath, saveGeometry());
    FramelessWidget::closeEvent(event);
}

void MainWindow::initialize()
{
    resize(800, 600);
    setWindowTitle(tr("FramelessHelper demo application - QOpenGLWidget"));
    m_titleBar = new StandardTitleBar(this);
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
        const QScopedPointer<QSettings> settings(appConfigFile());
        const QByteArray data = settings->value(kIniKeyPath).toByteArray();
        if (data.isEmpty()) {
            helper->moveWindowToDesktopCenter();
        } else {
            restoreGeometry(data);
        }
    });
}
