add_subdirectory(shaders)
add_subdirectory(scripts)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.c glad/glad.c wren/wren.c)

set_target_properties(${PROJECT_NAME}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${GLFW3_LIBRARY}
  m
)

# Graphics API
if (VULKAN_ENABLED)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${Vulkan_LIBRARIES})
else() # OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE
  OpenGL::GL)
endif()


configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)
