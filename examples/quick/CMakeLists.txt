#[[
  MIT License

  Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick)

set(SOURCES
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    main.cpp
    quicksettings.h
    quicksettings.cpp
)

if(QT_VERSION VERSION_LESS "6.2")
    list(APPEND SOURCES resources.qrc)
endif()

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES
        ../shared/example.rc
        ../shared/example.manifest
    )
endif()

add_executable(Quick ${SOURCES})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2")
    set(QML_SOURCES
        "qml/Window.qml"
        "qml/ApplicationWindow.qml"
        "qml/HomePage.qml"
    )
    set_source_files_properties(${QML_SOURCES}
        PROPERTIES QT_DISCARD_FILE_CONTENTS TRUE
    )
    qt_add_qml_module(Quick
        URI Demo
        VERSION 1.0
        #IMPORT_PATH "${PROJECT_BINARY_DIR}/qml"
        OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/qml/Demo"
        RESOURCE_PREFIX "/"
        NO_RESOURCE_TARGET_PATH
        IMPORTS
            QtQuick/auto
        DEPENDENCIES
            org.wangwenx190.FramelessHelper/auto
        QML_FILES ${QML_SOURCES}
        #ENABLE_TYPE_COMPILER
    )
    qt_add_resources(Quick resources
        PREFIX
            "/"
        FILES
            "images/microsoft.svg"
    )
endif()

target_link_libraries(Quick PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    FramelessHelper::Quick
)

target_compile_definitions(Quick PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

include(../../cmake/utils.cmake)
setup_gui_app(
    TARGETS Quick
    BUNDLE_ID org.wangwenx190.Quick
    BUNDLE_VERSION 1.0.0
    BUNDLE_VERSION_SHORT 1.0
)
setup_compile_params(TARGETS Quick)
if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    set(__extra_flags)
    if(FRAMELESSHELPER_NO_INSTALL)
        set(__extra_flags NO_INSTALL)
    endif()
    deploy_qt_runtime(
        TARGET Quick
        QML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qml"
        ${__extra_flags}
    )
endif()
