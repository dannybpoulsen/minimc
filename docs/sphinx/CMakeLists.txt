find_program (SPINXBUILD NAMES sphinx-build)

add_custom_target (sphinx_doc  COMMAND ${SPINXBUILD} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
