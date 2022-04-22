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
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

MainWindow::MainWindow(QWidget *parent) : FramelessWidget(parent)
{
    setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::updateMaximizeButton()
{
    m_maxBtn->setText(isZoomed() ? tr("RESTORE") : tr("MAXIMIZE"));
}

void MainWindow::setupUi()
{
    m_titleLabel.reset(new QLabel(this));
    QFont f = font();
    f.setPointSize(kDefaultTitleBarFontPointSize);
    m_titleLabel->setFont(f);
    connect(this, &MainWindow::windowTitleChanged, m_titleLabel.data(), &QLabel::setText);
    m_minBtn.reset(new QPushButton(this));
    m_minBtn->setText(tr("MINIMIZE"));
    connect(m_minBtn.data(), &QPushButton::clicked, this, &MainWindow::showMinimized);
    m_maxBtn.reset(new QPushButton(this));
    updateMaximizeButton();
    connect(m_maxBtn.data(), &QPushButton::clicked, this, &MainWindow::toggleMaximized);
    connect(this, &MainWindow::zoomedChanged, this, &MainWindow::updateMaximizeButton);
    m_closeBtn.reset(new QPushButton(this));
    m_closeBtn->setText(tr("CLOSE"));
    connect(m_closeBtn.data(), &QPushButton::clicked, this, &MainWindow::close);
    m_titleBarWidget.reset(new QWidget(this));
    m_titleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBarWidget->setFixedHeight(kDefaultTitleBarHeight);
    const auto titleBarLayout = new QHBoxLayout(m_titleBarWidget.data());
    titleBarLayout->setSpacing(0);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->addSpacerItem(new QSpacerItem(kDefaultTitleBarTitleLabelMargin, kDefaultTitleBarTitleLabelMargin));
    titleBarLayout->addWidget(m_titleLabel.data());
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(m_minBtn.data());
    titleBarLayout->addWidget(m_maxBtn.data());
    titleBarLayout->addWidget(m_closeBtn.data());
    m_titleBarWidget->setLayout(titleBarLayout);
    m_glWidget.reset(new GLWidget(this));
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_titleBarWidget.data());
    mainLayout->addWidget(m_glWidget.data());
    setLayout(mainLayout);
    setTitleBarWidget(m_titleBarWidget.data());
    resize(800, 600);
    setWindowTitle(tr("QOpenGLWidget demo"));
}
