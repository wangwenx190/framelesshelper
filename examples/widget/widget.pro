TEMPLATE = app
TARGET = FramelessHelperDemo-Widget
QT += widgets
HEADERS += \
    ../shared/log.h \
    ../shared/settings.h \
    widget.h
SOURCES += \
    ../shared/log.cpp \
    ../shared/settings.cpp \
    widget.cpp \
    main.cpp
include(../../qmake/core.pri)
include(../../qmake/widgets.pri)
