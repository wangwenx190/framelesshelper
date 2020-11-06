cmake_minimum_required(VERSION 3.18)

project(FramelessHelper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
else()
    set(CMAKE_DEBUG_POSTFIX _debug)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Quick)

set(SOURCES
    framelesshelper_global.h
    framelesswindowsmanager.h
    framelesswindowsmanager.cpp
)

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    list(APPEND SOURCES
        framelessquickhelper.h
        framelessquickhelper.cpp
    )
endif()

if(WIN32)
    list(APPEND SOURCES
        winnativeeventfilter.h
        winnativeeventfilter.cpp
    )
else()
    list(APPEND SOURCES
        framelesshelper.h
        framelesshelper.cpp
    )
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
    enable_language(RC)
    list(APPEND SOURCES framelesshelper.rc)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        FRAMELESSHELPER_STATIC
    )
endif()
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
        target_link_options(${PROJECT_NAME} PRIVATE /GUARD:CF)
    endif()
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    FRAMELESSHELPER_BUILD_LIBRARY
)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
        WNEF_LINK_SYSLIB
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32 shell32 gdi32 dwmapi shcore uxtheme d2d1
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::GuiPrivate
)
if(TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
    )
endif()
if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Quick
    )
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
