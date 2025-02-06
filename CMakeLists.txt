cmake_minimum_required(VERSION 3.10)

# Project name
project(laz_engine VERSION 1.0
	DESCRIPTION "A tiny game engine"
	LANGUAGES C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(GLFW3 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined -Wall -Wextra -Wpedantic -Werror=vla")

add_subdirectory(src)
