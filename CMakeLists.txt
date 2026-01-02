cmake_minimum_required(VERSION 3.15)
project(HexViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable CRT, RTTI, exceptions (match your batch script)
if (MSVC)
    add_compile_options(
        /nologo /W3 /O2 /std:c++17
        /GS- /GR- /EHs- /EHc- /EHa- /RTC- /sdl- /Zl
        /D_CRT_SECURE_NO_WARNINGS
        /D_HAS_EXCEPTIONS=0
    )
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/ui
    ${CMAKE_SOURCE_DIR}/include/system
)

set(SOURCES
    src/hexviewer.cpp
    src/core/hexdata.cpp
    src/core/render.cpp
    src/core/panelcontent.cpp
    src/ui/menu.cpp
    src/ui/options.cpp
    src/ui/searchdialog.cpp
    src/ui/contextmenu.cpp
)

add_executable(HexViewer ${SOURCES})

if (WIN32)
    target_link_options(HexViewer PRIVATE
        /SUBSYSTEM:WINDOWS
        /ENTRY:entry
        /NODEFAULTLIB
    )

    target_link_libraries(HexViewer PRIVATE
        kernel32.lib
        user32.lib
        gdi32.lib
        comdlg32.lib
        dwmapi.lib
        advapi32.lib
        shell32.lib
    )
endif()

if (APPLE)
    target_link_libraries(HexViewer
        "-framework Cocoa"
        "-framework AppKit"
        "-framework CoreFoundation"
    )
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(HexViewer
        X11
        pthread
        dl
    )
endif()
