TARGET = RenderControl
TEMPLATE = app
QT += quick qml opengl quickcontrols2
SOURCES += main.cpp window_singlethreaded.cpp
HEADERS += window_singlethreaded.h
RESOURCES += rendercontrol.qrc
include($$PWD/../common.pri)
