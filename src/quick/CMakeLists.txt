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

include(GNUInstallDirs)

if(FRAMELESSHELPER_ENABLE_UNIVERSAL_BUILD)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

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
    set(__rc_path "${CMAKE_CURRENT_BINARY_DIR}/${SUB_PROJ_NAME}.rc")
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
    set(SUB_MOD_LIB_TYPE "STATIC")
else()
    set(SUB_MOD_LIB_TYPE "SHARED")
endif()
add_library(${SUB_PROJ_NAME} ${SUB_MOD_LIB_TYPE} ${ALL_SOURCES})
add_library(${PROJECT_NAME}::${SUB_PROJ_NAME} ALIAS ${SUB_PROJ_NAME})
add_library(${PROJECT_NAME}::${SUB_MOD_NAME} ALIAS ${SUB_PROJ_NAME})

set_target_properties(${SUB_PROJ_NAME} PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    set(SUB_MOD_LIB_DIR "${CMAKE_INSTALL_BINDIR}")
else()
    set(SUB_MOD_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
endif()

set(__prefix "")
if(NOT WIN32)
    set(__prefix "lib")
endif()
set(__suffix "")
if(FRAMELESSHELPER_BUILD_STATIC)
    if(MSVC)
        set(__suffix "lib")
    else()
        set(__suffix "a")
    endif()
else()
    if(WIN32)
        set(__suffix "dll")
    elseif(APPLE)
        set(__suffix "dylib")
    elseif(UNIX)
        set(__suffix "so")
    endif()
endif()
set(SUB_MOD_FILE_PREFIX "${__prefix}")
set(SUB_MOD_FILE_SUFFIX "${__suffix}")
set(SUB_MOD_FILE_BASENAME "${SUB_MOD_FILE_PREFIX}${SUB_PROJ_NAME}")
if("x${CMAKE_BUILD_TYPE}" STREQUAL "xDebug")
    string(APPEND SUB_MOD_FILE_BASENAME "${CMAKE_DEBUG_POSTFIX}")
endif()
set(SUB_MOD_FILE_NAME "${SUB_MOD_FILE_BASENAME}.${SUB_MOD_FILE_SUFFIX}")
unset(__suffix)
unset(__prefix)

set(__import_base_dir "${PROJECT_BINARY_DIR}/imports")
if(DEFINED FRAMELESSHELPER_IMPORT_DIR)
    set(__import_base_dir "${FRAMELESSHELPER_IMPORT_DIR}")
endif()

# qt_add_qml_module() was introduced in Qt 6.2 but qt_query_qml_module()
# was introduced in 6.3, to simplify the CMake code, I decide to limit the
# version check to 6.3, otherwise we'll need a lot of code to query and
# calculate the generated files, which will also break Ninja Multi-Config
# builds.
if(QT_VERSION VERSION_GREATER_EQUAL "6.3")
    qt_add_qml_module(${SUB_PROJ_NAME}
        URI "org.wangwenx190.${PROJECT_NAME}"
        VERSION "1.0"
        OUTPUT_DIRECTORY "${__import_base_dir}/org/wangwenx190/${PROJECT_NAME}"
        RESOURCE_PREFIX "/"
        #NO_RESOURCE_TARGET_PATH # Can't be used for non-executables.
        OUTPUT_TARGETS __qml_targets
        IMPORTS
            QtQml/auto
            QtQuick/auto
            QtQuick.Controls.Basic/auto
    )
    if(__qml_targets)
        foreach(__target ${__qml_targets})
            # We have some resources bundled into the library,
            # however, for static builds, the object files will
            # not be packed into our final static library file,
            # and thus it causes linker errors for our users,
            # so we need this hack here.
            if(FRAMELESSHELPER_BUILD_STATIC)
                target_sources(${SUB_PROJ_NAME} PRIVATE
                    $<TARGET_OBJECTS:${__target}>
                )
            endif()
        endforeach()
    endif()
    if(NOT FRAMELESSHELPER_NO_INSTALL)
        qt_query_qml_module(${SUB_PROJ_NAME}
            URI module_uri
            VERSION module_version
            PLUGIN_TARGET module_plugin_target
            TARGET_PATH module_target_path
            QMLDIR module_qmldir
            TYPEINFO module_typeinfo
            #QML_FILES module_qml_files
            #QML_FILES_DEPLOY_PATHS module_qml_files_deploy_paths
            #RESOURCES module_resources
            #RESOURCES_DEPLOY_PATHS module_resources_deploy_paths
        )
        if(module_target_path)
            set(__qml_plugin_dir "qml/${module_target_path}")
            if(module_plugin_target)
                install(TARGETS ${module_plugin_target}
                    RUNTIME DESTINATION "${__qml_plugin_dir}"
                    LIBRARY DESTINATION "${__qml_plugin_dir}"
                    ARCHIVE DESTINATION "${__qml_plugin_dir}"
                )
            endif()
            if(module_qmldir)
                install(FILES "${module_qmldir}" DESTINATION "${__qml_plugin_dir}")
            endif()
            if(module_typeinfo)
                install(FILES "${module_typeinfo}" DESTINATION "${__qml_plugin_dir}")
            endif()
            if(module_qml_files)
                list(LENGTH module_qml_files num_files)
                math(EXPR last_index "${num_files} - 1")
                foreach(i RANGE 0 ${last_index})
                    list(GET module_qml_files ${i} src_file)
                    list(GET module_qml_files_deploy_paths ${i} deploy_path)
                    get_filename_component(dest_dir "${deploy_path}" DIRECTORY)
                    install(FILES "${src_file}" DESTINATION "${__qml_plugin_dir}/${dest_dir}")
                endforeach()
            endif()
            if(module_resources)
                list(LENGTH module_resources num_files)
                math(EXPR last_index "${num_files} - 1")
                foreach(i RANGE 0 ${last_index})
                    list(GET module_resources ${i} src_file)
                    list(GET module_resources_deploy_paths ${i} deploy_path)
                    get_filename_component(dest_dir "${deploy_path}" DIRECTORY)
                    install(FILES "${src_file}" DESTINATION "${__qml_plugin_dir}/${dest_dir}")
                endforeach()
            endif()
        endif()
    endif()
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    set(__def FRAMELESSHELPER_QUICK_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC ${__def})
    list(APPEND SUB_MOD_DEFS ${__def})
    unset(__def)
endif()

if(FRAMELESSHELPER_NO_DEBUG_OUTPUT)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
    )
endif()

if(FRAMELESSHELPER_NO_BUNDLE_RESOURCE)
    set(__def FRAMELESSHELPER_QUICK_NO_BUNDLE_RESOURCE)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC ${__def})
    list(APPEND SUB_MOD_DEFS ${__def})
    unset(__def)
