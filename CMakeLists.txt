cmake_minimum_required(VERSION 3.10)

project(laz_engine VERSION 1.0
  DESCRIPTION "A tiny game engine"
  LANGUAGES C)

option(VULKAN_ENABLED "Enable Vulkan instead of OpenGL" OFF)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror=vla")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules/)

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR}/res)

find_package(GLFW3 REQUIRED)
find_program(GLSLC glslc REQUIRED)
if (VULKAN_ENABLED)
  find_package(Vulkan REQUIRED)
else() # OpenGL
  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL REQUIRED)
endif()

add_subdirectory(src)
