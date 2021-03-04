TARGET = Quick
TEMPLATE = app
QT += quick quickcontrols2
HEADERS += ../../framelessquickhelper.h
SOURCES += ../../framelessquickhelper.cpp main.cpp
RESOURCES += qml.qrc
include($$PWD/../common.pri)
