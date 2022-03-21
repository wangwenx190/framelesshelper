set(SUB_PROJ_NAME FramelessHelperCore)

set(INCLUDE_PREFIX ../../include/FramelessHelper/Core)

set(SOURCES
    ${INCLUDE_PREFIX}/framelesshelpercore_global.h
    ${INCLUDE_PREFIX}/framelesshelper_qt.h
    ${INCLUDE_PREFIX}/framelesswindowsmanager.h
    ${INCLUDE_PREFIX}/utils.h
    framelesshelpercore.qrc
    framelesswindowsmanager_p.h
    utils.cpp
    framelesshelper_qt.cpp
    framelesswindowsmanager.cpp
)

if(WIN32)
    list(APPEND SOURCES
        ${INCLUDE_PREFIX}/framelesshelper_windows.h
        ${INCLUDE_PREFIX}/framelesshelper_win.h
        qwinregistry_p.h
        qwinregistry.cpp
        utils_win.cpp
        framelesshelper_win.cpp
    )
elseif(APPLE)
    list(APPEND SOURCES utils_mac.cpp)
elseif(UNIX)
    list(APPEND SOURCES utils_linux.cpp)
endif()

if(WIN32 AND NOT FRAMELESSHELPER_BUILD_STATIC)
    enable_language(RC)
    list(APPEND SOURCES framelesshelpercore.rc)
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    add_library(${SUB_PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${SUB_PROJ_NAME} SHARED ${SOURCES})
endif()

if(FRAMELESSHELPER_BUILD_STATIC)
    target_compile_definitions(${SUB_PROJ_NAME} PUBLIC
        FRAMELESSHELPER_CORE_STATIC
    )
endif()

target_compile_definitions(${SUB_PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    FRAMELESSHELPER_CORE_LIBRARY
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
        /utf-8 /W4 /WX
    )
else()
    target_compile_options(${SUB_PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()

target_link_libraries(${SUB_PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::GuiPrivate
)

target_include_directories(${SUB_PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/${INCLUDE_PREFIX}"
)
