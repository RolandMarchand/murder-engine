add_subdirectory(shaders)

# CCache violates correctness with files embedded using #embed:
# https://github.com/ccache/ccache/issues/1540#issuecomment-2699251319
file(GLOB OTHER_SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.wren"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl")

find_program(CCACHE_PROGRAM ccache)

if (CCACHE_PROGRAM)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
    COMMAND ccache -C > /dev/null
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
    DEPENDS ${OTHER_SRC_FILES}
    COMMENT "Clearing CCache"
  )

  add_custom_target(clear_cache ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
  )
endif()

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


