TARGET = Win32Demo
TEMPLATE = app
QT += widgets
LIBS += -luser32 -ldwmapi
HEADERS += widget.h
SOURCES += widget.cpp main.cpp
FORMS += widget.ui
include($$PWD/../common.pri)
