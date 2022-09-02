// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dialog.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfileiconprovider.h>
#include <StandardTitleBar>
#include <FramelessWidgetsHelper>
#include <StandardSystemButton>
#include <private/framelesswidgetshelper_p.h>

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

Dialog::Dialog(QWidget *parent) : FramelessDialog(parent)
{
    setupUi();
}

Dialog::~Dialog() = default;

void Dialog::setupUi()
{
    setWindowTitle(tr("Qt Dialog demo"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));

    titleBar = new StandardTitleBar(this);
    titleBar->setWindowIconVisible(true);
    titleBar->maximizeButton()->hide();

    label = new QLabel(tr("Find &what:"));
    lineEdit = new QLineEdit;
    label->setBuddy(lineEdit);

    caseCheckBox = new QCheckBox(tr("Match &case"));
    fromStartCheckBox = new QCheckBox(tr("Search from &start"));
    fromStartCheckBox->setChecked(true);

    findButton = new QPushButton(tr("&Find"));
    findButton->setDefault(true);

    moreButton = new QPushButton(tr("&More"));
    moreButton->setCheckable(true);
    moreButton->setAutoDefault(false);

    extension = new QWidget;

    wholeWordsCheckBox = new QCheckBox(tr("&Whole words"));
    backwardCheckBox = new QCheckBox(tr("Search &backward"));
    searchSelectionCheckBox = new QCheckBox(tr("Search se&lection"));

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(findButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);

    connect(moreButton, &QPushButton::toggled, extension, &QWidget::setVisible);

    QVBoxLayout *extensionLayout = new QVBoxLayout;
    extensionLayout->setContentsMargins(0, 0, 0, 0);
    extensionLayout->addWidget(wholeWordsCheckBox);
    extensionLayout->addWidget(backwardCheckBox);
    extensionLayout->addWidget(searchSelectionCheckBox);
    extension->setLayout(extensionLayout);

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label);
    topLeftLayout->addWidget(lineEdit);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(caseCheckBox);
    leftLayout->addWidget(fromStartCheckBox);

    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->setContentsMargins(11, 11, 11, 11);
    controlsLayout->addLayout(leftLayout, 0, 0);
    controlsLayout->addWidget(buttonBox, 0, 1);
    controlsLayout->addWidget(extension, 1, 0, 1, 2);
    controlsLayout->setRowStretch(2, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(titleBar);
    mainLayout->addLayout(controlsLayout);

    setLayout(mainLayout);

    extension->hide();

    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(titleBar);
    helper->setSystemButton(titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(titleBar->closeButton(), SystemButtonType::Close);
    FramelessWidgetsHelperPrivate::get(helper)->setProperty(FRAMELESSHELPER_BYTEARRAY_LITERAL("FRAMELESSHELPER_DONT_OVERRIDE_CURSOR"), true);
}
