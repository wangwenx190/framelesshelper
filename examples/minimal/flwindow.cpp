#include "flwindow.h"
#include "../../framelesshelper.h"

#include <QScreen>

FRAMELESSHELPER_USE_NAMESPACE

FLWindow::FLWindow(QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setStyleSheet(QString::fromLatin1("background:blue"));

	move(screen()->geometry().center() - frameGeometry().center());
}

FLWindow::~FLWindow()
{

}

void FLWindow::initFramelessWindow()
{
	FramelessHelper* helper = new FramelessHelper(windowHandle());
	helper->setResizeBorderThickness(8);
	helper->setTitleBarHeight(40);
	helper->setResizable(true);
	helper->install();
}

void FLWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    static bool inited = false;
    if (!inited) {
        inited = true;
		initFramelessWindow();
    }
}