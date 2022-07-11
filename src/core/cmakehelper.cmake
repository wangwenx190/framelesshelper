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

function(setup_compile_params arg_target)
    target_compile_definitions(${arg_target} PRIVATE
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
        #QT_NO_KEYWORDS # QtQuick headers still use traditional Qt keywords.
        QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
        QT_NO_FOREACH
        QT_USE_QSTRINGBUILDER
        QT_DEPRECATED_WARNINGS
        QT_DISABLE_DEPRECATED_BEFORE=0x060500
    )
    if(MSVC)
        set(_WIN32_WINNT_WIN10 0x0A00)
        set(NTDDI_WIN10_CO 0x0A00000B)
        target_compile_definitions(${arg_target} PRIVATE
            _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
            _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
            _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
            WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
            _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
        )
        target_compile_options(${arg_target} PRIVATE
            /utf-8 /W3 /WX # Cannot use /W4 here, Qt's own headers are not warning-clean.
        )
        target_link_options(${arg_target} PRIVATE /WX)
    else()
        target_compile_options(${arg_target} PRIVATE
            -Wall -Wextra -Werror
        )
    endif()
endfunction()

function(setup_gui_app arg_target)
    set_target_properties(${arg_target} PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER org.wangwenx190.${PROJECT_NAME}.app
        MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0.0
        MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    )
endfunction()

function(setup_package_export arg_target arg_path arg_public arg_alias arg_private)
    include(GNUInstallDirs)
    install(TARGETS ${arg_target}
        EXPORT ${arg_target}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${arg_path}
    )
    export(EXPORT ${arg_target}Targets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${arg_target}Targets.cmake"
        NAMESPACE ${PROJECT_NAME}::
    )
    install(FILES ${arg_public} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${arg_path})
    install(FILES ${arg_alias} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${arg_path})
    install(FILES ${arg_private} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${arg_path}/private)
    install(EXPORT ${arg_target}Targets
        FILE ${arg_target}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endfunction()

function(deploy_qt_runtime arg_target)
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
        set(__old_deploy_params)
        if(${QT_VERSION_MAJOR} LESS 6)
            set(__old_deploy_params --no-webkit2 --no-angle)
        endif()
        add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
            "${QT_DEPLOY_EXECUTABLE}"
            --dir "$<TARGET_FILE_DIR:${arg_target}>/qml"
            --libdir "$<TARGET_FILE_DIR:${arg_target}>"
            --plugindir "$<TARGET_FILE_DIR:${arg_target}>/plugins"
            --qmldir "$<TARGET_PROPERTY:${arg_target},SOURCE_DIR>"
            --qmlimport "${PROJECT_BINARY_DIR}/qml"
            --no-translations
            --no-system-d3d-compiler
            --no-virtualkeyboard
            --no-compiler-runtime
            --no-opengl-sw
            --verbose 0
            ${__old_deploy_params}
            "$<TARGET_FILE:${arg_target}>"
        )
    elseif(APPLE)
        add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
            "${QT_DEPLOY_EXECUTABLE}"
            "$<TARGET_BUNDLE_DIR:${arg_target}>"
            -qmldir="$<TARGET_PROPERTY:${arg_target},SOURCE_DIR>"
            -qmlimport="${PROJECT_BINARY_DIR}/qml"
            -verbose=0
        )
    elseif(UNIX)
        # TODO
    endif()
endfunction()
