
add_library (host SHARED )
target_include_directories (host PUBLIC ${PROJECT_SOURCE_DIR}/include)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
endif()


target_sources(host
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/host/casts.hpp
    	  ${PROJECT_SOURCE_DIR}/include/host/host.hpp
	    ${PROJECT_SOURCE_DIR}/include/host/operataions.hpp
	      ${PROJECT_SOURCE_DIR}/include/host/rightshifts.hpp
	        ${PROJECT_SOURCE_DIR}/include/host/types.hpp
	
	
	
)
