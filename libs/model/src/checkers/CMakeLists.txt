add_library (checkers INTERFACE)

add_subdirectory (typechecker)
add_subdirectory (structurechecker)

target_link_libraries (checkers INTERFACE typechecker structurechecker )

	
	
	
	
