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

cmake_minimum_required(VERSION 3.20)

project(FramelessHelper
    VERSION 2.3.5.0
    DESCRIPTION "Cross-platform window customization framework for Qt Widgets and Qt Quick."
    HOMEPAGE_URL "https://github.com/wangwenx190/framelesshelper/"
)

option(FRAMELESSHELPER_BUILD_STATIC "Build FramelessHelper as a static library." OFF)
option(FRAMELESSHELPER_BUILD_WIDGETS "Build FramelessHelper's Widgets module." ON)
option(FRAMELESSHELPER_BUILD_QUICK "Build FramelessHelper's Quick module." ON)
option(FRAMELESSHELPER_BUILD_EXAMPLES "Build FramelessHelper demo applications." OFF)
option(FRAMELESSHELPER_EXAMPLES_DEPLOYQT "Deploy the Qt framework after building the demo projects." OFF)
option(FRAMELESSHELPER_NO_DEBUG_OUTPUT "Suppress the debug messages from FramelessHelper." OFF)
option(FRAMELESSHELPER_NO_BUNDLE_RESOURCE "Do not bundle any resources within FramelessHelper." OFF)
option(FRAMELESSHELPER_NO_PRIVATE "Do not use any private functionalities from Qt." OFF)
option(FRAMELESSHELPER_ENABLE_VCLTL "MSVC only: link to the system MSVCRT/UCRT and get rid of API sets." OFF)
option(FRAMELESSHELPER_ENABLE_YYTHUNKS "MSVC only: dynamic load most Win32 APIs to give better compatibility for old Windows versions." OFF)
option(FRAMELESSHELPER_NO_PERMISSIVE_CHECKS "MSVC only: disable the additional permissive checks." OFF)
option(FRAMELESSHELPER_NO_INSTALL "Don't install any files." OFF)
option(FRAMELESSHELPER_NO_SUMMARY "Don't show CMake configure summary." OFF)
option(FRAMELESSHELPER_ENABLE_SPECTRE "Mitigate Spectre security vulnerabilities." OFF)
option(FRAMELESSHELPER_ENABLE_EHCONTGUARD "Enable EH Continuation (EHCONT) Metadata." OFF)
option(FRAMELESSHELPER_ENABLE_INTELCET "Enable Intel CET." ON)
option(FRAMELESSHELPER_ENABLE_INTELJCC "Enable Intel JCC." ON)
option(FRAMELESSHELPER_ENABLE_CFGUARD "Enable Control Flow Guard (CFG)." ON)

if(FRAMELESSHELPER_NO_BUNDLE_RESOURCE)
    message(WARNING "Nothing will be embeded into FramelessHelper, the chrome buttons will have no icon.")
endif()

if(FRAMELESSHELPER_ENABLE_VCLTL AND NOT MSVC)
    message(WARNING "VC-LTL is only available for the MSVC toolchain.")
endif()

include(cmake/utils.cmake)

set(__extra_flags)
if(NOT FRAMELESSHELPER_BUILD_STATIC)
    list(APPEND __extra_flags ENABLE_LTO)
endif()
setup_project(
    QT_PROJECT
    QML_IMPORT_DIR "${PROJECT_BINARY_DIR}/imports"
    LANGUAGES CXX RC
    MAX_WARNING
    RTTI
    EXCEPTIONS
    ${__extra_flags}
)
unset(__extra_flags)

set(PROJECT_VERSION_COMMIT "UNKNOWN")
get_commit_hash(RESULT PROJECT_VERSION_COMMIT)

set(PROJECT_COMPILE_DATETIME "UNKNOWN")
string(TIMESTAMP PROJECT_COMPILE_DATETIME UTC)

if(MSVC)
    if(FRAMELESSHELPER_ENABLE_VCLTL)
        include(cmake/VC-LTL.cmake)
        if("x${SupportLTL}" STREQUAL "xtrue")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
        endif()
    endif()
    if(FRAMELESSHELPER_ENABLE_YYTHUNKS)
        set(YYTHUNKS_TARGET_OS "WinXP" CACHE STRING "" FORCE)
        include(cmake/YY-Thunks.cmake)
    endif()
endif()

set(__extra_flags)
if(FRAMELESSHELPER_NO_INSTALL)
    list(APPEND __extra_flags NO_INSTALL)
endif()
prepare_package_export(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    ${__extra_flags}
)
unset(__extra_flags)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

find_package(QT NAMES Qt6 Qt5 QUIET COMPONENTS Widgets Quick)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Widgets Quick)

