add_library (checkers STATIC ${PROJECT_BINARY_DIR}/isa/typechecker.cpp structurechecker.cpp)

set_source_files_properties(${PROJECT_BINARY_DIR}/isa/typechecker.cpp PROPERTIES GENERATED TRUE)

target_link_libraries (checkers modelisa support util host)
target_include_directories (checkers PUBLIC ${PROJECT_SOURCE_DIR}/include)


	
	
	
	
