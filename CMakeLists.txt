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

find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets Quick)

set(SOURCES
    framelesshelper_global.h
    framelesswindowsmanager.h
    framelesswindowsmanager.cpp
)

if(Qt5Quick_FOUND)
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
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::GuiPrivate)
if(Qt5Widgets_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Widgets)
endif()
if(Qt5Quick_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Quick)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
