add_library (model STATIC types.cpp variables.cpp)
target_include_directories (model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries (model support)

add_subdirectory(loaders)
add_subdirectory (analysis)
add_subdirectory (checkers)
add_subdirectory (modifications)


target_link_libraries (model checkers loader modifications modelanalysis)

	
	
	
	
