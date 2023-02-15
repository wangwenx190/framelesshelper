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

set(SOURCES
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    dialog.h
    dialog.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES
        ../shared/example.rc
        ../shared/example.manifest
    )
endif()

add_executable(Dialog ${SOURCES})

target_link_libraries(Dialog PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    FramelessHelper::Widgets
)

include(../../cmake/utils.cmake)
setup_gui_app(
    TARGETS Dialog
    BUNDLE_ID org.wangwenx190.Dialog
    BUNDLE_VERSION 1.0.0
    BUNDLE_VERSION_SHORT 1.0
)
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
setup_compile_params(TARGETS Dialog ${__extra_flags})
if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    set(__extra_flags)
    if(FRAMELESSHELPER_NO_INSTALL)
        set(__extra_flags NO_INSTALL)
    endif()
    deploy_qt_runtime(TARGET Dialog ${__extra_flags})
endif()
