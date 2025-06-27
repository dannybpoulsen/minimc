add_library (checkers STATIC  structurechecker.cpp)


target_link_libraries (checkers modelisa support util host)
target_include_directories (checkers PUBLIC ${PROJECT_SOURCE_DIR}/include)


	
	
	
	
