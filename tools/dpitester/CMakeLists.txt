cmake_minimum_required(VERSION 3.24)

project(DpiTester LANGUAGES RC CXX)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

string(REGEX REPLACE "[-|/]GR" "/GR-" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
#string(REGEX REPLACE "[-|/]EHsc" "/EHs-c-" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REGEX REPLACE "[-|/]W[0|1|2|3|4]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REGEX REPLACE "[-|/]Ob[0|1|2|3]" "/Ob3" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    app.rc app.manifest main.cpp
)

set(_WIN32_WINNT_WIN10 0x0A00)
set(NTDDI_WIN10_CO 0x0A00000B)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
    _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
    _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
    WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
    _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    /await:strict /bigobj /d2FH4 /MP /FS /utf-8 /WX /permissive- /ZH:SHA_256
    /Zc:char8_t,__cplusplus,externC,externConstexpr,hiddenFriend,lambda,preprocessor,referenceBinding,rvalueCast,strictStrings,ternary,throwingNew,trigraphs
    $<$<CONFIG:Debug>:/JMC>
    $<$<NOT:$<CONFIG:Debug>>:/guard:cf /guard:ehcont /GA /GT /Gw /Gy /Oi /Oy /QIntel-jcc-erratum /Qspectre-load /Zc:inline>
)

target_link_options(${PROJECT_NAME} PRIVATE
    /DYNAMICBASE /HIGHENTROPYVA /LARGEADDRESSAWARE /NXCOMPAT /TSAWARE /WX
    $<$<NOT:$<CONFIG:Debug>>:/CETCOMPAT /GUARD:CF /GUARD:EHCONT /OPT:REF /OPT:ICF>
)
