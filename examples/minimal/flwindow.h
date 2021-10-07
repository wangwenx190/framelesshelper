#pragma once

#include <QWidget>
#include "core/framelesshelper.h"

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

#ifdef Q_OS_MAC
    void resizeEvent(QResizeEvent *event) override;
#endif // Q_OS_MAC

private:
	void initFramelessWindow();
	void setupUi();

private:
	__flh_ns::FramelessHelper *m_helper = nullptr;
	QWidget *m_titleBarWidget = nullptr;
    QPushButton *m_minimizeButton = nullptr;
    QPushButton *m_maximizeButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};
