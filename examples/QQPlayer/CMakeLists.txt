cmake_minimum_required(VERSION 3.15)

project(QQPlayer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 COMPONENTS Quick REQUIRED)

set(source_files qml.qrc images.qrc main.cpp ../../framelessquickhelper.h ../../framelessquickhelper.cpp)

if(WIN32)
    enable_language(RC)
    list(APPEND source_files QQPlayer.exe.manifest QQPlayer.rc ../../winnativeeventfilter.h ../../winnativeeventfilter.cpp)
else()
    list(APPEND source_files ../../framelesshelper.h ../../framelesshelper.cpp)
endif()

add_executable(QQPlayer WIN32 ${source_files})

if(MSVC)
    # Silence a compiler warning caused by the Chinese comments in the source file.
    target_compile_options(QQPlayer PRIVATE -utf-8)
endif()
target_compile_definitions(QQPlayer PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(QQPlayer PRIVATE Qt::Quick Qt::GuiPrivate)
