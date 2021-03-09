set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick QuickControls2 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick QuickControls2 REQUIRED)

set(SOURCES
    ../images.qrc
    qml.qrc
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES ../windows.rc ../windows.manifest)
endif()

add_executable(Quick WIN32 ${SOURCES})

target_link_libraries(Quick PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    wangwenx190::FramelessHelper
)

target_compile_definitions(Quick PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

if(MSVC)
    target_compile_options(Quick PRIVATE /utf-8)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        target_compile_options(Quick PRIVATE /guard:cf)
        target_link_options(Quick PRIVATE /GUARD:CF)
    endif()
endif()

if(WIN32)
    target_link_libraries(Quick PRIVATE user32 shell32 gdi32 dwmapi)
endif()
