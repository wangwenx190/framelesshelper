#pragma once

#include <QWidget>

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
};