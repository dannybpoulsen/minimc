add_executable (testee test.cpp)
target_link_libraries (testee PUBLIC model)
target_include_directories (testee PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries (testee PUBLIC support)
	
	
	
	
