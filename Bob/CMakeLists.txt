# Dedicated `bob` executable (REQ-LAUNCH-02). Shares this project's compat
# layer (ProjectDefaults/WIN32_COMPAT/RowanMFC) — the BoB game module lands
# incrementally from bob-flight-sim's mfc/ tree onto it (T116 inventory).
get_filename_component(ROWAN_ENGINE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../rowan-engine" ABSOLUTE)
get_filename_component(BOB_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../battle-of-britain/source-ported/bob-flight-sim" ABSOLUTE)

add_executable(bob
    bob_main.cpp
    ${ROWAN_ENGINE}/shell/detect.cpp
    ${ROWAN_ENGINE}/shell/manifests.cpp
)

target_include_directories(bob PRIVATE ${ROWAN_ENGINE}/shell)
target_link_libraries(bob PRIVATE ProjectDefaults)

# ---------------------------------------------------------------------------
# BoB frontend compile environment (T118).
#
# BoB sources spell #include names in every case variation plus Windows-style
# "..\dir\file" paths. Rather than editing ~400 spellings, TUs compile against
# a generated overlay dir of case-folded symlinks (rowan-engine's
# make_bob_overlay.sh encodes the precedence rules: ported r-widget headers
# first, mfc/ guard-forwarders before h/, vendored vs2005 only when no earlier
# -I dir provides the spelling).
#
# "../mfc/x"-style relative includes resolve against the opened file's dir —
# i.e. <overlay>/../mfc/x — so mfc/h/lib3d sibling links into bob-flight-sim
# sit beside the overlay.
# ---------------------------------------------------------------------------
set(BOB_OVERLAY ${CMAKE_BINARY_DIR}/bob_overlay)

execute_process(COMMAND bash
    ${ROWAN_ENGINE}/tools/build/make_bob_overlay.sh
    ${BOB_OVERLAY}
    ${CMAKE_SOURCE_DIR}/H
    # shadow dirs = every -I dir that precedes the overlay (port shims,
    # mig compat, rowan compat, sysroot/SDL)
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${CMAKE_SOURCE_DIR}/H
    ${ROWAN_ENGINE}/compat
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/../../sysroot-i386/usr/include
    RESULT_VARIABLE _ov_rc
    OUTPUT_VARIABLE _ov_out)
if(NOT _ov_rc EQUAL 0)
    message(FATAL_ERROR "bob_overlay generation failed: ${_ov_out}")
endif()
message(STATUS "BoB overlay: ${_ov_out}")

foreach(d mfc h lib3d)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/${d})
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${BOB_SRC}/${d} ${CMAKE_BINARY_DIR}/${d})
    endif()
endforeach()

# Frontend object library: every standalone mfc/ TU (unity aggregations
# _*.cpp are excluded — they #include sibling TUs and are not standalone).
file(GLOB BOB_MFC_SOURCES CONFIGURE_DEPENDS ${BOB_SRC}/mfc/*.cpp)
list(FILTER BOB_MFC_SOURCES EXCLUDE REGEX "/_[^/]+\\.cpp$")

add_library(bob_frontend STATIC ${BOB_MFC_SOURCES})
target_include_directories(bob_frontend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${BOB_OVERLAY}
    ${CMAKE_SOURCE_DIR}/H
    ${ROWAN_ENGINE}/compat
)
target_compile_options(bob_frontend PRIVATE -fpermissive)
target_link_libraries(bob_frontend PRIVATE ProjectDefaults)
