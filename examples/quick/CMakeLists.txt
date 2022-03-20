find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS QuickControls2)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickControls2)

set(SOURCES
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
    FramelessHelperQuick
)

target_compile_definitions(Quick PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
