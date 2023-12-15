add_library (model STATIC cfg.cpp types.cpp variables.cpp program.cpp instructions.cpp  output.cpp symbol.cpp)
target_include_directories (model PUBLIC ${PROJECT_SOURCE_DIR}/include )
target_link_libraries (model util support host)

add_subdirectory (analysis)
add_subdirectory (checkers)
add_subdirectory (modifications)

target_link_libraries (model modelisa checkers modifications modelanalysis )

	
	
