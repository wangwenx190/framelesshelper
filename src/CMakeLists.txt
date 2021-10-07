set(SOURCES
    framelesshelper_global.h
    core/framelesshelper.h
    core/framelesshelper.cpp
    core/utilities.h
    core/utilities.cpp
    core/framelesswindowsmanager.h
    core/framelesswindowsmanager.cpp
)

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    list(APPEND SOURCES
        quick/framelessquickhelper.h
        quick/framelessquickhelper.cpp
    )
endif()

if(WIN32)
    list(APPEND SOURCES
        core/framelesshelper_windows.h
        core/utilities_win32.cpp
        core/framelesshelper_win32.h
        core/framelesshelper_win32.cpp
    )
else()
    if(APPLE)
        list(APPEND SOURCES core/utilities_macos.mm)
    else()
        find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras REQUIRED)
        list(APPEND SOURCES core/utilities_linux.cpp)
    endif()
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
    enable_language(RC)
    list(APPEND SOURCES framelesshelper.rc)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
add_library(wangwenx190::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        FRAMELESSHELPER_STATIC
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060100
    FRAMELESSHELPER_BUILD_LIBRARY
)

if(TEST_UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        FRAMELESSHELPER_TEST_UNIX
    )
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dwmapi
    )
else()
    if(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            "-framework Cocoa -framework Carbon"
        )
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt${QT_VERSION_MAJOR}::X11Extras
            X11
        )
    endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::GuiPrivate
)

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Quick
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
