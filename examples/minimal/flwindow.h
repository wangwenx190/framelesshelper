#pragma once

#include <QWidget>
#include "framelesshelper.h"

class QPushButton;

class FLWindow : public QWidget
{
	Q_OBJECT
public:
	explicit FLWindow(QWidget *parent = nullptr);
	~FLWindow();

protected:
	void showEvent(QShowEvent *event) override;
#ifdef Q_OS_WIN
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif // Q_OS_WIN

private:
	void initFramelessWindow();
	void setupUi();

private:
	__flh_ns::FramelessHelper *m_flsHelper = nullptr;
	QWidget *m_titleBarWidget = nullptr;
    QPushButton *m_minimizeButton = nullptr;
    QPushButton *m_maximizeButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};