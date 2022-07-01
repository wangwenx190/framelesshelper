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
    main.cpp
    settings.h
    settings.cpp
)

if(${QT_VERSION} VERSION_LESS 6.2)
    list(APPEND SOURCES qml.qrc)
endif()

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES ../example.rc ../example.manifest)
endif()

add_executable(Quick ${SOURCES})

if(${QT_VERSION} VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(Quick
        URI Demo
        VERSION 1.0
        IMPORT_PATH ${PROJECT_BINARY_DIR}/imports
        IMPORTS
            QtQml
            QtQuick
            QtQuick.Controls.Basic
            org.wangwenx190.FramelessHelper
        QML_FILES MainWindow.qml
        #ENABLE_TYPE_COMPILER # We can't use it for now due to it still can't compile singletons.
                              # There's some hope to get it supported in Qt 6.5.
    )
endif()

set_target_properties(Quick PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER org.wangwenx190.framelesshelper.demo
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0.0
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
)

target_link_libraries(Quick PRIVATE
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::QuickControls2Private
    FramelessHelper::Quick
)

target_compile_definitions(Quick PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060500
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
