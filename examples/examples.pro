TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets) {
    SUBDIRS += QWidget QMainWindow
    win32: SUBDIRS += Win32Demo
    versionAtLeast(QT_VERSION, 5.15.0): SUBDIRS += QWidget2
}
qtHaveModule(quick): SUBDIRS += Quick
