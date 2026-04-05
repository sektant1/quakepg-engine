# QuakePG Engine - Static Library

set(ENGINE_SOURCES
    src/core/console.cpp
    src/core/debug_ui.cpp
    src/core/imgui_theme.cpp
    src/platform/window.cpp
    src/platform/input.cpp
    src/platform/timer.cpp
    src/renderer/shader.cpp
    src/renderer/mesh.cpp
    src/renderer/renderer.cpp
    src/renderer/texture.cpp
    src/renderer/material.cpp
    src/renderer/camera.cpp
    src/renderer/model.cpp
    src/physics/collision.cpp)

add_library(quakepg_engine STATIC ${ENGINE_SOURCES})

# Public headers - consumers include <engine/...>
target_include_directories(quakepg_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Internal dependencies (not exposed to consumers)
target_link_libraries(
  quakepg_engine
  PUBLIC glad imgui
  PRIVATE ${GLFW_LIB} assimp::assimp ${CMAKE_DL_LIBS} m)

# Internal include paths (glad, stb_image, glfw)
target_include_directories(quakepg_engine PRIVATE ${CMAKE_SOURCE_DIR}/vendor/glad/include ${CMAKE_SOURCE_DIR}/vendor)
