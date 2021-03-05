TARGET = Quick
TEMPLATE = app
QT += quick quickcontrols2
CONFIG(release, debug|release): CONFIG += qtquickcompiler
SOURCES += main.cpp
RESOURCES += qml.qrc
include($$PWD/../common.pri)
