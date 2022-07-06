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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS QuickTemplates2 QuickControls2)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickTemplates2 QuickControls2)

set(SUB_MOD_NAME Quick)
set(SUB_PROJ_NAME ${PROJECT_NAME}${SUB_MOD_NAME})

set(INCLUDE_PREFIX ../../include/${PROJECT_NAME}/${SUB_MOD_NAME})

set(SOURCES
    ${INCLUDE_PREFIX}/framelesshelperquick_global.h
    ${INCLUDE_PREFIX}/framelessquickmodule.h
    ${INCLUDE_PREFIX}/framelessquickhelper.h
    ${INCLUDE_PREFIX}/framelessquickutils.h
    ${INCLUDE_PREFIX}/quickchromepalette.h
    ${INCLUDE_PREFIX}/private/quickstandardsystembutton_p.h
    ${INCLUDE_PREFIX}/private/quickstandardtitlebar_p.h
    ${INCLUDE_PREFIX}/private/framelessquickhelper_p.h
    ${INCLUDE_PREFIX}/private/framelessquickwindow_p.h
    ${INCLUDE_PREFIX}/private/framelessquickwindow_p_p.h
    quickstandardsystembutton.cpp
    quickstandardtitlebar.cpp
    framelessquickutils.cpp
    framelessquickmodule.cpp
    framelessquickwindow.cpp
    framelessquickhelper.cpp
    quickchromepalette.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/framelesshelper.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelperquick.rc @ONLY)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelperquick.rc
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${SOURCES})
endif()
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

set(_import_dir ${PROJECT_BINARY_DIR}/imports)
if(${FRAMELESSHELPER_IMPORT_DIR})
    set(_import_dir ${FRAMELESSHELPER_IMPORT_DIR})
endif()
string(APPEND _import_dir /org/wangwenx190/${PROJECT_NAME})

if(${QT_VERSION} VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(${SUB_PROJ_NAME}
        URI org.wangwenx190.${PROJECT_NAME}
        VERSION 1.0
        OUTPUT_DIRECTORY ${_import_dir}
        IMPORTS
            QtQml
            QtQuick
            QtQuick.Controls.Basic
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_QUICK_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    #QT_NO_KEYWORDS # Some private Qt Quick headers are not keyword-clean, so sad :(
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060500
    FRAMELESSHELPER_QUICK_LIBRARY
)

if(MSVC)
    set(_WIN32_WINNT_WIN10 0x0A00)
    set(NTDDI_WIN10_CO 0x0A00000B)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
        _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
        WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
        _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    )
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        /utf-8 /W3 /WX
    )
else()
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::QuickTemplates2Private
    Qt${QT_VERSION_MAJOR}::QuickControls2Private
)

target_link_libraries(${SUB_PROJ_NAME} PUBLIC
    ${PROJECT_NAME}::Core
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}/private"
)
