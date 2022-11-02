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
set(SUB_PROJ_PATH ${PROJECT_NAME}/${SUB_MOD_NAME})

set(INCLUDE_PREFIX ../../include/${SUB_PROJ_PATH})

set(PUBLIC_HEADERS
    ${INCLUDE_PREFIX}/framelesshelperquick_global.h
    ${INCLUDE_PREFIX}/framelessquickmodule.h
    ${INCLUDE_PREFIX}/framelessquickhelper.h
    ${INCLUDE_PREFIX}/framelessquickutils.h
    ${INCLUDE_PREFIX}/quickchromepalette.h
    ${INCLUDE_PREFIX}/quickmicamaterial.h
    ${INCLUDE_PREFIX}/quickimageitem.h
    ${INCLUDE_PREFIX}/quickwindowborder.h
)

set(PUBLIC_HEADERS_ALIAS
    ${INCLUDE_PREFIX}/Global
    ${INCLUDE_PREFIX}/FramelessQuickModule
    ${INCLUDE_PREFIX}/FramelessQuickHelper
    ${INCLUDE_PREFIX}/FramelessQuickUtils
    ${INCLUDE_PREFIX}/QuickChromePalette
    ${INCLUDE_PREFIX}/QuickMicaMaterial
    ${INCLUDE_PREFIX}/QuickImageItem
    ${INCLUDE_PREFIX}/QuickWindowBorder
)

set(PRIVATE_HEADERS
    ${INCLUDE_PREFIX}/private/quickstandardsystembutton_p.h
    ${INCLUDE_PREFIX}/private/quickstandardtitlebar_p.h
    ${INCLUDE_PREFIX}/private/framelessquickhelper_p.h
    ${INCLUDE_PREFIX}/private/framelessquickwindow_p.h
    ${INCLUDE_PREFIX}/private/framelessquickwindow_p_p.h
    ${INCLUDE_PREFIX}/private/framelessquickapplicationwindow_p.h
    ${INCLUDE_PREFIX}/private/framelessquickapplicationwindow_p_p.h
    ${INCLUDE_PREFIX}/private/quickmicamaterial_p.h
    ${INCLUDE_PREFIX}/private/quickimageitem_p.h
    ${INCLUDE_PREFIX}/private/quickwindowborder_p.h
)

set(SOURCES
    quickstandardsystembutton.cpp
    quickstandardtitlebar.cpp
    framelessquickutils.cpp
    framelessquickmodule.cpp
    framelessquickwindow.cpp
    framelessquickapplicationwindow.cpp
    framelessquickhelper.cpp
    quickchromepalette.cpp
    framelesshelperquick_global.cpp
    quickmicamaterial.cpp
    quickimageitem.cpp
    quickwindowborder.cpp
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

set(ALL_SOURCES ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${ALL_SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${ALL_SOURCES})
endif()
add_library(${PROJECT_NAME}::${SUB_PROJ_NAME} ALIAS ${SUB_PROJ_NAME})
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

set(__import_base_dir ${PROJECT_BINARY_DIR}/qml)
if(DEFINED FRAMELESSHELPER_IMPORT_DIR)
    set(__import_base_dir ${FRAMELESSHELPER_IMPORT_DIR})
endif()
set(__import_uri org/wangwenx190/${PROJECT_NAME})
set(__import_dir ${__import_base_dir}/${__import_uri})

if(${QT_VERSION} VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(${SUB_PROJ_NAME}
        URI org.wangwenx190.${PROJECT_NAME}
        VERSION 1.0
        OUTPUT_DIRECTORY "${__import_dir}"
        IMPORTS
            QtQml/auto
            QtQuick/auto
            QtQuick.Controls.Basic/auto
    )
    set(__lib_prefix)
    if(UNIX)
        set(__lib_prefix lib)
    endif()
    set(__lib_suffix)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(__lib_suffix ${CMAKE_DEBUG_POSTFIX})
    endif()
    set(__lib_ext)
    if(FRAMELESSHELPER_BUILD_STATIC)
        if(WIN32)
            set(__lib_ext lib)
        else()
            set(__lib_ext a)
        endif()
    else()
        if(WIN32)
            set(__lib_ext dll)
        elseif(APPLE)
            set(__lib_ext dylib)
        elseif(UNIX)
            set(__lib_ext so)
        endif()
    endif()
    install(FILES
        "${__import_dir}/qmldir"
        "${__import_dir}/${SUB_PROJ_NAME}.qmltypes"
        "${__import_dir}/${__lib_prefix}${SUB_PROJ_NAME}plugin${__lib_suffix}.${__lib_ext}"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/qml/${__import_uri}
    )
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_QUICK_STATIC
    )
endif()

if(FRAMELESSHELPER_NO_DEBUG_OUTPUT)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    FRAMELESSHELPER_QUICK_LIBRARY
)

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::QuickTemplates2Private
    Qt${QT_VERSION_MAJOR}::QuickControls2Private
)

target_link_libraries(${SUB_PROJ_NAME} PUBLIC
    ${PROJECT_NAME}::Core
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PREFIX}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PREFIX}/private>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}/private>"
)

include(../core/cmakehelper.cmake)
setup_compile_params(${SUB_PROJ_NAME})
setup_package_export(${SUB_PROJ_NAME} ${SUB_PROJ_PATH} "${PUBLIC_HEADERS}" "${PUBLIC_HEADERS_ALIAS}" "${PRIVATE_HEADERS}")
