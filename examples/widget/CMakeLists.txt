set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(SOURCES
    ../images.qrc
    widget.h
    widget.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES ../example.rc ../example.manifest)
endif()

add_executable(Widget WIN32 ${SOURCES})

target_link_libraries(Widget PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    wangwenx190::FramelessHelper
)

target_compile_definitions(Widget PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060100
)

if(WIN32)
    target_link_libraries(Widget PRIVATE dwmapi)
endif()
