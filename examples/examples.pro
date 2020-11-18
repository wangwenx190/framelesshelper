TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += QWidget QWidget2 QMainWindow
qtHaveModule(quick): SUBDIRS += Quick
win32: qtHaveModule(widgets): SUBDIRS += Win32Demo
