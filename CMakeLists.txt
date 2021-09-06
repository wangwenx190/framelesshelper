cmake_minimum_required(VERSION 3.5)

project(FramelessHelper LANGUAGES CXX)

option(BUILD_EXAMPLES "Build examples." ON)
option(TEST_UNIX "Test UNIX version (from Win32)." OFF)

set(BUILD_SHARED_LIBS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick)

set(SOURCES
    framelesshelper_global.h
    framelesshelper.h
    framelesshelper.cpp
    framelesswindowsmanager.h
    framelesswindowsmanager.cpp
    utilities.h
    utilities.cpp
)

if(TARGET Qt${QT_VERSION_MAJOR}::Quick)
    list(APPEND SOURCES
        framelessquickhelper.h
        framelessquickhelper.cpp
    )
endif()

if(WIN32)
    list(APPEND SOURCES
        framelesshelper_windows.h
        utilities_win32.cpp
        framelesshelper_win32.h
        framelesshelper_win32.cpp
    )
else()
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

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
