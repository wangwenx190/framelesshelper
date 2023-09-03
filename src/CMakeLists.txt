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

set(_targets)
set(_doc_input_dirs)

if(TARGET Qt${QT_VERSION_MAJOR}::Core AND TARGET Qt${QT_VERSION_MAJOR}::Gui)
    add_subdirectory(core)
    list(APPEND _targets ${PROJECT_NAME}::Core)
    list(APPEND _doc_input_dirs core ../include/FramelessHelper/Core)
endif()

if(FRAMELESSHELPER_BUILD_WIDGETS AND TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    add_subdirectory(widgets)
    list(APPEND _targets ${PROJECT_NAME}::Widgets)
    list(APPEND _doc_input_dirs widgets ../include/FramelessHelper/Widgets)
endif()

if(FRAMELESSHELPER_BUILD_QUICK AND TARGET Qt${QT_VERSION_MAJOR}::Quick)
    add_subdirectory(quick)
    list(APPEND _targets ${PROJECT_NAME}::Quick)
    list(APPEND _doc_input_dirs quick ../include/FramelessHelper/Quick)
endif()

if(FRAMELESSHELPER_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen QUIET)

        if(NOT DOXYGEN_FOUND)
            message(WARNING "Doxygen executable not found, skip building documentations.")
            return()
        endif()
    endif()

    include(../doxygen/doxygen.cmake)

    set(_install_options)

    if(NOT FRAMELESSHELPER_NO_INSTALL)
        set(_install_options INSTALL_DIR share/doc/FramelessHelper)
    endif()

    setup_doxygen_command(FramelessHelper_RunDoxygen
        DESCRIPTION "Qt Window Customization Framework"
        MDFILE ../doc/doxygen_mainpage.md
        OUTPUT_DIR ${CMAKE_BINARY_DIR}
        INPUT ${_doc_input_dirs}
        TARGETS ${_targets}
        DEPENDS ${_targets}
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=Q_SIGNALS Q_SLOTS=Q_SLOTS
        ${_install_options}
    )
endif()