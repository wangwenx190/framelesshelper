#include "flwindow.h"
#include "../../framelesshelper.h"

FRAMELESSHELPER_USE_NAMESPACE

FLWindow::FLWindow(QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	setStyleSheet(QString::fromLatin1("background:blue"));

	setAttribute(Qt::WA_ShowModal);
	resize(500, 500);
}

FLWindow::~FLWindow()
{

}

void FLWindow::initFramelessWindow()
{
	FramelessHelper* helper = new FramelessHelper(windowHandle());
	helper->setResizeBorderThickness(8);
	helper->setTitleBarHeight(20);
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