TARGET = Win32Demo
TEMPLATE = app
QT += gui-private widgets
HEADERS += widget.h
SOURCES += widget.cpp main.cpp
include($$PWD/../common.pri)
