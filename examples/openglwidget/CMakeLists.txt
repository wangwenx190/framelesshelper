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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGL)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS OpenGLWidgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets)
endif()

set(SOURCES
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    images.qrc
    logo.h
    logo.cpp
    glwidget.h
    glwidget.cpp
    mainwindow.h
    mainwindow.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES
        ../shared/example.rc
        ../shared/example.manifest
    )
endif()

add_executable(OpenGLWidget ${SOURCES})

target_link_libraries(OpenGLWidget PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    FramelessHelper::Widgets
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(OpenGLWidget PRIVATE
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )
endif()

target_compile_definitions(OpenGLWidget PRIVATE
    QT_NO_KEYWORDS
)

include(../../src/core/cmakehelper.cmake)
setup_gui_app(OpenGLWidget)
setup_compile_params(OpenGLWidget)
if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    deploy_qt_runtime(OpenGLWidget)
endif()
