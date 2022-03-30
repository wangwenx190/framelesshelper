add_subdirectory(core)

if(FRAMELESSHELPER_BUILD_WIDGETS AND TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    add_subdirectory(widgets)
endif()

if(FRAMELESSHELPER_BUILD_QUICK AND TARGET Qt${QT_VERSION_MAJOR}::Quick)
    add_subdirectory(quick)
endif()
