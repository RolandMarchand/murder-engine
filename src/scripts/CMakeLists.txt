file(GLOB SCRIPT_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.wren")

# https://github.com/ccache/ccache/issues/1540#issuecomment-2699251319
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
    COMMAND ccache -C > /dev/null
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
    DEPENDS ${SCRIPT_FILES}
    COMMENT "Clearing CCache"
  )

  add_custom_target(clear_cache ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ccache_cleared.timestamp
  )
endif()
