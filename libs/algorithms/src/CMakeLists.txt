add_subdirectory (gencases)
add_library (algorithms SHARED reachability.cpp )
target_link_libraries (algorithms PUBLIC cpa storage  smt gencases_lib)
target_include_directories (algorithms PUBLIC ${PROJECT_SOURCE_DIR}/include)


target_sources(algorithms
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/algorithms/gencases.hpp
    	  ${PROJECT_SOURCE_DIR}/include/algorithms/reachability.hpp
    	  
	  )
