# Dedicated `bob` executable (REQ-LAUNCH-02). Shares this project's compat
# layer (ProjectDefaults/WIN32_COMPAT/RowanMFC) — the BoB game module lands
# incrementally from bob-flight-sim's mfc/ tree onto it (T116 inventory).
get_filename_component(ROWAN_ENGINE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../rowan-engine" ABSOLUTE)

add_executable(bob
    bob_main.cpp
    ${ROWAN_ENGINE}/shell/detect.cpp
    ${ROWAN_ENGINE}/shell/manifests.cpp
)

target_include_directories(bob PRIVATE ${ROWAN_ENGINE}/shell)
target_link_libraries(bob PRIVATE ProjectDefaults)
