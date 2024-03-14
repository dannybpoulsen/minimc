
add_library (cpa SHARED interface.cpp )
target_link_libraries (cpa PUBLIC  pathformula_headers concrete_headers model hash PRIVATE -Wl,-whole-archive concrete pathformula -Wl,-no-whole-archive)

target_include_directories (cpa PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_sources(cpa
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/cpa/interface.hpp
    	  ${PROJECT_SOURCE_DIR}/include/minimc/cpa/query.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/cpa/state.hpp
  	  ${PROJECT_SOURCE_DIR}/include/minimc/cpa/successorgen.hpp
		  
)
