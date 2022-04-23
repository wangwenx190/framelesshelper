TARGET = RenderControl
TEMPLATE = app
QT += quick qml opengl
SOURCES += main.cpp window_singlethreaded.cpp
HEADERS += window_singlethreaded.h
RESOURCES += rendercontrol.qrc
include($$PWD/../common.pri)
