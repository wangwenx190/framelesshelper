TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += widget qmainwindow
qtHaveModule(quick): SUBDIRS += quick
