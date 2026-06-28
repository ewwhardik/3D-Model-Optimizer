cmake_minimum_required(VERSION 3.20)
project(poop3DCompiler VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find packages
find_package(OpenGL REQUIRED)

# ─── GLAD ──────────────────────────────────────────────────────────────────────
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/src/glad)

if(NOT EXISTS "${GLAD_DIR}/glad.c")
    message(FATAL_ERROR
        "\n"
        "GLAD not found at src/glad/glad.c\n"
        "Run: python fetch_deps.py\n"
        "Then re-run CMake.\n")
endif()

add_library(glad STATIC ${GLAD_DIR}/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR})

# ─── ImGui (auto-download via FetchContent if not present) ─────────────────────
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/imgui)

if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
    message(STATUS "ImGui not found locally — downloading via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.90.5
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    message(STATUS "ImGui downloaded to: ${IMGUI_DIR}")
else()
    message(STATUS "ImGui found at: ${IMGUI_DIR}")
endif()

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ─── GLFW ──────────────────────────────────────────────────────────────────────
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    # Try pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW REQUIRED glfw3)
        add_library(glfw_pc INTERFACE)
        target_include_directories(glfw_pc INTERFACE ${GLFW_INCLUDE_DIRS})
        target_link_libraries(glfw_pc INTERFACE ${GLFW_LIBRARIES})
        add_library(glfw ALIAS glfw_pc)
    else()
        message(FATAL_ERROR "GLFW not found. Install via vcpkg (setup_windows.bat) or:\n  Linux: sudo apt-get install libglfw3-dev\n  Mac:   brew install glfw")
    endif()
endif()

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# ─── Assimp ────────────────────────────────────────────────────────────────────
set(HAVE_ASSIMP FALSE)
find_package(assimp CONFIG QUIET)
if(assimp_FOUND)
    set(HAVE_ASSIMP TRUE)
    message(STATUS "Assimp found via CMake config")
else()
    find_package(ASSIMP QUIET)
    if(ASSIMP_FOUND)
        set(HAVE_ASSIMP TRUE)
        message(STATUS "Assimp found via find_package")
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(ASSIMP QUIET assimp)
            if(ASSIMP_FOUND)
                set(HAVE_ASSIMP TRUE)
                message(STATUS "Assimp found via pkg-config")
            endif()
        endif()
    endif()
endif()

if(NOT HAVE_ASSIMP)
    message(STATUS "Assimp NOT found — OBJ-only import mode (install libassimp-dev for full format support)")
endif()

# ─── Main Application ──────────────────────────────────────────────────────────
add_executable(poop3D
    src/main.cpp
    src/app.cpp
    src/renderer.cpp
    src/mesh_optimizer.cpp
    src/ui_theme.cpp
)

if(WIN32)
    target_sources(poop3D PRIVATE src/windows_main.cpp)
endif()

target_include_directories(poop3D PRIVATE
    src
    ${GLAD_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_compile_definitions(poop3D PRIVATE
    _CRT_SECURE_NO_WARNINGS
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(poop3D PRIVATE
    glad
    imgui
    glfw
    OpenGL::GL
)

# Link Assimp
if(HAVE_ASSIMP)
    target_compile_definitions(poop3D PRIVATE HAVE_ASSIMP=1)
    if(TARGET assimp::assimp)
        target_link_libraries(poop3D PRIVATE assimp::assimp)
    elseif(TARGET assimp)
        target_link_libraries(poop3D PRIVATE assimp)
    else()
        target_link_libraries(poop3D PRIVATE ${ASSIMP_LIBRARIES})
        target_include_directories(poop3D PRIVATE ${ASSIMP_INCLUDE_DIRS})
    endif()
endif()

# Platform
if(WIN32)
    target_link_libraries(poop3D PRIVATE dwmapi)
    set_target_properties(poop3D PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    target_link_libraries(poop3D PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif(UNIX)
    target_link_libraries(poop3D PRIVATE dl pthread)
endif()

# Compiler flags
if(MSVC)
    target_compile_options(poop3D PRIVATE /W3 /fp:fast $<$<CONFIG:Release>:/O2>)
    target_compile_options(imgui  PRIVATE /W0 $<$<CONFIG:Release>:/O2>)
    target_compile_options(glad   PRIVATE /W0)
else()
    target_compile_options(poop3D PRIVATE -Wall -Wextra -O2 -ffast-math)
    target_compile_options(imgui  PRIVATE -w -O2)
    target_compile_options(glad   PRIVATE -w)
endif()

message(STATUS "")
message(STATUS "=== poop3D Compiler Build Config ===")
message(STATUS "  OpenGL:   ${OPENGL_LIBRARIES}")
message(STATUS "  Assimp:   ${HAVE_ASSIMP}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "=====================================")