endif()

if(FRAMELESSHELPER_NO_PRIVATE)
    set(__def FRAMELESSHELPER_QUICK_NO_PRIVATE)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC ${__def})
    list(APPEND SUB_MOD_DEFS ${__def})
    unset(__def)
endif()

if(DEFINED FRAMELESSHELPER_NAMESPACE)
    if("x${FRAMELESSHELPER_NAMESPACE}" STREQUAL "x")
        message(FATAL_ERROR "FRAMELESSHELPER_NAMESPACE can't be empty!")
    endif()
    set(__def FRAMELESSHELPER_NAMESPACE=${FRAMELESSHELPER_NAMESPACE})
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC ${__def})
    list(APPEND SUB_MOD_DEFS ${__def})
    unset(__def)
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    FRAMELESSHELPER_QUICK_LIBRARY
)

if(FRAMELESSHELPER_NO_PRIVATE)
    target_link_libraries(${SUB_PROJ_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Quick
    )
else()
    target_link_libraries(${SUB_PROJ_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::QuickPrivate
        Qt${QT_VERSION_MAJOR}::QuickTemplates2Private
        Qt${QT_VERSION_MAJOR}::QuickControls2Private
    )
endif()

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

setup_qt_stuff(TARGETS ${SUB_PROJ_NAME} ALLOW_KEYWORD)
set(__extra_flags)
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
if(FRAMELESSHELPER_ENABLE_INTELJCC)
    list(APPEND __extra_flags INTELJCC)
endif()
if(FRAMELESSHELPER_ENABLE_CFGUARD)
    list(APPEND __extra_flags CFGUARD)
endif()
setup_compile_params(TARGETS ${SUB_PROJ_NAME} ${__extra_flags})
if(NOT FRAMELESSHELPER_NO_INSTALL)
    set(__cmake_dir "${CMAKE_CURRENT_BINARY_DIR}/cmake")
    set(__config_file "${__cmake_dir}/${SUB_PROJ_NAME}Config.cmake")
    configure_file(../../FramelessHelperModuleConfig.cmake.in ${__config_file} @ONLY)
    set(__targets_file "${__cmake_dir}/${SUB_PROJ_NAME}Targets.cmake")
    configure_file(../../FramelessHelperModuleTargets.cmake.in ${__targets_file} @ONLY)
    install(
        FILES "${__config_file}" "${__targets_file}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${SUB_PROJ_NAME}"
    )
    set(__inc_dir "${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}")
    install(
        FILES ${PUBLIC_HEADERS} ${PUBLIC_HEADERS_ALIAS}
        DESTINATION "${__inc_dir}"
    )
    install(
        FILES ${PRIVATE_HEADERS}
        DESTINATION "${__inc_dir}/private"
    )
    install(
        TARGETS  ${SUB_PROJ_NAME}
        RUNTIME  DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        INCLUDES DESTINATION "${__inc_dir}"
    )
endif()

if(NOT FRAMELESSHELPER_NO_SUMMARY)
    dump_target_info(TARGETS ${SUB_PROJ_NAME})
endif()
