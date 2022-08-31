TEMPLATE = app
TARGET = Quick
QT += qml quick quickcontrols2
CONFIG -= embed_manifest_exe
DEFINES += QUICK_USE_QMAKE
HEADERS += settings.h
SOURCES += settings.cpp main.cpp
RESOURCES += resources.qrc
win32: RC_FILE = ../example.rc
include(../../qmake/core.pri)
include(../../qmake/quick.pri)
