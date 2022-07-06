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

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    find_package(X11 REQUIRED)
    pkg_search_module(GTK3 REQUIRED gtk+-3.0)
endif()

set(SUB_MOD_NAME Core)
set(SUB_PROJ_NAME ${PROJECT_NAME}${SUB_MOD_NAME})

set(INCLUDE_PREFIX ../../include/${PROJECT_NAME}/${SUB_MOD_NAME})

configure_file(framelesshelper_version.inc.in
    ${PROJECT_BINARY_DIR}/framelesshelper_version.inc @ONLY)

set(SOURCES
    ${PROJECT_BINARY_DIR}/framelesshelper_version.inc
    ${INCLUDE_PREFIX}/framelesshelpercore_global.h
    ${INCLUDE_PREFIX}/framelesshelper_qt.h
    ${INCLUDE_PREFIX}/framelessmanager.h
    ${INCLUDE_PREFIX}/utils.h
    ${INCLUDE_PREFIX}/chromepalette.h
    ${INCLUDE_PREFIX}/private/framelessmanager_p.h
    ${INCLUDE_PREFIX}/private/framelessconfig_p.h
    ${INCLUDE_PREFIX}/private/sysapiloader_p.h
    ${INCLUDE_PREFIX}/private/chromepalette_p.h
    framelesshelpercore.qrc
    utils.cpp
    framelesshelper_qt.cpp
    framelessmanager.cpp
    framelessconfig.cpp
    sysapiloader.cpp
    chromepalette.cpp
)

if(WIN32)
    list(APPEND SOURCES
        ${INCLUDE_PREFIX}/framelesshelper_windows.h
        ${INCLUDE_PREFIX}/framelesshelper_win.h
        utils_win.cpp
        framelesshelper_win.cpp
    )
elseif(APPLE)
    list(APPEND SOURCES utils_mac.mm)
elseif(UNIX)
    list(APPEND SOURCES utils_linux.cpp)
endif()

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/framelesshelper.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelpercore.rc @ONLY)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelpercore.rc
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${SOURCES})
endif()
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_CORE_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_NO_KEYWORDS
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_FOREACH
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060500
    FRAMELESSHELPER_CORE_LIBRARY
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

if (APPLE)
    target_link_libraries(${SUB_PROJ_NAME} PRIVATE
        "-framework Foundation"
        "-framework Cocoa"
    )
elseif (UNIX)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        GDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_6
    )
    target_link_libraries(${SUB_PROJ_NAME} PRIVATE
        ${GTK3_LIBRARIES}
        X11::xcb
    )
    target_include_directories(${SUB_PROJ_NAME} PRIVATE
        ${GTK3_INCLUDE_DIRS}
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::GuiPrivate
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}/private"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)
