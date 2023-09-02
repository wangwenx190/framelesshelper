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

if(TARGET Qt${QT_VERSION_MAJOR}::Core AND TARGET Qt${QT_VERSION_MAJOR}::Gui)
    add_subdirectory(core)
    list(APPEND _targets ${PROJECT_NAME}::Core)
endif()

if(FRAMELESSHELPER_BUILD_WIDGETS AND TARGET Qt${QT_VERSION_MAJOR}::Widgets)
    add_subdirectory(widgets)
    list(APPEND _targets ${PROJECT_NAME}::Widgets)
endif()

if(FRAMELESSHELPER_BUILD_QUICK AND TARGET Qt${QT_VERSION_MAJOR}::Quick)
    add_subdirectory(quick)
    list(APPEND _targets ${PROJECT_NAME}::Quick)
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
    setup_doxygen_command(FramelessHelper_RunDoxygen
        DESCRIPTION "Qt Window Customization Framework"
        MDFILE ../doc/doxygen_mainpage.md
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/html
        INPUT . ../include
        TARGETS ${_targets}
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=signals
    )
endif()