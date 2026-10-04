# Unit tests for the native MiG Alley port (32-bit).
#
# Exercises the real 1998 code paths (shape instruction handlers, matrix
# clip/projection math, palette selection, fixed-point union layout) through
# the same static libraries the game links.

add_executable(unit_tests
    unit_tests.cpp
    ${CMAKE_SOURCE_DIR}/MigAlley/MigCString.cpp
)

target_link_libraries(unit_tests PRIVATE ProjectDefaults)

target_link_libraries(unit_tests PRIVATE
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

add_test(NAME unit_tests COMMAND unit_tests)

# The 32-bit sysroot SDL/mixer libs aren't on the default loader path.
set_tests_properties(unit_tests PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/../../sysroot-i386/usr/lib/i386-linux-gnu"
)
