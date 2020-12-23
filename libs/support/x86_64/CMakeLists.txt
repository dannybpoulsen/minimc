
add_library (x86_64support OBJECT rightshifts.cpp div.cpp sext.cpp)
target_include_directories (x86_64support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_options(x86_64support PUBLIC "-fPIC")

	
