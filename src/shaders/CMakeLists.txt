if (VULKAN_ENABLED)
  set(VERT_IN ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-vertex.glsl)
  set(VERT_OUT ${CMAKE_CURRENT_BINARY_DIR}/vert.spv)
  set(FRAG_IN ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-fragment.glsl)
  set(FRAG_OUT ${CMAKE_CURRENT_BINARY_DIR}/frag.spv)

  add_custom_command(
    OUTPUT ${FRAG_OUT}
    COMMAND glslc -fshader-stage=frag ${FRAG_IN} -o ${FRAG_OUT}
    DEPENDS ${FRAG_IN}
    COMMENT "Compiling fragment shader"
  )

  add_custom_command(
    OUTPUT ${VERT_OUT}
    COMMAND glslc -fshader-stage=vert ${VERT_IN} -o ${VERT_OUT}
    DEPENDS ${VERT_IN}
    COMMENT "Compiling vertex shader"
  )

  add_custom_target(shaders
    DEPENDS ${VERT_OUT} ${FRAG_OUT}
  )
  add_dependencies(${PROJECT_NAME} shaders)

  # For config.h
  set(VULKAN_FRAGMENT_SHADER_PATH ${FRAG_OUT} PARENT_SCOPE)
  set(VULKAN_VERTEX_SHADER_PATH ${VERT_OUT} PARENT_SCOPE)
else()
endif()
