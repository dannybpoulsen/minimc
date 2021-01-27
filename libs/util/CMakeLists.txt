add_library (util INTERFACE)
target_link_libraries (util INTERFACE smtlib support hash)
target_include_directories (util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
