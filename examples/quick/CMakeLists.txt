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
    list(APPEND SOURCES ../example.rc ../example.manifest)
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
    QT_DISABLE_DEPRECATED_BEFORE=0x060100
)

if(WIN32)
    target_link_libraries(Quick PRIVATE dwmapi)
endif()
