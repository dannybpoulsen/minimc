add_library (checkers INTERFACE)

add_subdirectory (typechecker)

target_link_libraries (checkers INTERFACE typechecker  )

	
	
	
	
