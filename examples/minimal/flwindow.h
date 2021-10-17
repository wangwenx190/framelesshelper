#pragma once

#include <QWidget>
#include "widget/framelesswindow.h"

class QPushButton;

class FLWindow : public __flh_ns::FramelessWindow<QWidget>
{
	Q_OBJECT
public:
	explicit FLWindow(QWidget *parent = nullptr);
    ~FLWindow() override;

private:
	void setupUi();

private:
	QWidget *m_titleBarWidget = nullptr;
    QPushButton *m_minimizeButton = nullptr;
    QPushButton *m_maximizeButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};
