add_executable (test test.cpp)
target_link_libraries (test PUBLIC model)
target_include_directories (test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries (test PUBLIC support)
	
	
	
	
