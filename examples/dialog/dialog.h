// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <FramelessHelper/Widgets/framelessdialog.h>

FRAMELESSHELPER_REQUIRE_CONFIG(window)

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

#if FRAMELESSHELPER_CONFIG(titlebar)
FRAMELESSHELPER_BEGIN_NAMESPACE
class StandardTitleBar;
FRAMELESSHELPER_END_NAMESPACE
#endif

class Dialog : public FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessDialog)
{
    Q_OBJECT
    Q_DISABLE_COPY(Dialog)

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    void waitReady();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();

private:
#if FRAMELESSHELPER_CONFIG(titlebar)
    FRAMELESSHELPER_PREPEND_NAMESPACE(StandardTitleBar) *titleBar = nullptr;
#endif
    QLabel *label = nullptr;
    QLineEdit *lineEdit = nullptr;
    QCheckBox *caseCheckBox = nullptr;
    QCheckBox *fromStartCheckBox = nullptr;
    QCheckBox *wholeWordsCheckBox = nullptr;
    QCheckBox *searchSelectionCheckBox = nullptr;
    QCheckBox *backwardCheckBox = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    QPushButton *findButton = nullptr;
    QPushButton *moreButton = nullptr;
    QWidget *extension = nullptr;
};
