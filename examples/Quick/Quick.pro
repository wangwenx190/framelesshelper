TARGET = Quick
TEMPLATE = app
QT += quick
HEADERS += $$PWD/../../framelessquickhelper.h
SOURCES += $$PWD/../../framelessquickhelper.cpp main.cpp
RESOURCES += qml.qrc
include($$PWD/../common.pri)
