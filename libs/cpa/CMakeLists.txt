add_subdirectory (location)
add_subdirectory (concrete)

add_library (cpa STATIC interface.cpp)


target_link_libraries (cpa hash location concrete)
target_precompile_headers (cpa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/cpa/location.hpp ${CMAKE_CURRENT_SOURCE_DIR}/include/cpa/arg.hpp
			  ${CMAKE_CURRENT_SOURCE_DIR}/include/cpa/compound.hpp
			  ${CMAKE_CURRENT_SOURCE_DIR}/include/cpa/pathformula.hpp	)

if(ENABLE_SYMBOLIC)
add_subdirectory (pathformula)
target_link_libraries (cpa pathformula	)
endif(ENABLE_SYMBOLIC)
	

target_include_directories (cpa INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