if(FRAMELESSHELPER_BUILD_QUICK AND NOT TARGET Qt${QT_VERSION_MAJOR}::Quick)
    message(WARNING "Can't find the QtQuick module. FramelessHelper's QtQuick implementation and the QtQuick demo won't be built.")
    set(FRAMELESSHELPER_BUILD_QUICK OFF)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Core AND TARGET Qt${QT_VERSION_MAJOR}::Gui)
    add_subdirectory(src)
else()
    message(WARNING "Can't find the QtCore and QtGui module. Nothing will be built.")
    set(FRAMELESSHELPER_BUILD_WIDGETS OFF)
    set(FRAMELESSHELPER_BUILD_EXAMPLES OFF)
endif()

if(FRAMELESSHELPER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(NOT FRAMELESSHELPER_NO_SUMMARY)
    message("--------------------------- Compiler ----------------------------")
    message("CMake version: ${CMAKE_VERSION}")
    message("Host system: ${CMAKE_HOST_SYSTEM}")
    message("Host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    #message("C compiler: ${CMAKE_C_COMPILER_ID} (${CMAKE_C_COMPILER})") # Currently we are not using any C compilers.
    #message("C compiler version: ${CMAKE_C_COMPILER_VERSION}")
    message("C++ compiler: ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER})")
    message("C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
    message("Linker: ${CMAKE_LINKER}")
    message("Make program: ${CMAKE_MAKE_PROGRAM}")
    message("Build type: ${CMAKE_BUILD_TYPE}")
    message("Install prefix: ${CMAKE_INSTALL_PREFIX}")
    message("------------------------------ Qt -------------------------------")
    set(__qt_inst_dir)
    if(DEFINED Qt6_DIR)
        set(__qt_inst_dir "${Qt6_DIR}")
    else()
        set(__qt_inst_dir "${Qt5_DIR}")
    endif()
    # /whatever/Qt/6.4.0/gcc_64/lib/cmake/Qt6
    set(__qt_inst_dir ${__qt_inst_dir}/../../..)
    cmake_path(NORMAL_PATH __qt_inst_dir)
    message("Qt install dir: ${__qt_inst_dir}")
    message("Qt version: ${QT_VERSION}")
    get_target_property(__qt_type Qt${QT_VERSION_MAJOR}::Core TYPE)
    if(__qt_type STREQUAL "STATIC_LIBRARY")
        set(__qt_type static)
    else()
        set(__qt_type shared)
    endif()
    message("Qt library type: ${__qt_type}")
    message("------------------------ FramelessHelper ------------------------")
    message("FramelessHelper version: ${PROJECT_VERSION}")
    message("FramelessHelper commit hash: ${PROJECT_VERSION_COMMIT}")
    message("FramelessHelper configure date and time: ${PROJECT_COMPILE_DATETIME} (UTC)")
    message("Build the static version of FramelessHelper: ${FRAMELESSHELPER_BUILD_STATIC}")
    message("Build the FramelessHelper::Widgets module: ${FRAMELESSHELPER_BUILD_WIDGETS}")
    message("Build the FramelessHelper::Quick module: ${FRAMELESSHELPER_BUILD_QUICK}")
    message("Build the FramelessHelper demo applications: ${FRAMELESSHELPER_BUILD_EXAMPLES}")
    message("Deploy Qt libraries after compilation: ${FRAMELESSHELPER_EXAMPLES_DEPLOYQT}")
    message("Suppress debug messages from FramelessHelper: ${FRAMELESSHELPER_NO_DEBUG_OUTPUT}")
    message("Do not bundle any resources within FramelessHelper: ${FRAMELESSHELPER_NO_BUNDLE_RESOURCE}")
    message("Do not use any private functionalities from Qt: ${FRAMELESSHELPER_NO_PRIVATE}")
    message("[MSVC] Link to system C runtime library: ${FRAMELESSHELPER_ENABLE_VCLTL}")
    message("[MSVC] Thunk system APIs for old system: ${FRAMELESSHELPER_ENABLE_YYTHUNKS}")
    message("[MSVC] Disable permissive checks: ${FRAMELESSHELPER_NO_PERMISSIVE_CHECKS}")
    message("Do not install anything for CMake install: ${FRAMELESSHELPER_NO_INSTALL}")
    message("Mitigate Spectre security vulnerabilities: ${FRAMELESSHELPER_ENABLE_SPECTRE}")
    message("Enable EH Continuation (EHCONT) Metadata: ${FRAMELESSHELPER_ENABLE_EHCONTGUARD}")
    message("Enable Intel CET: ${FRAMELESSHELPER_ENABLE_INTELCET}")
    message("Enable Intel JCC: ${FRAMELESSHELPER_ENABLE_INTELJCC}")
    message("Enable Control Flow Guard (CFG): ${FRAMELESSHELPER_ENABLE_CFGUARD}")
    message("-----------------------------------------------------------------")
endif()
