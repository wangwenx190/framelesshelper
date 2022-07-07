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

function(deploy_qt_libraries arg_target)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
    if(NOT DEFINED QT_QMAKE_EXECUTABLE)
        get_target_property(QT_QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
    endif()
    if(NOT EXISTS "${QT_QMAKE_EXECUTABLE}")
        message("Cannot find the QMake executable.")
        return()
    endif()
    get_filename_component(QT_BIN_DIRECTORY "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(QT_DEPLOY_EXECUTABLE NAMES windeployqt macdeployqt HINTS "${QT_BIN_DIRECTORY}")
    if(NOT EXISTS "${QT_DEPLOY_EXECUTABLE}")
        message("Cannot find the deployqt tool.")
        return()
    endif()
    if(WIN32)
        add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
            "${QT_DEPLOY_EXECUTABLE}"
            --dir "$<TARGET_FILE_DIR:${arg_target}>/qml"
            --libdir "$<TARGET_FILE_DIR:${arg_target}>"
            --plugindir "$<TARGET_FILE_DIR:${arg_target}>/plugins"
            --qmldir "$<TARGET_PROPERTY:${arg_target},SOURCE_DIR>"
            --qmlimport "${PROJECT_BINARY_DIR}/imports"
            --no-translations
            --no-system-d3d-compiler
            --no-virtualkeyboard
            --no-compiler-runtime
            --no-opengl-sw
            "$<TARGET_FILE:${arg_target}>"
        )
    elseif(APPLE)
        add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
            "${QT_DEPLOY_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:${arg_target}>"
        )
    endif()
endfunction()
