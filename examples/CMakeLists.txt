find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)

if(TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    add_subdirectory(widget)
    add_subdirectory(mainwindow)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    add_subdirectory(quick)
endif()
