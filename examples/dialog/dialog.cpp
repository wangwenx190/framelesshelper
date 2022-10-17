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
#include <QtWidgets/qmessagebox.h>
#include <StandardTitleBar>
#include <FramelessWidgetsHelper>
#include <StandardSystemButton>
#include <private/framelesswidgetshelper_p.h>
#include "../shared/settings.h"

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)

Dialog::Dialog(QWidget *parent) : FramelessDialog(parent)
{
    setupUi();
}

Dialog::~Dialog() = default;

void Dialog::closeEvent(QCloseEvent *event)
{
    Settings::set({}, kGeometry, saveGeometry());
    FramelessDialog::closeEvent(event);
}

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
    connect(findButton, &QPushButton::clicked, this, [this](){
        const QString text = lineEdit->text();
        if (text.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("You didn't enter anything in the search box."));
        } else {
            QMessageBox::information(this, tr("Result"), tr("You wanted to find: \"%1\".").arg(text));
        }
    });

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
    // Special hack to disable the overriding of the mouse cursor, it's totally different
    // with making the window un-resizable: we still want the window be able to resize
    // programatically, but we also want the user not able to resize the window manually.
    // So apparently we can't use QWidget::setFixedWidth/Height/Size() here.
    FramelessWidgetsHelperPrivate::get(helper)->setProperty(kDontOverrideCursorVar, true);
    connect(helper, &FramelessWidgetsHelper::ready, this, [this, helper](){
        const QByteArray data = Settings::get({}, kGeometry);
        if (data.isEmpty()) {
            helper->moveWindowToDesktopCenter();
        } else {
            restoreGeometry(data);
        }
    });
}
