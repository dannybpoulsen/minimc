add_library (model SHARED types.cpp variables.cpp)
target_include_directories (model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries (model support)
add_subdirectory (tests)

	
	
	
	
