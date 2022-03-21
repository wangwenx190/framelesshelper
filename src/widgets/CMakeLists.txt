set(SUB_PROJ_NAME FramelessHelperWidgets)

set(SOURCES
    framelesshelperwidgets_global.h
    framelesswidgetshelper.h
    framelesswidgetshelper.cpp
    framelesswidget.h
    framelesswidget.cpp
    framelessmainwindow.h
    framelessmainwindow.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    list(APPEND SOURCES framelesshelperwidgets.rc)
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${SOURCES})
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_WIDGETS_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    FRAMELESSHELPER_WIDGETS_LIBRARY
)

if(MSVC)
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        /W4 /WX
    )
else()
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_link_libraries(${SUB_PROJ_NAME} PUBLIC
    FramelessHelperCore
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../include/FramelessHelper/Widgets"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../include"
)
