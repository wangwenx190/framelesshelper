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
set(SUB_PROJ_PATH ${PROJECT_NAME}/${SUB_MOD_NAME})

set(INCLUDE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../../include/${SUB_PROJ_PATH})
message(${INCLUDE_PREFIX})

configure_file(framelesshelper.version.in
    ${CMAKE_CURRENT_BINARY_DIR}/framelesshelper.version @ONLY)

set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/framelesshelper.version
    ${INCLUDE_PREFIX}/framelesshelpercore_global.h
    ${INCLUDE_PREFIX}/framelesshelper_qt.h
    ${INCLUDE_PREFIX}/framelessmanager.h
    ${INCLUDE_PREFIX}/utils.h
    ${INCLUDE_PREFIX}/chromepalette.h
    ${INCLUDE_PREFIX}/micamaterial.h
    ${INCLUDE_PREFIX}/windowborderpainter.h
)

set(PUBLIC_HEADERS_ALIAS
    ${INCLUDE_PREFIX}/Global
    ${INCLUDE_PREFIX}/FramelessHelper_Qt
    ${INCLUDE_PREFIX}/FramelessManager
    ${INCLUDE_PREFIX}/Utils
    ${INCLUDE_PREFIX}/ChromePalette
    ${INCLUDE_PREFIX}/MicaMaterial
    ${INCLUDE_PREFIX}/WindowBorderPainter
)

set(PRIVATE_HEADERS
    ${INCLUDE_PREFIX}/private/framelessmanager_p.h
    ${INCLUDE_PREFIX}/private/framelessconfig_p.h
    ${INCLUDE_PREFIX}/private/sysapiloader_p.h
    ${INCLUDE_PREFIX}/private/chromepalette_p.h
    ${INCLUDE_PREFIX}/private/micamaterial_p.h
    ${INCLUDE_PREFIX}/private/windowborderpainter_p.h
)

set(SOURCES
    utils.cpp
    framelesshelper_qt.cpp
    framelessmanager.cpp
    framelessconfig.cpp
    sysapiloader.cpp
    chromepalette.cpp
    framelesshelpercore_global.cpp
    micamaterial.cpp
    windowborderpainter.cpp
)

if(WIN32)
    list(APPEND PUBLIC_HEADERS
        ${INCLUDE_PREFIX}/framelesshelper_windows.h
        ${INCLUDE_PREFIX}/framelesshelper_win.h
    )
    list(APPEND PUBLIC_HEADERS_ALIAS
        ${INCLUDE_PREFIX}/FramelessHelper_Windows
        ${INCLUDE_PREFIX}/FramelessHelper_Win
    )
    list(APPEND PRIVATE_HEADERS
        ${INCLUDE_PREFIX}/private/registrykey_p.h
        ${INCLUDE_PREFIX}/private/winverhelper_p.h
    )
    list(APPEND SOURCES
        registrykey.cpp
        utils_win.cpp
        framelesshelper_win.cpp
        winverhelper.cpp
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

set(ALL_SOURCES ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${ALL_SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${ALL_SOURCES})
endif()
add_library(${PROJECT_NAME}::${SUB_PROJ_NAME} ALIAS ${SUB_PROJ_NAME})
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

if(${QT_VERSION} VERSION_GREATER_EQUAL 6.2)
    qt_add_resources(${SUB_PROJ_NAME} framelesshelpercore
        PREFIX
            "/org.wangwenx190.${PROJECT_NAME}"
        FILES
            "resources/fonts/Micon.ttf"
            "resources/images/noise.png"
    )
else()
    target_sources(${SUB_PROJ_NAME} PRIVATE
        framelesshelpercore.qrc
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_CORE_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_KEYWORDS
    FRAMELESSHELPER_CORE_LIBRARY
)

if(APPLE)
    target_link_libraries(${SUB_PROJ_NAME} PRIVATE
        "-framework Foundation"
        "-framework Cocoa"
        "-framework AppKit"
    )
elseif(UNIX)
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
    "$<BUILD_INTERFACE:${INCLUDE_PREFIX}>"
    "$<BUILD_INTERFACE:${INCLUDE_PREFIX}/private>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}/private>"
)

include(cmakehelper.cmake)
setup_compile_params(${SUB_PROJ_NAME})
setup_package_export(${SUB_PROJ_NAME} ${SUB_PROJ_PATH} "${PUBLIC_HEADERS}" "${PUBLIC_HEADERS_ALIAS}" "${PRIVATE_HEADERS}")
