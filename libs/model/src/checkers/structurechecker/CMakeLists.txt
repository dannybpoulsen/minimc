add_library (structurechecker STATIC structurechecker.cpp)
target_include_directories (structurechecker PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories (structurechecker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (structurechecker support util host)

	
	
	
	
