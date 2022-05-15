set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui OpenGL Qml Quick QuickControls2)

set(SOURCES
    ../images.qrc
    rendercontrol.qrc
    main.cpp
    window_singlethreaded.cpp
)

add_executable(rendercontrol_openglexample WIN32 ${SOURCES})

target_link_libraries(rendercontrol_openglexample PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QuickControls2
    wangwenx190::FramelessHelper
)

target_compile_definitions(Quick PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060200
)
