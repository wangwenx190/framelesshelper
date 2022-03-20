set(SUB_PROJ_NAME FramelessHelperQuick)

set(SOURCES
    framelesshelperquick_global.h
    framelessquickhelper.h
    framelessquickhelper.cpp
    framelessquickutils.h
    framelessquickutils.cpp
    framelesshelperimageprovider.h
    framelesshelperimageprovider.cpp
    framelessquickeventfilter.h
    framelessquickeventfilter.cpp
    framelesshelper_quick.h
    framelesshelper_quick.cpp
)

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    list(APPEND SOURCES framelesshelperquick.rc)
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${SOURCES})
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_QUICK_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    FRAMELESSHELPER_QUICK_LIBRARY
)

if(MSVC)
    set(_WIN32_WINNT_WIN10 0x0A00)
    set(NTDDI_WIN10_CO 0x0A00000B)
    target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
        _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
        _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
        WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
        _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    )
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        /W4 /WX
    )
else()
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
)

target_link_libraries(${SUB_PROJ_NAME} PUBLIC
    FramelessHelperCore
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../include/FramelessHelper/Quick"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../include"
)
