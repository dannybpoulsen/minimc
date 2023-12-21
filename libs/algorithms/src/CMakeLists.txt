add_subdirectory (gencases)
add_library (algorithms SHARED reachability.cpp )
target_link_libraries (algorithms PUBLIC cpa storage  PRIVATE gencases_lib)
target_include_directories (algorithms PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include>)


target_sources(algorithms
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/algorithms/gencases.hpp
    	  ${PROJECT_SOURCE_DIR}/include/minimc/algorithms/reachability.hpp
    	  
	  )
