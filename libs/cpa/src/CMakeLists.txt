add_subdirectory (location)

add_library (cpa STATIC interface.cpp )
target_link_libraries (cpa PUBLIC  hash location concrete pathformula)
		      

target_include_directories (cpa PUBLIC ${PROJECT_SOURCE_DIR}/include)
