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
#include <FramelessHelper/Widgets/standardtitlebar.h>
#include <FramelessHelper/Widgets/framelesswidgetshelper.h>
#include <FramelessHelper/Widgets/standardsystembutton.h>
#include <FramelessHelper/Widgets/private/framelesswidgetshelper_p.h>
#include "../shared/settings.h"

extern template void Settings::set<QRect>(const QString &, const QString &, const QRect &);
extern template void Settings::set<qreal>(const QString &, const QString &, const qreal &);

extern template QRect Settings::get<QRect>(const QString &, const QString &);
extern template qreal Settings::get<qreal>(const QString &, const QString &);

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)
FRAMELESSHELPER_STRING_CONSTANT(DevicePixelRatio)

Dialog::Dialog(QWidget *parent) : FramelessDialog(parent)
{
    setupUi();
}

Dialog::~Dialog() = default;

void Dialog::closeEvent(QCloseEvent *event)
{
    if (!parent()) {
        const QString id = objectName();
        Settings::set(id, kGeometry, geometry());
        Settings::set(id, kDevicePixelRatio, devicePixelRatioF());
    }
    FramelessDialog::closeEvent(event);
}

void Dialog::setupUi()
{
    setWindowTitle(tr("FramelessHelper demo application - QDialog"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));
    connect(this, &Dialog::objectNameChanged, this, [this](const QString &name){
        if (name.isEmpty()) {
            return;
        }
        setWindowTitle(windowTitle() + FRAMELESSHELPER_STRING_LITERAL(" [%1]").arg(name));
    });

#if FRAMELESSHELPER_CONFIG(titlebar)
    titleBar = new StandardTitleBar(this);
    titleBar->setWindowIconVisible(true);
#  if (!defined(Q_OS_MACOS) && FRAMELESSHELPER_CONFIG(system_button))
    titleBar->maximizeButton()->hide();
#  endif
#endif

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

    QVBoxLayout *extensionLayout = new QVBoxLayout(extension);
    extensionLayout->setContentsMargins(0, 0, 0, 0);
    extensionLayout->addWidget(wholeWordsCheckBox);
    extensionLayout->addWidget(backwardCheckBox);
    extensionLayout->addWidget(searchSelectionCheckBox);

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

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
#if FRAMELESSHELPER_CONFIG(titlebar)
    mainLayout->addWidget(titleBar);
#endif
    mainLayout->addLayout(controlsLayout);

    extension->hide();

#if FRAMELESSHELPER_CONFIG(titlebar)
    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(titleBar);
#  ifdef Q_OS_WINDOWS
    FramelessWidgetsHelperPrivate *helperPriv = FramelessWidgetsHelperPrivate::get(helper);
    helperPriv->setProperty(kSysMenuRemoveRestoreVar, true);
    helperPriv->setProperty(kSysMenuRemoveSizeVar, true);
    helperPriv->setProperty(kSysMenuRemoveMinimizeVar, true);
    helperPriv->setProperty(kSysMenuRemoveMaximizeVar, true);
    helperPriv->setProperty(kSysMenuRemoveSeparatorVar, true);
#  endif
#  if (!defined(Q_OS_MACOS) && FRAMELESSHELPER_CONFIG(system_button))
    helper->setSystemButton(titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(titleBar->closeButton(), SystemButtonType::Close);
#  endif
#endif
}

void Dialog::waitReady()
{
    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->waitForReady();
    const QString id = objectName();
    const auto savedGeometry = Settings::get<QRect>(id, kGeometry);
    if (savedGeometry.isValid() && !parent()) {
        const auto savedDpr = Settings::get<qreal>(id, kDevicePixelRatio);
        // Qt doesn't support dpr < 1.
        const qreal oldDpr = std::max(savedDpr, qreal(1));
        const qreal scale = (devicePixelRatioF() / oldDpr);
        setGeometry({savedGeometry.topLeft() * scale, savedGeometry.size() * scale});
    } else {
        helper->moveWindowToDesktopCenter();
    }
}
