cmake_minimum_required(VERSION 3.20)
project(arcdps_qol_tweaks VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui sources (shared, at repo root)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# MinHook (x86/x64 hooking library)
set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/minhook)
add_library(minhook STATIC
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde32.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC ${MINHOOK_DIR}/include)
target_include_directories(minhook PRIVATE ${MINHOOK_DIR}/src)

# Main addon DLL
add_library(arcdps_qol_tweaks SHARED
    src/main.cpp
    src/settings.cpp
    src/skip_logic.cpp
)

target_link_libraries(arcdps_qol_tweaks PRIVATE imgui minhook)
target_include_directories(arcdps_qol_tweaks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${IMGUI_DIR})

# Output name must contain "arcdps" for ArcDPS to discover it
set_target_properties(arcdps_qol_tweaks PROPERTIES
    OUTPUT_NAME "arcdps_qol_tweaks"
    SUFFIX ".dll"
    PREFIX ""
)
