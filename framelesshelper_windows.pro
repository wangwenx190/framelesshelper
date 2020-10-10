TARGET = framelessapplication
CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,d)
TEMPLATE = app
QT += gui-private
qtHaveModule(widgets): QT += widgets
qtHaveModule(quick) {
    QT += quick
    HEADERS += framelessquickhelper.h
    SOURCES += framelessquickhelper.cpp
}
CONFIG += c++17 strict_c++ utf8_source warn_on windeployqt
DEFINES += \
    WIN32_LEAN_AND_MEAN \
    _CRT_SECURE_NO_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    FRAMELESSHELPER_STATIC
LINK_TO_SYSTEM_DLL {
    DEFINES += WNEF_LINK_SYSLIB
    LIBS += -luser32 -lgdi32 -ldwmapi -lshcore
}
CONFIG -= embed_manifest_exe
RC_FILE = framelesshelper_windows.rc
HEADERS += framelesshelper_global.h winnativeeventfilter.h
SOURCES += winnativeeventfilter.cpp main_windows.cpp
RESOURCES += framelesshelper_windows.qrc
OTHER_FILES += framelesshelper_windows.manifest
