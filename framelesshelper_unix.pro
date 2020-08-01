TARGET = framelessapplication
CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,_debug)
TEMPLATE = app
QT += gui-private
qtHaveModule(widgets): QT += widgets
qtHaveModule(quick) {
    QT += quick
    HEADERS += framelessquickhelper.h
    SOURCES += framelessquickhelper.cpp
}
CONFIG += c++17 strict_c++ warn_on utf8_source
DEFINES += \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    FRAMELESSHELPER_STATIC
VERSION = 1.0.0
HEADERS += framelesshelper_global.h framelesshelper.h
SOURCES += framelesshelper.cpp main_unix.cpp
