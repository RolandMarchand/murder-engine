add_executable(${PROJECT_NAME} main.c)

set_target_properties(${PROJECT_NAME}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${GLFW3_LIBRARY}
  ${Vulkan_LIBRARIES})
