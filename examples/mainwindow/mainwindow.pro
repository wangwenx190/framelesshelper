TARGET = MainWindow
TEMPLATE = app
QT += widgets
HEADERS += mainwindow.h
SOURCES += mainwindow.cpp main.cpp
FORMS += TitleBar.ui MainWindow.ui
include($$PWD/../common.pri)
