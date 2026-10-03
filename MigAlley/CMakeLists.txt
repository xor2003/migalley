cmake_minimum_required(VERSION 3.16)
project(MigAlley LANGUAGES CXX)

if (WIN32)
    target_sources(MigAlley PRIVATE MIG.RC)
endif()

add_executable(MigAlley
    MigCString.cpp
    MigAlley.cpp
)

target_precompile_headers(MigAlley PRIVATE stdafx.h)

# Inherit defaults
target_link_libraries(MigAlley PRIVATE ProjectDefaults)

# Extra include dirs specific to MigAlley
target_include_directories(MigAlley PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../RowanUI
    C:/DX81SDK/include
)

# Internal libraries
target_link_libraries(MigAlley PRIVATE
 -Wl,--start-group
    My3D
    Assembler
    AI
    Bfields
    Overlay
    Comms
    Files
    General
    Graphics
    Input
    Math
    Movecode
    Missman
    Hardware
    RowanMFC
    RowanAFX
    RowanSheets
    CollisionsEtAl
    Model
    Aircraft
 -Wl,--end-group
)

# External libraries via find_package

target_link_libraries(MigAlley PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

if (WIN32)
# Windows system libraries
target_link_libraries(MigAlley PRIVATE
    kernel32 user32 gdi32 comdlg32 winspool winmm shell32 comctl32
    ole32 oleaut32 uuid rpcrt4 advapi32 wsock32 wininet
    opengl32 d3dx dplayx dsound dinput dxguid ddraw
)
endif()

if (MSVC)
    target_link_options(MigAlley PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd.lib>
    )
endif()

