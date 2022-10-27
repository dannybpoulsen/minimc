add_subdirectory (location)
add_subdirectory (concrete)

add_library (cpa STATIC interface.cpp )


target_link_libraries (cpa PUBLIC  hash location concrete  vm)

if(ENABLE_SYMBOLIC)
add_subdirectory (pathformula)
target_link_libraries (cpa PUBLIC pathformula	)
endif(ENABLE_SYMBOLIC)
	

target_include_directories (cpa PUBLIC ${PROJECT_SOURCE_DIR}/include)
