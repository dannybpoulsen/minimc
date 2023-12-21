
add_library (host SHARED )
target_include_directories (host PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include>)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
endif()


target_sources(host
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/host/casts.hpp
    	  ${PROJECT_SOURCE_DIR}/include/minimc/host/host.hpp
	    ${PROJECT_SOURCE_DIR}/include/minimc/host/operataions.hpp
	      ${PROJECT_SOURCE_DIR}/include/minimc/host/rightshifts.hpp
	        ${PROJECT_SOURCE_DIR}/include/minimc/host/types.hpp
	
	
	
)
