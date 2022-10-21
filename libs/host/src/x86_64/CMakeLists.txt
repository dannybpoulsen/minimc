
add_library (x86_64host STATIC rightshifts.cpp div.cpp sext.cpp zext.cpp)
target_include_directories (x86_64host PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (x86_64host PUBLIC support)
target_compile_options(x86_64host PUBLIC "-fPIC")

	
