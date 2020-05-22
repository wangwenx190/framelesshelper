cmake_minimum_required(VERSION 3.15)

project(QQPlayer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
else()
    set(CMAKE_DEBUG_POSTFIX "_debug")
endif()

find_package(Qt5 COMPONENTS Core Quick QuickCompiler REQUIRED)

set(source_files images.qrc main.cpp ../../framelessquickhelper.h ../../framelessquickhelper.cpp)

if(WIN32)
    enable_language(RC)
    list(APPEND source_files QQPlayer.exe.manifest QQPlayer.rc ../../winnativeeventfilter.h ../../winnativeeventfilter.cpp)
else()
    list(APPEND source_files ../../framelesshelper.h ../../framelesshelper.cpp)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND source_files qml.qrc)
else()
    set(qml_files "")
    qtquick_compiler_add_resources(qml_files qml.qrc)
    list(APPEND source_files ${qml_files})
endif()

if(WIN32)
    # Without WIN32, a console window will show.
    add_executable(QQPlayer WIN32 ${source_files})
else()
    add_executable(QQPlayer ${source_files})
endif()

if(MSVC)
    # Silence a compiler warning caused by the Chinese comments in the source file.
    target_compile_options(QQPlayer PRIVATE -utf-8)
endif()
target_compile_definitions(QQPlayer PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(QQPlayer PRIVATE Qt::Core Qt::Quick Qt::GuiPrivate)
