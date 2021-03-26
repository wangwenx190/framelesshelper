cmake_minimum_required(VERSION 3.18)

project(FramelessHelper LANGUAGES CXX)

option(BUILD_EXAMPLES "Build examples." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_DEBUG_POSTFIX)
    if(WIN32)
        set(CMAKE_DEBUG_POSTFIX d)
    else()
        set(CMAKE_DEBUG_POSTFIX _debug)
    endif()
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick)

set(SOURCES
    qtacrylichelper.qrc
    framelesshelper_global.h
    framelesswindowsmanager.h
    framelesswindowsmanager.cpp
    utilities.h
    utilities.cpp
    qtacryliceffecthelper.h
    qtacryliceffecthelper.cpp
)

if(TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    list(APPEND SOURCES
        qtacrylicwidget.h
        qtacrylicwidget.cpp
        qtacrylicmainwindow.h
        qtacrylicmainwindow.cpp
    )
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    list(APPEND SOURCES
        framelessquickhelper.h
        framelessquickhelper.cpp
        qtacrylicitem.h
        qtacrylicitem.cpp
    )
endif()

if(WIN32)
    list(APPEND SOURCES
        utilities_win32.cpp
        framelesshelper_win32.h
        framelesshelper_win32.cpp
        qtacryliceffecthelper_win32.h
        qtacryliceffecthelper_win32.cpp
    )
else()
    list(APPEND SOURCES
        framelesshelper.h
        framelesshelper.cpp
    )
    if(MACOS)
        list(APPEND SOURCES utilities_macos.mm)
    else()
        list(APPEND SOURCES utilities_linux.cpp)
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
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    FRAMELESSHELPER_BUILD_LIBRARY
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32 shell32 gdi32 dwmapi
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

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
