TARGET = framelesswidget
TEMPLATE = app
QT += gui-private widgets
CONFIG += c++17 strict_c++ utf8_source warn_on
win32 {
    DEFINES += WIN32_LEAN_AND_MEAN
    CONFIG += windeployqt
    CONFIG -= embed_manifest_exe
    RC_FILE = resources.rc
    HEADERS += winnativeeventfilter.h
    SOURCES += winnativeeventfilter.cpp
    OTHER_FILES += manifest.xml
}
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
HEADERS += framelesshelper.h
SOURCES += framelesshelper.cpp main.cpp
