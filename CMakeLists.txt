cmake_minimum_required(VERSION 3.20)
project(quakepg LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -- Third party: glad --
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glad/include)

# -- Third party: GLFW --
if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/glfw/CMakeLists.txt)
  message(STATUS "Using bundled GLFW from vendor/glfw")
  set(GLFW_BUILD_DOCS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11
      ON
      CACHE BOOL "" FORCE)
  add_subdirectory(vendor/glfw)
  set(GLFW_LIB glfw)
else()
  message(STATUS "Looking for system GLFW...")
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
endif()

# -- Third party: Dear ImGui (GLFW + OpenGL3 backend) --
add_library(
  imgui STATIC
  vendor/imgui/imgui.cpp
  vendor/imgui/imgui_demo.cpp
  vendor/imgui/imgui_draw.cpp
  vendor/imgui/imgui_tables.cpp
  vendor/imgui/imgui_widgets.cpp
  vendor/imgui/backends/imgui_impl_glfw.cpp
  vendor/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/vendor/imgui
                                        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends)
target_link_libraries(imgui PRIVATE ${GLFW_LIB} glad)

# -- Engine (static library) --
add_subdirectory(engine)

# -- Game (executable) --
add_subdirectory(game)

# -- Documentation (Doxygen) --
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)

  add_custom_target(
    docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
  message(STATUS "Doxygen found - 'make docs' target available")
else()
  message(STATUS "Doxygen not found - 'make docs' target unavailable")
endif()

# -- Editor (executable, future) --
# add_subdirectory(editor)
