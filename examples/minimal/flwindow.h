#pragma once

#include <QWidget>

class QPushButton;

class FLWindow : public QWidget
{
	Q_OBJECT
public:
	explicit FLWindow(QWidget *parent = nullptr);
	~FLWindow();

protected:
	void showEvent(QShowEvent *event) override;

private:
	void initFramelessWindow();
	void setupUi();

private:
	QWidget *m_titleBarWidget = nullptr;
    QPushButton *m_minimizeButton = nullptr;
    QPushButton *m_maximizeButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};