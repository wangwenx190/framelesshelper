// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <FramelessDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

FRAMELESSHELPER_BEGIN_NAMESPACE
class StandardTitleBar;
FRAMELESSHELPER_END_NAMESPACE

class Dialog : public FRAMELESSHELPER_PREPEND_NAMESPACE(FramelessDialog)
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Dialog)

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();

private:
    FRAMELESSHELPER_PREPEND_NAMESPACE(StandardTitleBar) *titleBar = nullptr;
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
