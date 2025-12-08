cmake_minimum_required(VERSION 3.15)
project(HexViewerExten LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HEADERS
    include/ClassFactory.h
    include/framework.h
    include/pch.h
    include/Reg.h
    include/ShellEx.h
    res/resource.h
)

set(SOURCES
    src/ClassFactory.cpp
    src/dllmain.cpp
    src/pch.cpp
    src/Reg.cpp
    src/ShellEx.cpp
)

set(RESOURCES
    res/Resource.rc
)

add_library(HexViewerExten SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

target_precompile_headers(HexViewerExten PRIVATE include/pch.h)

target_include_directories(HexViewerExten PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/res
)

target_compile_definitions(HexViewerExten PRIVATE
    DIESHELLEXTENSION_EXPORTS
    _WINDOWS
    _USRDLL
    UNICODE
    _UNICODE
)

if(WIN32)
    target_compile_definitions(HexViewerExten PRIVATE WIN32)
endif()

if(MSVC)
    target_compile_options(HexViewerExten PRIVATE /W3 /sdl /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(HexViewerExten PRIVATE
            /O1
            /GL
            /Gy
        )
        target_link_options(HexViewerExten PRIVATE
            /LTCG
            /OPT:REF
            /OPT:ICF
        )
        set_property(TARGET HexViewerExten PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    else()
        set_property(TARGET HexViewerExten PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    endif()
else()
    target_compile_options(HexViewerExten PRIVATE -Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(HexViewerExten PRIVATE -Os -s)
    endif()
endif()

if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/DEBUG" "" CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
endif()

if(MSVC)
    set_target_properties(HexViewerExten PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

set_target_properties(HexViewerExten PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

install(TARGETS HexViewerExten
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)