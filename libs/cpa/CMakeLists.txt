add_subdirectory (location)
add_subdirectory (concrete)

add_library (cpa STATIC interface.cpp)


target_link_libraries (cpa hash location concrete vm)

if(ENABLE_SYMBOLIC)
add_subdirectory (pathformula)
target_link_libraries (cpa pathformula	)
endif(ENABLE_SYMBOLIC)
	

target_include_directories (cpa INTERFACE ${PROJECT_SOURCE_DIR}/include)
