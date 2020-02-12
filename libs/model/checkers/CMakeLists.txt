add_library (checkers INTERFACE)
target_include_directories (model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory (typechecker)
add_subdirectory (structurechecker)

target_link_libraries (checkers INTERFACE typechecker structurechecker)

	
	
	
	
