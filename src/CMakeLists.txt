add_subdirectory(core)

if(TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    add_subdirectory(widgets)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    add_subdirectory(quick)
endif()
