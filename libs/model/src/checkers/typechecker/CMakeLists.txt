add_library (typechecker STATIC typechecker.cpp)
target_include_directories (typechecker PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (typechecker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (typechecker support util host)

	
	
	
	
