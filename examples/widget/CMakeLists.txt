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

set(DEMO_NAME FramelessHelperDemo-Widget)

if(FRAMELESSHELPER_ENABLE_UNIVERSAL_BUILD)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

if(FRAMELESSHELPER_EXAMPLES_STANDALONE)
    cmake_minimum_required(VERSION 3.20)
    project(${DEMO_NAME} VERSION 1.0)
    include(../../cmake/utils.cmake)
    setup_project(
        QT_PROJECT
        LANGUAGES CXX RC
        NO_WARNING
        ENABLE_LTO
    )
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
    find_package(FramelessHelper REQUIRED COMPONENTS Core Widgets)
endif()

add_executable(${DEMO_NAME})

target_sources(${DEMO_NAME} PRIVATE
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    widget.h
    widget.cpp
    main.cpp
)

if(WIN32)
    set(__rc_path "${PROJECT_BINARY_DIR}/${DEMO_NAME}.rc")
    if(NOT EXISTS "${__rc_path}")
        generate_win32_rc_file(
            PATH "${__rc_path}"
            VERSION "${PROJECT_VERSION}"
            COMPANY "wangwenx190"
            DESCRIPTION "FramelessHelper Demo Application: Widget"
            COPYRIGHT "MIT License"
            PRODUCT "FramelessHelper Demo"
            ICONS "../shared/example.ico"
        )
    endif()
    set(__manifest_path "${PROJECT_BINARY_DIR}/${DEMO_NAME}.manifest")
    if(NOT EXISTS "${__manifest_path}")
        generate_win32_manifest_file(
            PATH "${__manifest_path}"
            ID "org.wangwenx190.demo.Widget"
            VERSION "${PROJECT_VERSION}"
            DESCRIPTION "FramelessHelper Demo Application: Widget"
            VISTA_COMPAT
            WIN7_COMPAT
            WIN8_COMPAT
            WIN8_1_COMPAT
            WIN10_COMPAT
            WIN11_COMPAT
            XAML_ISLANDS_COMPAT
            UTF8_CODEPAGE
        )
    endif()
    target_sources(${DEMO_NAME} PRIVATE
        "${__rc_path}"
        "${__manifest_path}"
    )
endif()

target_link_libraries(${DEMO_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    FramelessHelper::Core
    FramelessHelper::Widgets
)

setup_gui_app(
    TARGETS ${DEMO_NAME}
    BUNDLE_ID "org.wangwenx190.demo.Widget"
    BUNDLE_VERSION "1.0.0"
    BUNDLE_VERSION_SHORT "1.0"
)

setup_target_rpaths(TARGETS ${DEMO_NAME})

setup_qt_stuff(TARGETS ${DEMO_NAME})

set(__extra_flags "")
if(NOT FRAMELESSHELPER_NO_PERMISSIVE_CHECKS)
    list(APPEND __extra_flags PERMISSIVE)
endif()
if(FRAMELESSHELPER_ENABLE_SPECTRE)
    list(APPEND __extra_flags SPECTRE)
endif()
if(FRAMELESSHELPER_ENABLE_EHCONTGUARD)
    list(APPEND __extra_flags EHCONTGUARD)
endif()
if(FRAMELESSHELPER_ENABLE_INTELCET)
    list(APPEND __extra_flags INTELCET)
endif()
if(FRAMELESSHELPER_ENABLE_CFGUARD)
    list(APPEND __extra_flags CFGUARD)
endif()
if(FRAMELESSHELPER_FORCE_LTO)
    list(APPEND __extra_flags FORCE_LTO)
endif()
setup_compile_params(TARGETS ${DEMO_NAME} ${__extra_flags})

if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    set(__extra_flags)
    if(FRAMELESSHELPER_NO_INSTALL)
        set(__extra_flags NO_INSTALL)
    endif()
    deploy_qt_runtime(TARGET ${DEMO_NAME} ${__extra_flags})
endif()

#dump_target_info(TARGETS ${DEMO_NAME})
