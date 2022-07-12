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
)

set(PRIVATE_HEADERS
    ${INCLUDE_PREFIX}/private/framelesswidgetshelper_p.h
    ${INCLUDE_PREFIX}/private/standardsystembutton_p.h
    ${INCLUDE_PREFIX}/private/standardtitlebar_p.h
    ${INCLUDE_PREFIX}/private/framelesswidget_p.h
    ${INCLUDE_PREFIX}/private/framelessmainwindow_p.h
    ${INCLUDE_PREFIX}/private/widgetssharedhelper_p.h
)

set(SOURCES
    framelessmainwindow.cpp
    framelesswidgetshelper.cpp
    framelesswidget.cpp
    standardsystembutton.cpp
    standardtitlebar.cpp
    widgetssharedhelper.cpp
    global.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/framelesshelper.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelperwidgets.rc @ONLY)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/framelesshelperwidgets.rc
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

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_WIDGETS_STATIC
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
    FRAMELESSHELPER_WIDGETS_LIBRARY
)

if(MSVC)
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        /utf-8 /W3 /WX
    )
else()
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::WidgetsPrivate
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

install(TARGETS ${SUB_PROJ_NAME}
    EXPORT ${SUB_PROJ_NAME}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}
)

export(EXPORT ${SUB_PROJ_NAME}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${SUB_PROJ_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)

install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH})
install(FILES ${PRIVATE_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${SUB_PROJ_PATH}/private)

install(EXPORT ${SUB_PROJ_NAME}Targets
    FILE ${SUB_PROJ_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
