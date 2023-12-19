add_subdirectory (location)

add_library (cpa SHARED interface.cpp )
target_link_libraries (cpa PUBLIC  hash location concrete pathformula)
		      

target_include_directories (cpa PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_sources(cpa
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/cpa/interface.hpp
    	  ${PROJECT_SOURCE_DIR}/include/cpa/location.hpp
	  ${PROJECT_SOURCE_DIR}/include/cpa/query.hpp
	  ${PROJECT_SOURCE_DIR}/include/cpa/state.hpp
  	  ${PROJECT_SOURCE_DIR}/include/cpa/successorgen.hpp
		  
)
