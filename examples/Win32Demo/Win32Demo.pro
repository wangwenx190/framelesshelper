TARGET = Win32Demo
TEMPLATE = app
QT += widgets
HEADERS += widget.h
SOURCES += widget.cpp main.cpp
FORMS += widget.ui
include($$PWD/../common.pri)
