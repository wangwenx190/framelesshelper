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

cmake_minimum_required(VERSION 3.20)

project(FramelessHelper
    VERSION 2.3.4.0
    DESCRIPTION "Cross-platform window customization framework for Qt Widgets and Qt Quick."
    HOMEPAGE_URL "https://github.com/wangwenx190/framelesshelper/"
    LANGUAGES CXX
)

option(FRAMELESSHELPER_BUILD_STATIC "Build FramelessHelper as a static library." OFF)
option(FRAMELESSHELPER_BUILD_WIDGETS "Build FramelessHelper's Widgets module." ON)
option(FRAMELESSHELPER_BUILD_QUICK "Build FramelessHelper's Quick module." ON)
option(FRAMELESSHELPER_BUILD_EXAMPLES "Build FramelessHelper demo applications." ON)
option(FRAMELESSHELPER_EXAMPLES_DEPLOYQT "Deploy the Qt framework after building the demo projects." ON)
option(FRAMELESSHELPER_NO_DEBUG_OUTPUT "Suppress the debug messages from FramelessHelper." OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE)
    # MinGW has many bugs when LTO is enabled, and they are all very
    # hard to workaround, so just don't enable LTO at all for MinGW.
    if(NOT MINGW)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    endif()
endif()

if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    if(WIN32)
        set(CMAKE_DEBUG_POSTFIX d)
    else()
        set(CMAKE_DEBUG_POSTFIX _debug)
    endif()
endif()

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_VERSION_COMMIT "UNKNOWN")
# Get a git hash value. We do not want to use git command here
# because we don't want to make git a build-time dependency.
if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
    file(READ "${CMAKE_SOURCE_DIR}/.git/HEAD" PROJECT_VERSION_COMMIT)
    string(STRIP "${PROJECT_VERSION_COMMIT}" PROJECT_VERSION_COMMIT)
    if(PROJECT_VERSION_COMMIT MATCHES "^ref: (.*)")
        set(HEAD "${CMAKE_MATCH_1}")
        if(EXISTS "${CMAKE_SOURCE_DIR}/.git/${HEAD}")
            file(READ "${CMAKE_SOURCE_DIR}/.git/${HEAD}" PROJECT_VERSION_COMMIT)
            string(STRIP "${PROJECT_VERSION_COMMIT}" PROJECT_VERSION_COMMIT)
        else()
            file(READ "${CMAKE_SOURCE_DIR}/.git/packed-refs" PACKED_REFS)
            string(REGEX REPLACE ".*\n([0-9a-f]+) ${HEAD}\n.*" "\\1" PROJECT_VERSION_COMMIT "\n${PACKED_REFS}")
        endif()
    endif()
endif()

set(PROJECT_COMPILE_DATETIME "UNKNOWN")
string(TIMESTAMP PROJECT_COMPILE_DATETIME UTC)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

find_package(QT NAMES Qt6 Qt5 QUIET COMPONENTS Widgets Quick)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Widgets Quick)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

if(TARGET Qt${QT_VERSION_MAJOR}::Core AND TARGET Qt${QT_VERSION_MAJOR}::Gui)
    add_subdirectory(src)
endif()

if(FRAMELESSHELPER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

message("#######################################")
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
message("#######################################")
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
# TODO: output shared/static Qt.
message("#######################################")
message("FramelessHelper version: ${PROJECT_VERSION}")
message("FramelessHelper commit hash: ${PROJECT_VERSION_COMMIT}")
message("FramelessHelper configure date and time: ${PROJECT_COMPILE_DATETIME} (UTC)")
message("Build the static version of FramelessHelper: ${FRAMELESSHELPER_BUILD_STATIC}")
message("Build the FramelessHelper::Widgets module: ${FRAMELESSHELPER_BUILD_WIDGETS}")
message("Build the FramelessHelper::Quick module: ${FRAMELESSHELPER_BUILD_QUICK}")
message("Build the FramelessHelper demo applications: ${FRAMELESSHELPER_BUILD_EXAMPLES}")
message("Deploy Qt libraries after compilation: ${FRAMELESSHELPER_EXAMPLES_DEPLOYQT}")
message("Suppress debug messages from FramelessHelper: ${FRAMELESSHELPER_NO_DEBUG_OUTPUT}")
message("#######################################")
