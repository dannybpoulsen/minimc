add_library (support SHARED feedback.cpp base64encode.cpp)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_link_libraries (support PRIVATE -Wl,-whole-archive linuxsupport -Wl,-no-whole-archive )
endif()	       


configure_file (
 "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/minimc/support/config.h"
)

target_include_directories (support PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>  )
target_link_libraries (support PUBLIC host )

target_sources(support
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/support/exceptions.hpp
    	  ${PROJECT_SOURCE_DIR}/include/minimc/support/feedback.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/support/localisation.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/support/overload.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/support/pipes.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/support/workinglist.hpp
)

target_sources(support
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}	
    FILES ${CMAKE_CURRENT_BINARY_DIR}/minimc/support/config.h)

