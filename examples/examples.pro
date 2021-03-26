TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += widget mainwindow
qtHaveModule(quick): SUBDIRS += quick
