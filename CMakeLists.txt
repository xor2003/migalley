cmake_minimum_required(VERSION 3.16)
project(MigAlley LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Needed so clang-check/clang-tidy/cppcheck can consume per-file flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MIG_SANITIZERS "Build with AddressSanitizer + UBSan (GCC, 32-bit)" OFF)
option(MIG_LINT "Run clang-tidy + cppcheck during the build" OFF)

# -------------------------------------------------------------------
# Dependencies via pkg-config (ensures 32-bit libs are found)
# -------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(VULKAN REQUIRED vulkan)

# Common settings for all in-house targets
add_library(ProjectDefaults INTERFACE)

if(NOT MSVC)
    # Force 32-bit build on GCC/Clang, and disable enum comparison warnings
    message(STATUS "Forcing 32-bit build: adding -m32 to CMAKE_CXX_FLAGS")
    # -fno-delete-null-pointer-checks: this codebase's MSVC-era idiom passes
    # null "references" (e.g. `=*(CRect*)NULL` default args, `if (&ref)` guards).
    # GCC assumes references are non-null and deletes those guards, turning
    # intended "no arg" cases into null derefs (View3d::MakeInteractive crash).
    target_compile_options(ProjectDefaults INTERFACE
        -m32 -fms-extensions -Wno-enum-compare -fno-delete-null-pointer-checks)
    target_link_options(ProjectDefaults INTERFACE -m32)

    if(MIG_SANITIZERS)
        message(STATUS "Sanitizers enabled: ASan + UBSan")
        target_compile_options(ProjectDefaults INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(ProjectDefaults INTERFACE
            -fsanitize=address,undefined)
    endif()

    if(MIG_LINT)
        find_program(CLANG_TIDY_EXE NAMES clang-tidy)
        find_program(CPPCHECK_EXE NAMES cppcheck)
        if(CLANG_TIDY_EXE)
            set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE}
                -header-filter='^${CMAKE_SOURCE_DIR}/(H|MFC|MigAlley)/')
        else()
            message(WARNING "MIG_LINT=ON but clang-tidy not found")
        endif()
        if(CPPCHECK_EXE)
            set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXE}
                --enable=warning,performance,portability
                --inline-suppr --quiet
                -I${CMAKE_SOURCE_DIR}/H
                -D__GNUC__)
        else()
            message(WARNING "MIG_LINT=ON but cppcheck not found")
        endif()
    endif()
endif()

target_include_directories(ProjectDefaults INTERFACE
    ${CMAKE_SOURCE_DIR}/H
    ${SDL2_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
)

target_link_libraries(ProjectDefaults INTERFACE
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${VULKAN_LIBRARIES}
)

target_compile_definitions(ProjectDefaults INTERFACE
    NDEBUG
    $<$<CONFIG:Debug>:_DEBUG>
)

# -------------------------------------------------------------------
# Add subprojects
# -------------------------------------------------------------------
add_subdirectory(AI)
add_subdirectory(AIRCRAFT)
add_subdirectory(3D)
add_subdirectory(Comms)
add_subdirectory(Files)
add_subdirectory(Graphics)
add_subdirectory(Hardware)
add_subdirectory(Input)
add_subdirectory(Math)
add_subdirectory(Missman)
add_subdirectory(Model)
add_subdirectory(Movecode)
add_subdirectory(Bfields)
add_subdirectory(Overlay)
add_subdirectory(CollisionsEtAl)
add_subdirectory(Assembler)
add_subdirectory(GENERAL)
add_subdirectory(MFC)
add_subdirectory(MigAlley)

enable_testing()
add_subdirectory(tests)
