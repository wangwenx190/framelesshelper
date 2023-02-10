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

set(SUB_MOD_NAME Widgets)
set(SUB_PROJ_NAME ${PROJECT_NAME}${SUB_MOD_NAME})
set(SUB_PROJ_PATH ${PROJECT_NAME}/${SUB_MOD_NAME})

set(INCLUDE_PREFIX ../../include/${SUB_PROJ_PATH})

set(PUBLIC_HEADERS
    ${INCLUDE_PREFIX}/framelesshelperwidgets_global.h
    ${INCLUDE_PREFIX}/framelesswidget.h
    ${INCLUDE_PREFIX}/framelessmainwindow.h
    ${INCLUDE_PREFIX}/standardsystembutton.h
    ${INCLUDE_PREFIX}/framelesswidgetshelper.h
    ${INCLUDE_PREFIX}/standardtitlebar.h
    ${INCLUDE_PREFIX}/framelessdialog.h
)

set(PUBLIC_HEADERS_ALIAS
    ${INCLUDE_PREFIX}/Global
    ${INCLUDE_PREFIX}/FramelessWidget
    ${INCLUDE_PREFIX}/FramelessMainWindow
    ${INCLUDE_PREFIX}/StandardSystemButton
    ${INCLUDE_PREFIX}/FramelessWidgetsHelper
    ${INCLUDE_PREFIX}/StandardTitleBar
    ${INCLUDE_PREFIX}/FramelessDialog
)

set(PRIVATE_HEADERS
    ${INCLUDE_PREFIX}/private/framelesswidgetshelper_p.h
    ${INCLUDE_PREFIX}/private/standardsystembutton_p.h
    ${INCLUDE_PREFIX}/private/standardtitlebar_p.h
    ${INCLUDE_PREFIX}/private/framelesswidget_p.h
    ${INCLUDE_PREFIX}/private/framelessmainwindow_p.h
    ${INCLUDE_PREFIX}/private/widgetssharedhelper_p.h
    ${INCLUDE_PREFIX}/private/framelessdialog_p.h
)

set(SOURCES
    framelessmainwindow.cpp
    framelesswidgetshelper.cpp
    framelesswidget.cpp
    standardsystembutton.cpp
    standardtitlebar.cpp
    widgetssharedhelper.cpp
    framelesshelperwidgets_global.cpp
    framelessdialog.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    set(__rc_path "${CMAKE_CURRENT_BINARY_DIR}/framelesshelperwidgets.rc")
    generate_win32_rc_file(
        PATH "${__rc_path}"
        VERSION "${PROJECT_VERSION}"
        COMPANY "wangwenx190"
        DESCRIPTION "${PROJECT_NAME} ${SUB_MOD_NAME} Module"
        COPYRIGHT "MIT License"
        ORIGINAL_FILENAME "${PROJECT_NAME}${SUB_MOD_NAME}.dll"
        PRODUCT "${PROJECT_NAME}"
        COMMENTS "Built from commit ${PROJECT_VERSION_COMMIT} on ${PROJECT_COMPILE_DATETIME} (UTC)."
        LIBRARY
    )
    list(APPEND SOURCES "${__rc_path}")
endif()

set(ALL_SOURCES ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${ALL_SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${ALL_SOURCES})
endif()
add_library(${PROJECT_NAME}::${SUB_PROJ_NAME} ALIAS ${SUB_PROJ_NAME})
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_WIDGETS_STATIC
    )
endif()

if(FRAMELESSHELPER_NO_DEBUG_OUTPUT)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
    )
endif()

if(FRAMELESSHELPER_NO_BUNDLE_RESOURCE)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        FRAMELESSHELPER_WIDGETS_NO_BUNDLE_RESOURCE
    )
endif()

if(FRAMELESSHELPER_NO_PRIVATE)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        FRAMELESSHELPER_WIDGETS_NO_PRIVATE
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    FRAMELESSHELPER_WIDGETS_LIBRARY
)

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_link_libraries(${SUB_PROJ_NAME} PUBLIC
    ${PROJECT_NAME}::Core
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PREFIX}/../..>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PREFIX}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PREFIX}/private>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}/private>"
)

setup_qt_stuff(TARGETS ${SUB_PROJ_NAME})
set(__extra_flags)
if(NOT FRAMELESSHELPER_NO_PERMISSIVE_CHECKS)
    set(__extra_flags PERMISSIVE)
endif()
setup_compile_params(TARGETS ${SUB_PROJ_NAME} ${__extra_flags})
if(NOT FRAMELESSHELPER_NO_INSTALL)
    setup_package_export(
        TARGET ${SUB_PROJ_NAME}
        PACKAGE_NAME ${PROJECT_NAME}
        NAMESPACE ${PROJECT_NAME}
        INCLUDE_PATH ${SUB_PROJ_PATH}
        PUBLIC_HEADERS ${PUBLIC_HEADERS}
        ALIAS_HEADERS ${PUBLIC_HEADERS_ALIAS}
        PRIVATE_HEADERS ${PRIVATE_HEADERS}
    )
endif()
