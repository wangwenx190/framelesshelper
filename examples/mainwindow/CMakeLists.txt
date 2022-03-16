set(SOURCES
    TitleBar.ui
    MainWindow.ui
    mainwindow.h
    mainwindow.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES ../example.rc ../example.manifest)
endif()

add_executable(MainWindow WIN32 ${SOURCES})

target_link_libraries(MainWindow PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    FramelessHelperWidgets
)

target_compile_definitions(MainWindow PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
)
