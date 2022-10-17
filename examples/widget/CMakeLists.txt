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

set(SOURCES
    ../shared/log.h
    ../shared/log.cpp
    ../shared/settings.h
    ../shared/settings.cpp
    widget.h
    widget.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES
        ../shared/example.rc
        ../shared/example.manifest
    )
endif()

add_executable(Widget ${SOURCES})

target_link_libraries(Widget PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    FramelessHelper::Widgets
)

target_compile_definitions(Widget PRIVATE
    QT_NO_KEYWORDS
)

include(../../src/core/cmakehelper.cmake)
setup_gui_app(Widget)
setup_compile_params(Widget)
if(FRAMELESSHELPER_EXAMPLES_DEPLOYQT)
    deploy_qt_runtime(Widget)
endif()
