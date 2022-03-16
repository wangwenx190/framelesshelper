set(SOURCES
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
    FramelessHelperCore
    FramelessHelperWidgets
)

target_compile_definitions(Widget PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
)
