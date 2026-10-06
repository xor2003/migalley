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

# Native lib3d (T120a route C): BoB's own CLib3D compiled against the rowan
# DX7 emulation (rowan-engine/compat/{d3dtypes,d3d,ddraw}.h). Source list is
# the DSP/vcxproj/winelib-Makefile live set; texmgr.cpp + stdafx.cpp are not
# in any build list (PCH stub / superseded). Renders through state-recording
# no-ops for now — flight rendering is the later route-C render wave.
add_library(bob_lib3d STATIC
    ${BOB_SRC}/lib3d/lib3d.cpp
    ${BOB_SRC}/lib3d/polyvert.cpp
    ${BOB_SRC}/lib3d/flagsw.cpp
    ${BOB_SRC}/lib3d/l3dguid.cpp
    ${BOB_SRC}/lib3d/getdxver.cpp
    ${BOB_SRC}/lib3d/alloc.c
    ${BOB_SRC}/lib3d/radix.c
    ${BOB_SRC}/3d/monotxt.cpp)
# CXX-only project: the K&R .c files compile as C++ (MSVC treated them the
# same inside the lib3d project).
set_source_files_properties(
    ${BOB_SRC}/lib3d/alloc.c ${BOB_SRC}/lib3d/radix.c
    PROPERTIES LANGUAGE CXX)
target_include_directories(bob_lib3d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${BOB_OVERLAY}
    ${CMAKE_SOURCE_DIR}/H
    ${ROWAN_ENGINE}/compat
)
target_compile_options(bob_lib3d PRIVATE -fpermissive)
target_link_libraries(bob_lib3d PRIVATE ProjectDefaults)

# ---------------------------------------------------------------------------
# BoB game module (T121) — the authoritative Release-config TU set from
# mfc/bob.vcproj.
#
# MSVC Release compiled just 32 TUs: the game logic lives in 13 unity files
# (_AI.CPP ... _3D.CPP) that #include sibling .cpps directly; the mfc-side
# unities (_MFC/_AFX/_LW/_RAF/_sA/_full/_tool) are skipped here because
# bob_frontend already holds every mfc/*.cpp individually. keystub.cpp is
# added explicitly — _MFC.CPP used to pull it.
#
# Every entry is UsePrecompiledHeader="0" in the vcproj, so no -include
# injection: unity TUs supply their own context via _*.h umbrellas.
# ROWAN_USE_MIG_CSTRING makes h/cstring.h adopt the shared CString (required
# — otherwise BoB's local class collides with MigCstring across TU ABI).
# ---------------------------------------------------------------------------
add_library(bob_game STATIC
    # unity aggregations (each pulls its sibling .cpps internally)
    ${BOB_SRC}/3d/_3d.cpp
    ${BOB_SRC}/ai/_ai.cpp
    ${BOB_SRC}/aircraft/_airc.cpp
    ${BOB_SRC}/bfields/_bfie.cpp
    ${BOB_SRC}/comms/_comm.cpp
    ${BOB_SRC}/files/_file.cpp
    ${BOB_SRC}/general/_gene.cpp
    ${BOB_SRC}/hardware/_hard.cpp
    ${BOB_SRC}/input/_inpu.cpp
    ${BOB_SRC}/math/_math.cpp
    ${BOB_SRC}/missman/_miss.cpp
    ${BOB_SRC}/model/_mode.cpp
    ${BOB_SRC}/movecode/_move.cpp
    # standalone 3d TUs (not in _3D.CPP's include list)
    ${BOB_SRC}/3d/jimcol.cpp
    ${BOB_SRC}/3d/overlay.cpp
    ${BOB_SRC}/3d/viewsel.cpp
    ${BOB_SRC}/3d/smktrls.cpp
    ${BOB_SRC}/3d/tilemake.cpp
    ${BOB_SRC}/3d/weappak.cpp
    # standalone game TUs
    ${BOB_SRC}/missman/nodebob.cpp
    ${BOB_SRC}/movecode/sagmove.cpp
    ${BOB_SRC}/input/keystub.cpp
)
target_include_directories(bob_game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${BOB_OVERLAY}
    ${CMAKE_SOURCE_DIR}/H
    ${ROWAN_ENGINE}/compat
)
target_compile_definitions(bob_game PRIVATE ROWAN_USE_MIG_CSTRING)
target_compile_options(bob_game PRIVATE -fpermissive -Wno-narrowing)
target_link_libraries(bob_game PRIVATE ProjectDefaults)

# T120b smoke: stub3d's init sequence (Create/Initialise/GetDrivers/GetModes/
# GetDefaults/SetDriverAndMode/Release) driven for real through libbob_lib3d.
add_executable(bob_lib3d_smoke tests/lib3d_smoke.cpp tests/win32_leaves.cpp)
target_include_directories(bob_lib3d_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${BOB_OVERLAY}
    ${CMAKE_SOURCE_DIR}/H
    ${ROWAN_ENGINE}/compat
    ${BOB_SRC}/lib3d
)
target_compile_options(bob_lib3d_smoke PRIVATE -fpermissive)
target_link_libraries(bob_lib3d_smoke PRIVATE bob_lib3d ProjectDefaults)
add_test(NAME bob_lib3d_smoke COMMAND bob_lib3d_smoke)
