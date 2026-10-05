# Integration tests for the native MiG Alley port.
#
# smoke_xvfb: launches the real binary on a private Xvfb display, clicks
# through the intro video, exercises mouse moves and verifies the menu
# actually renders (catches the stuck-drag / white-screen regressions).
# Skips (exit 77) when Xvfb/xdotool/ImageMagick or the runtime game data
# are unavailable.

find_program(BASH_EXE bash REQUIRED)

add_subdirectory(unit)

add_test(
    NAME smoke_xvfb
    COMMAND ${BASH_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/smoke_xvfb.sh $<TARGET_FILE:MigAlley>
)

set_tests_properties(smoke_xvfb PROPERTIES
    TIMEOUT 240
    SKIP_RETURN_CODE 77
)

# lifecycle_xvfb: REQ-LIFE-01 — repeated gated launches on a private Xvfb,
# per-cycle fd/RSS/maps sampling; fails on survivors or post-warm-up growth.
add_test(
    NAME lifecycle_xvfb
    COMMAND ${BASH_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/lifecycle_xvfb.sh $<TARGET_FILE:MigAlley>
)

set_tests_properties(lifecycle_xvfb PROPERTIES
    TIMEOUT 600
    SKIP_RETURN_CODE 77
)
