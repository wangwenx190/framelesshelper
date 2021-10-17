cmake_minimum_required(VERSION 3.5)

project(FramelessHelper LANGUAGES CXX)

option(BUILD_EXAMPLES "Build examples." ON)
option(TEST_UNIX "Test UNIX version (from Win32)." OFF)

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick)

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()