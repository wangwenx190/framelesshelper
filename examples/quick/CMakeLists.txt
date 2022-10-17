#[[
  MIT License

  Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Qml Quick QuickControls2)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Qml Quick QuickControls2)

set(SOURCES
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    main.cpp
    quicksettings.h
    quicksettings.cpp
)

if(${QT_VERSION} VERSION_LESS 6.2)
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

if(${QT_VERSION} VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(Quick
        URI Demo
        VERSION 1.0
        IMPORT_PATH "${PROJECT_BINARY_DIR}/imports"
        IMPORTS
            QtQml/auto
            QtQuick/auto
            QtQuick.Controls.Basic/auto
            org.wangwenx190.FramelessHelper/auto
        QML_FILES
            Window.qml
            ApplicationWindow.qml
            HomePage.qml
        #ENABLE_TYPE_COMPILER # We can't use it for now due to it still can't compile singletons.
                              # There's some hope to get it supported in Qt 6.5.
    )
    qt_add_resources(Quick resources
        PREFIX
            "/Demo"
        FILES
            "images/microsoft.svg"
    )
endif()

target_link_libraries(Quick PRIVATE
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::QuickControls2Private
    FramelessHelper::Quick
)

target_compile_definitions(Quick PRIVATE
    QT_NO_KEYWORDS
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

include(../../src/core/cmakehelper.cmake)
setup_gui_app(Quick)
setup_compile_params(Quick)
if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    deploy_qt_runtime(Quick
        QML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        #QML_IMPORT_DIR "${PROJECT_BINARY_DIR}/qml" # Some applications failed to start.
    )
endif()
