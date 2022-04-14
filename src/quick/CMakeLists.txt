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

set(PROJ_TARGET framelesshelperplugin)
set(PROJ_CLASS FramelessHelperPlugin)
set(PROJ_URI org.wangwenx190.FramelessHelper)
set(PROJ_VERSION 1.0)
set(PROJ_IMPORTS_DIR ${PROJECT_BINARY_DIR}/imports/org/wangwenx190/FramelessHelper)

set(INCLUDE_PREFIX ../../include/FramelessHelper/Quick)

find_package(Qt6 REQUIRED COMPONENTS QuickTemplates2)

qt_add_qml_module(${PROJ_TARGET}
    URI ${PROJ_URI}
    VERSION ${PROJ_VERSION}
    PLUGIN_TARGET ${PROJ_TARGET}
    CLASS_NAME ${PROJ_CLASS}
    OUTPUT_DIRECTORY ${PROJ_IMPORTS_DIR}
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        ${INCLUDE_PREFIX}/framelesshelperquick_global.h
        ${INCLUDE_PREFIX}/framelessquickutils.h
        ${INCLUDE_PREFIX}/framelesshelperimageprovider.h
        ${INCLUDE_PREFIX}/framelessquickwindow.h
        quickstandardminimizebutton_p.h
        quickstandardminimizebutton.cpp
        quickstandardmaximizebutton_p.h
        quickstandardmaximizebutton.cpp
        quickstandardclosebutton_p.h
        quickstandardclosebutton.cpp
        quickstandardtitlebar_p.h
        quickstandardtitlebar.cpp
        framelessquickwindow_p.h
        framelessquickutils.cpp
        framelesshelperimageprovider.cpp
        framelessquickwindow.cpp
        plugin.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    target_sources(${PROJ_TARGET} PRIVATE
        framelesshelperquick.rc
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${PROJ_TARGET} PUBLIC
        FRAMELESSHELPER_QUICK_STATIC
    )
endif()

target_compile_definitions(${PROJ_TARGET} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    #QT_NO_KEYWORDS
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    FRAMELESSHELPER_QUICK_LIBRARY
)

if(MSVC)
    set(_WIN32_WINNT_WIN10 0x0A00)
    set(NTDDI_WIN10_CO 0x0A00000B)
    target_compile_definitions(${PROJ_TARGET} PRIVATE
        _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
        _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
        WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
        _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    )
    target_compile_options(${PROJ_TARGET} PRIVATE
        /utf-8 /W4 /WX
    )
else()
    target_compile_options(${PROJ_TARGET} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${PROJ_TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::QuickTemplates2Private
)

target_link_libraries(${PROJ_TARGET} PUBLIC
    FramelessHelperCore
)

target_include_directories(${PROJ_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}"
)
